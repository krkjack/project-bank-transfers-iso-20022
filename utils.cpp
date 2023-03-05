/*
 * MIT License
 * Copyright (c) 2017 Kevin Kirchner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * \file        utils.h
 * \brief       Source file implementing some useful utility functions
 * \author      Kevin Kirchner
 * \date        2017
 * \copyright   MIT LICENSE
 *
 * This source file implements some useful utility functions.
 */

#include "utils.h"

// If using Boost Random
#if USE_BOOST

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

/**
 * Generates and returns a randomly generated alphanumeric string.
 *
 * @param length The desired length of the string
 * @return Randomly generated alphanumeric string with length \p length
 */
std::string generateRandomString(const size_t length) {
    static const std::string chars("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, static_cast<int>(chars.size() - 1));
    std::string str = "";
    for (size_t i = 0; i < length; i++) {
        str.push_back(chars[static_cast<size_t>(index_dist(rng))]);
    }
    return str;
}

#else

#include <ctime>

/**
 * Generates and returns a randomly generated alphanumeric string.
 *
 * @param length The desired length of the string
 * @return Randomly generated alphanumeric string with length \p length
 */
std::string generateRandomString(const size_t length) {
    static const std::string chars("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    // add a changing factor 'div' to seed so there is a bit more diversity
    // The test will fail without it, because they are running so fast, that
    // each of them gets the same random number xD
    static long div = 100;
    std::srand(static_cast<unsigned int>(std::time(0) + (div % 1000000)));
    div += 42;

    std::string str = "";
    for (size_t i = 0; i < length; i++) {
        str.push_back(chars[static_cast<size_t>(std::rand()) % (chars.length() - 1)]);
    }
    return str;
}

#endif
