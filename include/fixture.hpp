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

enum class ProgressionPolicy {linear, geometric, semilogGemm, semilogAdd};

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

/**
 * Creates a set of matrix dimensions that follow a geometric progression
 * @param numberOfTests Total number of tests
 * @param increment Common ration for the geometric progression
 * @return A problemSpace (i.e. the set of matrix dimensions)
 */
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

/**
 * Creates a set of matrix dimensions to create a semilog plot when
 * benchmarking gdemm functions (faster decay of benchmark iterations)
 * @param numberOfTests Total number of tests
 * @return A problemSpace (i.e. the set of matrix dimensions)
 */
inline std::vector<celero::TestFixture::ExperimentValue>
semilogGemmProgression(int numberOfTests)
{
    std::vector<celero::TestFixture::ExperimentValue> problemSpace;

    std::int64_t matrixDim = 1;
    for (int i = 1; i <= numberOfTests; ++i) {
        auto orderMag = static_cast<std::int64_t>
            (std::floor(std::log10(matrixDim)));
        matrixDim += static_cast<std::int64_t>(std::pow(10, orderMag));

        // Adjust iterations of the problemSpace according to matrix dimensions
        static std::int64_t iterations;
        switch (matrixDim) {
            case      2: iterations = 100; break;
            case    100: iterations = 25;  break;
            case   1000: iterations = 5;   break;
            case 10'000: iterations = 3;   break;
        }
        problemSpace.push_back({matrixDim, iterations});
    }
    return problemSpace;
}

/**
 * Creates a set of matrix dimensions to create a semilog plot when
 * benchmarking matrix addition (slower decay of benchmark iterations)
 * @param numberOfTests Total number of tests
 * @return A problemSpace (i.e. the set of matrix dimensions)
 */
inline std::vector<celero::TestFixture::ExperimentValue>
semilogAddProgression(int numberOfTests)
{
    std::vector<celero::TestFixture::ExperimentValue> problemSpace;

    std::int64_t matrixDim = 1;
    for (int i = 1; i <= numberOfTests; ++i) {
        auto orderMag = static_cast<std::int64_t>
        (std::floor(std::log10(matrixDim)));
        matrixDim += static_cast<std::int64_t>(std::pow(10, orderMag));

        // Adjust iterations of the problemSpace according to matrix dimensions
        static std::int64_t iterations;
        switch (matrixDim) {
            case      2: iterations = 100; break;
            case   1000: iterations = 75;   break;
        }
        problemSpace.push_back({matrixDim, iterations});
    }
    return problemSpace;
}

/// Base class for all fixtures ralated to matrix operations
/// \tparam policy The ProgressionPolicy for the benchmark fixture
template<ProgressionPolicy policy>
class MatrixFixture: public celero::TestFixture
{
public:
    MatrixFixture() = default;

    /// The problem space is a set of matrix dimensions
    std::vector<celero::TestFixture::ExperimentValue> getExperimentValues() const override
    {
        switch (policy) {
            case ProgressionPolicy::linear:
                return linearProgression(numLinearProgressionTests
                                        ,increment);
            case ProgressionPolicy::geometric:
                return geometricProgression(numGeometricProgressionTests
                                           ,increment);
            case ProgressionPolicy::semilogGemm:
                return semilogGemmProgression(numSemilogGemmProgressionTests);
            case ProgressionPolicy::semilogAdd:
                return semilogAddProgression(numSemilogAddProgressionTests);
        }
    }

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

private:
    static constexpr int increment = 25;

    static constexpr int numLinearProgressionTests = 100;
    static constexpr int numGeometricProgressionTests = 100;
    static constexpr int numSemilogGemmProgressionTests = 35;

    // Fewer tests because of additional memory requirements (matrix copies)
    static constexpr int numSemilogAddProgressionTests = 35;

    static constexpr double dataMin = 0.0;
    static constexpr double dataMax = 1.0;
};

/// This MKL fixture allocates aligned buffers
template<ProgressionPolicy policy = ProgressionPolicy::linear>
class MKLFixture: public MatrixFixture<policy>
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
        mC = allocate_dmatrix(this->matrixSize);

        auto matrixData = this->makeRandomMatrixData();

        std::copy(matrixData.begin(), matrixData.end(), mA);
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

protected:
    // Helper allocation function
    virtual double* allocate_dmatrix(std::int64_t matrixSize)
    {
        auto ptr =
            static_cast<double*>
                (mkl_malloc(matrixSize*sizeof(double), this->align));
        if (!ptr) {
            std::cerr << "error: cannot allocate matrices\n";
            throw std::bad_alloc{};
        }
        return ptr;
    }

    MKLMatrix mA = nullptr;
    MKLMatrix mC = nullptr;

    static constexpr int align = 64; // default alignment on 64-byte bounndary
};

/// This MKL fixture with copies for mkl_domatadd
template<ProgressionPolicy policy = ProgressionPolicy::linear>
class MKLFixtureB: public MKLFixture<policy>
{
public:
    MKLFixtureB() = default;

    /// Before each run build matrices of random integers
    void setUp(const celero::TestFixture::ExperimentValue& experimentValue) override
    {
        // Update matrix dimensions and allocate mA
        MKLFixture<policy>::setUp(experimentValue);

        // Allocate additional space for mAcopy and mC
        mB = allocate_dmatrix(this->matrixSize);
        std::copy(this->mA, this->mA + this->matrixSize, mB);
        MKL_DEBUG(mB, this->matrixDim, this->matrixDim);
    }

    // Clear MKL matrices on tearDown and do not include deallocation time
    void tearDown() override
    {
        MKLFixture<policy>::tearDown();
        mkl_free(mB);
    }

protected:
    // Helper allocation function
    virtual double* allocate_dmatrix(std::int64_t matrixSize)
    {
        auto ptr =
             static_cast<double*>(mkl_malloc(matrixSize*sizeof(double), this->align));
        if (!ptr) {
            throw std::bad_alloc{};
        }
        return ptr;
    }

    MKLMatrix mB = nullptr;
};

template<ProgressionPolicy policy = ProgressionPolicy::linear>
class EigenFixture: public MatrixFixture<policy>
{
public:
    EigenFixture() = default;

    /// Before each run build matrices of random data and setup threading
    void setUp(const celero::TestFixture::ExperimentValue& experimentValue) override
    {
        if constexpr (numberOfThreads != 0) {
            // Eigen threading
            Eigen::setNbThreads(numberOfThreads);
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

protected:
    EigenMatrix eA;

    static constexpr int numberOfThreads = 4;
};

#endif //FIXTURE_DGEMM_H
