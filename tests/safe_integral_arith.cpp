/* Copyright 2009-2017 Francesco Biscani (bluescarni@gmail.com)

This file is part of the Piranha library.

The Piranha library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The Piranha library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the Piranha library.  If not,
see https://www.gnu.org/licenses/. */

#include <piranha/detail/safe_integral_arith.hpp>

#include <limits>
#include <random>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include <boost/algorithm/string/predicate.hpp>

#include <piranha/type_traits.hpp>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

using integral_types = std::tuple<char, unsigned char, signed char, short, unsigned short, int, unsigned, long,
                                  unsigned long, long long, unsigned long long>;

static std::mt19937 rng;

static const int ntries = 1000;

struct add_tester {
    template <typename T>
    void operator()(const T &) const
    {
        CHECK(safe_int_add(std::numeric_limits<T>::max(), T(0)) == std::numeric_limits<T>::max());
        CHECK(safe_int_add(std::numeric_limits<T>::min(), T(0)) == std::numeric_limits<T>::min());
        CHECK_THROWS_MATCHES(safe_int_add(std::numeric_limits<T>::max(), T(1)), std::overflow_error,
                              test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral addition: "))
        );
        CHECK_THROWS_MATCHES(safe_int_add(std::numeric_limits<T>::max(), T(5)), std::overflow_error,
                              test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral addition: "))
        );
        CHECK_THROWS_MATCHES(safe_int_add(std::numeric_limits<T>::max(), T(50)), std::overflow_error,
                              test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral addition: "))
        );
        if (std::is_signed<T>::value) {
            CHECK_THROWS_MATCHES(safe_int_add(std::numeric_limits<T>::min(), T(-1)), std::overflow_error,
                                  test::ExceptionMatcher<std::overflow_error>(std::string(
                                      "overflow error in an integral addition: "))
                                  );
            CHECK_THROWS_MATCHES(safe_int_add(std::numeric_limits<T>::min(), T(-5)), std::overflow_error,
                                  test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral addition: "))
                                  );
            CHECK_THROWS_MATCHES(safe_int_add(std::numeric_limits<T>::min(), T(-50)), std::overflow_error,
                                  test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral addition: "))
            );
        }
        using r_type = decltype(T(0) + T(0));
        std::uniform_int_distribution<r_type> dist(std::numeric_limits<T>::min() / T(5),
                                                   std::numeric_limits<T>::max() / T(5));
        for (auto i = 0; i < ntries; ++i) {
            const auto a = dist(rng), b = dist(rng);
            CHECK(safe_int_add(static_cast<T>(a), static_cast<T>(b)) == a + b);
        }
    }
};

TEST_CASE("sia_add_test")
{
    tuple_for_each(integral_types{}, add_tester{});
}

struct sub_tester {
    template <typename T>
    void operator()(const T &) const
    {
        CHECK(safe_int_sub(std::numeric_limits<T>::max(), T(0)) == std::numeric_limits<T>::max());
        CHECK(safe_int_sub(std::numeric_limits<T>::min(), T(0)) == std::numeric_limits<T>::min());
        CHECK_THROWS_MATCHES(safe_int_sub(std::numeric_limits<T>::min(), T(1)), std::overflow_error,
                              test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral subtraction: "))
        );
        CHECK_THROWS_MATCHES(safe_int_sub(std::numeric_limits<T>::min(), T(5)), std::overflow_error,
                              test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral subtraction: "))
        );
        CHECK_THROWS_MATCHES(safe_int_sub(std::numeric_limits<T>::min(), T(50)), std::overflow_error,
                              test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral subtraction: "))
        );
        if (std::is_signed<T>::value) {
            CHECK_THROWS_MATCHES(safe_int_sub(std::numeric_limits<T>::max(), T(-1)), std::overflow_error,
                                  test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral subtraction: "))
            );
            CHECK_THROWS_MATCHES(safe_int_sub(std::numeric_limits<T>::max(), T(-5)), std::overflow_error,
                                  test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral subtraction: "))
            );
            CHECK_THROWS_MATCHES(safe_int_sub(std::numeric_limits<T>::max(), T(-50)), std::overflow_error,
                                  test::ExceptionMatcher<std::overflow_error>(std::string("overflow error in an integral subtraction: "))
            );
        }
        using r_type = decltype(T(0) - T(0));
        std::uniform_int_distribution<r_type> dist(std::numeric_limits<T>::min() / T(5),
                                                   std::numeric_limits<T>::max() / T(5));
        for (auto i = 0; i < ntries; ++i) {
            auto a = dist(rng), b = dist(rng);
            if (a < b && std::is_unsigned<T>::value) {
                std::swap(a, b);
            }
            CHECK(safe_int_sub(static_cast<T>(a), static_cast<T>(b)) == a - b);
        }
    }
};

TEST_CASE("sia_sub_test")
{
    tuple_for_each(integral_types{}, sub_tester{});
}
