/*
 * Copyright (c) Nuno Alves de Sousa 2019
 *
 * Use, modification and distribution is subject to the Boost Software License,
 * Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <algorithm>

#include <celero/Celero.h>

#include <Eigen/Eigen>

#include <mkl.h>

#include <random_vector.hpp>
#include <debug.hpp>

void dgemmTest()
{
    const int m = 3;
    const int n = 3;

    std::cout << "Generating random data...\n\n";
    auto randomDoubles = makeRandomVector(m*n, 0.0, 2.0);

    // Eigen initialization
    Eigen::MatrixXd eA;
    eA = Eigen::Map<Eigen::MatrixXd>(randomDoubles.data(), m, n);

    std::cout << "Eigen matrix:\n";
    EIGEN_DEBUG(eA);

    // MKL initialization
    auto mA = static_cast<double*>(mkl_malloc(m*n*sizeof(double), 64));
    auto mC = static_cast<double*>(mkl_malloc(m*n*sizeof(double), 64));
    if (mA == nullptr || mC == nullptr) {
        std::cerr << "error: memory allocation failed. Aborting...";
        mkl_free(mA);
        mkl_free(mC);
        return;
    }
    std::copy(randomDoubles.begin(), randomDoubles.end(), mA);
    std::fill(mC, mC + n*m, 0);

    std::cout << "\nMKL matrices:\n";
    MKL_DEBUG(mA, m, n);
    MKL_DEBUG(mC, m, n);

    std::cout << "\nEigen A * A\n";
    std::cout << eA * eA << '\n';

    std::cout << "\nMKL A * A\n";
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m, n, n, 1, mA, m, mA, m, 1, mC, m);
    MKL_DEBUG(mC, m, n);


    // Cleanup
    mkl_free(mA);
    mkl_free(mC);
}