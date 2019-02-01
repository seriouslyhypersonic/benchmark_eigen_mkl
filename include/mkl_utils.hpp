/*
 * Copyright (c) Nuno Alves de Sousa 2019
 *
 * Use, modification and distribution is subject to the Boost Software License,
 * Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef MKL_UTILS_H
#define MKL_UTILS_H

#include <iostream>
#include <string>
#include <iomanip>

/// Contiguous memory index for column-major memory layout
#define N_CM(i, j, M, N) (i + j*M)

/// Print matrix in column-major order
void printColMajor(const std::string& matrixName,
                   const double* matrix, std::size_t m, std::size_t n)
{
    std::cout << matrixName << " = {\n";
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            std::cout << std::setw(9) << std::right << matrix[N_CM(i,j, m, n)] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "}\n";
}

#endif //MKL_UTILS_H
