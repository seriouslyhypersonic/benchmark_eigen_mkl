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

const int numSamples = 3;
const int numIterations = 75;

BASELINE_F(Gemm, Baseline, MKLFixture<>, numSamples, numIterations)
{
    squareDgemm(mA, mA, mC, matrixDim, 1, 1);
}

BENCHMARK_F(Gemm, Eigen, EigenFixture<4>, numSamples, numIterations)
{
    celero::DoNotOptimizeAway((eA * eA).eval());
}