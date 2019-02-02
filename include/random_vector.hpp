/*
 * Copyright (c) Nuno Alves de Sousa 2019
 *
 * Use, modification and distribution is subject to the Boost Software License,
 * Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef RANDOM_VECTOR_H
#define RANDOM_VECTOR_H

#include <vector>
#include <random>
#include <cstdint>
#include <type_traits>

template<class T>
std::vector<T> makeRandomVector(std::int64_t size, T min, T max)
{
    std::random_device rd;  // To obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Random number engine

    std::vector<T> v;

    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<> uniformInt(min, max);
        v.reserve(size);
        for (std::size_t i = 0; i < size; ++i) {
            v.push_back(uniformInt(gen));
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<> uniformReal(min, max);
        v.reserve(size);
        for (std::size_t i = 0; i < size; ++i) {
            v.push_back(uniformReal(gen));
        }
    }
    return v;
}

#endif //RANDOM_VECTOR_H
