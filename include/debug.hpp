/*
 * Copyright (c) Nuno Alves de Sousa 2019
 *
 * Use, modification and distribution is subject to the Boost Software License,
 * Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <mkl_utils.hpp>

#ifdef PRINT_DEBUG_INFORMATION
    #define EIGEN_DEBUG(x) std::cout << #x " = {\n" << x << "\n}" << std::endl
    #define MKL_DEBUG(matrix, m, n) printColMajor(#matrix, matrix, m, n)
#else
    #define PRINT_DEBUG(x) void ANONYMOUS_FUNCTION()
#endif

#endif //DEBUG_H
