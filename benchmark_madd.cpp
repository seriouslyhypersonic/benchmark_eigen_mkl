/*
 * Copyright (c) Nuno Alves de Sousa 2019
 *
 * Use, modification and distribution is subject to the Boost Software License,
 * Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <celero/Celero.h>

#include <fixture.hpp>

#include <mkl.h>

CELERO_MAIN

const int numSemilogSamples = 15;
const int numSemilogIterations = 0;

BASELINE_F(MatrixAdd, Baseline, MKLFixture<ProgressionPolicy::semilogAdd>, numSemilogSamples, numSemilogIterations)
{
    cblas_daxpy(matrixSize, 1, mA, 1, mA, 1);
}

BENCHMARK_F(MatrixAdd, MKLvAdd, MKLFixture<ProgressionPolicy::semilogAdd>, numSemilogSamples, numSemilogIterations)
{
    vdAdd(matrixSize, mA, mA, mA);
}

BENCHMARK_F(MatrixAdd, MKLdomatadd, MKLFixtureB<ProgressionPolicy::semilogAdd>, numSemilogSamples, numSemilogIterations)
{
    mkl_domatadd('C', 'N', 'N', matrixDim, matrixDim, 1, mA, matrixDim, 1, mB, matrixDim, mC, matrixDim);
}

BENCHMARK_F(MatrixAdd, Eigen, EigenFixture<ProgressionPolicy::semilogAdd>, numSemilogSamples, numSemilogIterations)
{
    celero::DoNotOptimizeAway((eA + eA).eval());
}

BENCHMARK_F(MatrixAdd, ForLoop, MKLFixture<ProgressionPolicy::semilogAdd>, numSemilogSamples, numSemilogIterations)
{
    for (int i = 0; i < this->matrixSize; ++i) {
        mA[i] = mA[i] + mA[i];
    }
    celero::DoNotOptimizeAway(mA);
}