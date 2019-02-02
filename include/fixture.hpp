/*
 * Copyright (c) Nuno Alves de Sousa 2019
 *
 * Use, modification and distribution is subject to the Boost Software License,
 * Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef FIXTURE_DGEMM_H
#define FIXTURE_DGEMM_H

#include <cstdint>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <new>

#include <celero/Celero.h>

#include <Eigen/Eigen>

#include <mkl.h>

#include <random_vector.hpp>
#include <debug.hpp>

using EigenMatrix = Eigen::MatrixXd;
using MKLMatrix = double*;

constexpr int defaultNumberOfTests = 100;
constexpr int defaultIncrement = 25;

enum class ProgressionPolicy {linear, geometric};

/**
 * Creates a set of linearly increasing matrix dimensions
 * @param numberOfTests Total number of tests
 * @param increment Increment value for the next set of matrix dimensions
 * @return A problemSpace (i.e. the set of matrix dimensions)
 */
inline std::vector<celero::TestFixture::ExperimentValue>
linearProgression(int numberOfTests, int increment)
{
    std::vector<celero::TestFixture::ExperimentValue> problemSpace;

    // Generate a set of matrix dimensions
    for (int i = 0; i < numberOfTests; ++i) {
        // matrix dimensions as a function of the test number
        problemSpace.push_back(static_cast<std::int64_t>(increment*(i+1)));
    }
    return problemSpace;
}

inline std::vector<celero::TestFixture::ExperimentValue>
geometricProgression(int numberOfTests, int increment)
{
    // Start Value = 1 and Iterations = 0 (default)
    std::vector<celero::TestFixture::ExperimentValue> problemSpace{1, 0};

    // Generate a set of matrix dimensions
    for (int i = 0; i < numberOfTests; ++i) {
        // matrix dimensions as a function of the test number
        problemSpace.push_back(static_cast<std::int64_t>(problemSpace.back().Value * increment));
    }
    return problemSpace;
}

template<ProgressionPolicy policy, int numberOfTests, int increment>
class MatrixFixture: public celero::TestFixture
{
public:
    MatrixFixture() = default;

    /// The problem space is a set of matrix dimensions
    std::vector<celero::TestFixture::ExperimentValue> getExperimentValues() const override
    {
        switch (policy) {
            case ProgressionPolicy::linear:
                return linearProgression(numberOfTests, increment);
            case ProgressionPolicy::geometric:
                return geometricProgression(numberOfTests, increment);
        }
    }

    static constexpr double dataMin = 0.0;
    static constexpr double dataMax = 1.0;

protected:
    // Helper setter
    void updateMatrixDim(std::int64_t newDim)
    {
        matrixDim = newDim;
        matrixSize = matrixDim * matrixDim;
    }

    // Helper generator according to current matrix dimensions
    std::vector<double> makeRandomMatrixData()
    {
        return makeRandomVector(matrixSize, dataMin, dataMax);
    }

    std::int64_t matrixDim;
    std::int64_t matrixSize;
};

template<ProgressionPolicy policy = ProgressionPolicy::linear
        ,int numberOfTests = defaultNumberOfTests
        ,int increment = defaultIncrement>
class MKLFixture: public MatrixFixture<policy, numberOfTests, increment>
{
public:
    MKLFixture() = default;

    /// Before each run build matrices of random integers
    void setUp(const celero::TestFixture::ExperimentValue& experimentValue) override
    {
        // Update matrix dimensions based on the current experiment
        this->updateMatrixDim(experimentValue.Value);

        // Initialize MKL matrices

        mA = allocate_dmatrix(this->matrixSize);
        auto matrixData = this->makeRandomMatrixData();
        std::copy(matrixData.begin(), matrixData.end(), mA);

        mC = allocate_dmatrix(this->matrixSize);
        std::fill(mC, mC + this->matrixSize, 0);

        MKL_DEBUG(mA, this->matrixDim, this->matrixDim);
        MKL_DEBUG(mC, this->matrixDim, this->matrixDim);
    }

    // Clear MKL matrices on tearDown and do not include deallocation time
    void tearDown() override
    {
        mkl_free(mA);
        mkl_free(mC);
    }

    MKLMatrix mA;
    MKLMatrix mC;

private:
    // Helper allocation function
    double* allocate_dmatrix(std::int64_t matrixSize)
    {
        auto ptr =
            static_cast<double*>(mkl_malloc(matrixSize*sizeof(double), align));
        if (!ptr) {
            throw std::bad_alloc{};
        }
        return ptr;
    }

    static constexpr int align = 64; // bytes
};

template<int nThreads
        ,ProgressionPolicy policy = ProgressionPolicy::linear
        ,int numberOfTests = defaultNumberOfTests
        ,int increment = defaultIncrement>
class EigenFixture: public MatrixFixture<policy, numberOfTests, increment>
{
public:
    EigenFixture() = default;

    /// Before each run build matrices of random data and setup threading
    void setUp(const celero::TestFixture::ExperimentValue& experimentValue) override
    {
        if constexpr (nThreads != 0) {
            // Eigen threading
            Eigen::setNbThreads(nThreads);
        }

        // Update matrix dimensions based on the current experiment
        this->updateMatrixDim(experimentValue.Value);

        // Initialize Eigen matrix
        auto matrixData = this->makeRandomMatrixData();
        eA = Eigen::Map<EigenMatrix>(matrixData.data()
                                    ,this->matrixDim
                                    ,this->matrixDim);
        EIGEN_DEBUG(eA);
    }

    EigenMatrix eA;
};

#endif //FIXTURE_DGEMM_H
