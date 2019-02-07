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

//const int numLinearSamples = 3;
//const int numLinearIterations = 75;
//
//BASELINE_F(Gemm, Baseline, MKLFixture<>, numLinearSamples, numLinearIterations)
//{
//    squareDgemm(mA, mA, mC, matrixDim, 1, 1);
//}
//
//BENCHMARK_F(Gemm, Eigen, EigenFixture<>, numLinearSamples, numLinearIterations)
//{
//    celero::DoNotOptimizeAway((eA * eA).eval());
//}

const int numSemilogSamples = 5;
const int numSemilogIterations = 0;

BASELINE_F(SemilogGemm, Baseline, MKLFixture<ProgressionPolicy::semilogGemm>
          ,numSemilogSamples, numSemilogIterations)
{
    squareDgemm(mA, mA, mC, matrixDim, 1, 1);
}

BENCHMARK_F(SemilogGemm, Eigen, EigenFixture<ProgressionPolicy::semilogGemm>
           ,numSemilogSamples, numSemilogIterations)
{
    celero::DoNotOptimizeAway((eA * eA).eval());
}