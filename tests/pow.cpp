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

#include <piranha/math/pow.hpp>

#include <cmath>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>

#include <mp++/config.hpp>
#include <mp++/exceptions.hpp>
#include <mp++/integer.hpp>

#include <piranha/integer.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"

using namespace piranha;

using size_types = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                              std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 7>,
                              std::integral_constant<std::size_t, 10>>;

struct b_00 {
    b_00() = default;
    b_00(const b_00 &) = delete;
    b_00(b_00 &&) = delete;
};

struct b_01 {
    b_01() = default;
    b_01(const b_01 &) = default;
    b_01(b_01 &&) = default;
    ~b_01() = delete;
};

struct foo {
};

namespace piranha
{

template <>
class pow_impl<b_00, b_00>
{
public:
    b_00 operator()(const b_00 &, const b_00 &) const;
};

template <>
class pow_impl<b_01, b_01>
{
public:
    b_01 operator()(const b_01 &, const b_01 &) const;
};

template <>
class pow_impl<foo, foo>
{
public:
    foo operator()(const foo &, const foo &) const;
    foo operator()(foo &, foo &) const = delete;
};
}

TEST_CASE("pow_fp_test")
{
    CHECK(piranha::pow(2., 2.) == std::pow(2., 2.));
    CHECK(piranha::pow(2.f, 2.) == std::pow(2.f, 2.));
    CHECK(piranha::pow(2., 2.f) == std::pow(2., 2.f));
    CHECK((std::is_same<decltype(piranha::pow(2., 2.)), double>::value));
    CHECK((std::is_same<decltype(piranha::pow(2.f, 2.f)), float>::value));
    CHECK((std::is_same<decltype(piranha::pow(2., 2.f)), double>::value));
    CHECK((std::is_same<decltype(piranha::pow(2.f, 2.)), double>::value));
    CHECK((std::is_same<decltype(piranha::pow(2.f, 2)), decltype(std::pow(2.f, 2))>::value));
    CHECK((std::is_same<decltype(piranha::pow(2.f, 2.L)), long double>::value));
    CHECK(piranha::pow(2., 2) == std::pow(2., 2));
    CHECK(piranha::pow(2.f, 2) == std::pow(2.f, 2));
    CHECK((std::is_same<decltype(piranha::pow(2., 2)), double>::value));
    CHECK((std::is_same<decltype(piranha::pow(2.f, 2)), decltype(std::pow(2.f, 2))>::value));
    CHECK((std::is_same<decltype(piranha::pow(2.f, char(2))), double>::value));
    CHECK((is_exponentiable<double, double>::value));
    CHECK((is_exponentiable<double>::value));
    CHECK((!is_exponentiable<void, double>::value));
    CHECK((!is_exponentiable<double, void>::value));
    CHECK((!is_exponentiable<void, void>::value));
    CHECK((!is_exponentiable<void>::value));
    CHECK((is_exponentiable<double, unsigned short>::value));
    CHECK((is_exponentiable<double &, double>::value));
    CHECK((is_exponentiable<double &>::value));
    CHECK((is_exponentiable<const double, double>::value));
    CHECK((is_exponentiable<const double>::value));
    CHECK((is_exponentiable<double &, double &>::value));
    CHECK((is_exponentiable<double &, double const &>::value));
    CHECK((is_exponentiable<double, double &>::value));
    CHECK((is_exponentiable<float, double>::value));
    CHECK((is_exponentiable<double, float>::value));
    CHECK((is_exponentiable<double, int>::value));
    CHECK((is_exponentiable<float, char>::value));
    CHECK((!is_exponentiable<std::string>::value));
    CHECK((is_exponentiable<foo>::value));
    CHECK((is_exponentiable<const foo>::value));
    CHECK((is_exponentiable<const foo &>::value));
    CHECK((!is_exponentiable<foo &>::value));
    CHECK((!is_exponentiable<foo &, foo &>::value));
    CHECK((is_exponentiable<const foo &, foo &>::value));
}

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long
#if defined(MPPP_HAVE_GCC_INT128)
                             ,
                             __int128_t, __uint128_t
#endif
                             >;

struct int_pow_tester {
    template <typename U>
    struct runner {
        template <typename T>
        void operator()(const T &) const
        {
            using int_type = mppp::integer<U::value>;
            CHECK((is_exponentiable<int_type, T>::value));
            CHECK((is_exponentiable<int_type, float>::value));
            CHECK((is_exponentiable<float, int_type>::value));
            CHECK((is_exponentiable<double, int_type>::value));
#if defined(MPPP_WITH_MPFR)
            CHECK((is_exponentiable<long double, int_type>::value));
#endif
            int_type n;
            CHECK((std::is_same<int_type, decltype(piranha::pow(n, T(0)))>::value));
            CHECK(piranha::pow(n, T(0)) == 1);
            // NOTE: for the 128-bit ints, is_signed will always be false.
            if (std::is_signed<T>::value) {
                CHECK_THROWS_AS(piranha::pow(n, T(-1)), mppp::zero_division_error);
            }
            n = 1;
            CHECK(piranha::pow(n, T(0)) == 1);
            if (std::is_signed<T>::value) {
                CHECK(piranha::pow(n, T(-1)) == 1);
            }
            n = -1;
            CHECK(piranha::pow(n, T(0)) == 1);
            if (std::is_signed<T>::value) {
                CHECK(piranha::pow(n, T(-1)) == -1);
            }
            n = 2;
            CHECK(piranha::pow(n, T(0)) == 1);
            CHECK(piranha::pow(n, T(1)) == 2);
            CHECK(piranha::pow(n, T(2)) == 4);
            CHECK(piranha::pow(n, T(4)) == 16);
            CHECK(piranha::pow(n, T(5)) == 32);
            if (std::is_signed<T>::value) {
                CHECK(piranha::pow(n, T(-1)) == 0);
            }
            n = -3;
            CHECK(piranha::pow(n, T(0)) == 1);
            CHECK(piranha::pow(n, T(1)) == -3);
            CHECK(piranha::pow(n, T(2)) == 9);
            CHECK(piranha::pow(n, T(4)) == 81);
            CHECK(piranha::pow(n, T(5)) == -243);
            CHECK(piranha::pow(n, T(13)) == -1594323);
            if (std::is_signed<T>::value) {
                CHECK(piranha::pow(n, T(-1)) == 0);
            }
            // Test here the various piranha::pow() overloads as well.
            // Integer--integer.
            CHECK((is_exponentiable<int_type, int_type>::value));
            CHECK((is_exponentiable<int_type>::value));
            CHECK((std::is_same<int_type, decltype(piranha::pow(int_type(1), int_type(1)))>::value));
            CHECK(piranha::pow(int_type(2), int_type(3)) == 8);
            // Integer -- integral.
            CHECK((is_exponentiable<int_type, int>::value));
            CHECK((is_exponentiable<int_type, char>::value));
            CHECK((is_exponentiable<int_type, unsigned long>::value));
            CHECK((std::is_same<int_type, decltype(piranha::pow(int_type(1), 1))>::value));
            CHECK((std::is_same<int_type, decltype(piranha::pow(int_type(1), 1ul))>::value));
            CHECK(
                (std::is_same<int_type, decltype(piranha::pow(int_type(1), static_cast<signed char>(1)))>::value));
            CHECK(piranha::pow(int_type(2), 3) == 8);
            // Integer -- floating-point.
            CHECK((is_exponentiable<int_type, double>::value));
            CHECK((std::is_same<double, decltype(piranha::pow(int_type(1), 1.))>::value));
            CHECK(piranha::pow(int_type(2), 3.) == piranha::pow(2., 3.));
            CHECK(piranha::pow(int_type(2), 1. / 3.) == piranha::pow(2., 1. / 3.));
            // Integral -- integer.
            CHECK((is_exponentiable<int, int_type>::value));
            CHECK((is_exponentiable<short, int_type>::value));
            CHECK((std::is_same<int_type, decltype(piranha::pow(1, int_type(1)))>::value));
            CHECK((std::is_same<int_type, decltype(piranha::pow(short(1), int_type(1)))>::value));
            CHECK(piranha::pow(2, int_type(3)) == 8.);
            // Floating-point -- integer.
            CHECK((is_exponentiable<float, int_type>::value));
            CHECK((is_exponentiable<double, int_type>::value));
            CHECK((std::is_same<float, decltype(piranha::pow(1.f, int_type(1)))>::value));
            CHECK((std::is_same<double, decltype(piranha::pow(1., int_type(1)))>::value));
            CHECK(piranha::pow(2.f, int_type(3)) == piranha::pow(2.f, 3.f));
            CHECK(piranha::pow(2., int_type(3)) == piranha::pow(2., 3.));
            CHECK(piranha::pow(2.f / 5.f, int_type(3)) == piranha::pow(2.f / 5.f, 3.f));
            CHECK(piranha::pow(2. / 7., int_type(3)) == piranha::pow(2. / 7., 3.));
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(int_types{}, runner<T>{});
    }
};

struct integer_pow_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK((is_exponentiable<int_type, int_type>::value));
        CHECK((is_exponentiable<int_type>::value));
        CHECK((!is_exponentiable<int_type, void>::value));
        CHECK((!is_exponentiable<void, int_type>::value));
        CHECK((!is_exponentiable<int_type, std::string>::value));
        CHECK((!is_exponentiable<std::string, int_type>::value));
        CHECK((is_exponentiable<const int_type, int_type &>::value));
        CHECK((is_exponentiable<float, int_type>::value));
        CHECK((is_exponentiable<float &&, const int_type &>::value));
        CHECK((is_exponentiable<double, int_type>::value));
        CHECK((is_exponentiable<double, int_type &>::value));
#if defined(MPPP_WITH_MPFR)
        CHECK((is_exponentiable<long double, int_type>::value));
        CHECK((is_exponentiable<const long double, int_type &&>::value));
#endif
        int_type n;
        CHECK((std::is_same<int_type, decltype(piranha::pow(n, n))>::value));
        CHECK(piranha::pow(n, int_type(0)) == 1);
        CHECK_THROWS_AS(piranha::pow(n, int_type(-1)), mppp::zero_division_error);
        n = 1;
        CHECK(piranha::pow(n, int_type(0)) == 1);
        CHECK(piranha::pow(n, int_type(-1)) == 1);
        n = -1;
        CHECK(piranha::pow(n, int_type(0)) == 1);
        CHECK(piranha::pow(n, int_type(-1)) == -1);
        n = 2;
        CHECK(piranha::pow(n, int_type(0)) == 1);
        CHECK(piranha::pow(n, int_type(1)) == 2);
        CHECK(piranha::pow(n, int_type(2)) == 4);
        CHECK(piranha::pow(n, int_type(4)) == 16);
        CHECK(piranha::pow(n, int_type(5)) == 32);
        CHECK(piranha::pow(n, int_type(-1)) == 0);
        n = -3;
        CHECK(piranha::pow(n, int_type(0)) == 1);
        CHECK(piranha::pow(n, int_type(1)) == -3);
        CHECK(piranha::pow(n, int_type(2)) == 9);
        CHECK(piranha::pow(n, int_type(4)) == 81);
        CHECK(piranha::pow(n, int_type(5)) == -243);
        CHECK(piranha::pow(n, int_type(13)) == -1594323);
        CHECK(piranha::pow(n, int_type(-1)) == 0);
    }
};

TEST_CASE("pow_integer_test")
{
    tuple_for_each(size_types{}, int_pow_tester{});
    tuple_for_each(size_types{}, integer_pow_tester{});
    // Integral--integral pow.
    CHECK(piranha::pow(4, 2) == 16);
    CHECK(piranha::pow(-3ll, static_cast<unsigned short>(3)) == -27);
    CHECK((std::is_same<integer, decltype(piranha::pow(-3ll, static_cast<unsigned short>(3)))>::value));
    CHECK((is_exponentiable<int, int>::value));
    CHECK((is_exponentiable<int>::value));
    CHECK((is_exponentiable<int, char>::value));
    CHECK((is_exponentiable<unsigned, long long>::value));
    CHECK((!is_exponentiable<mppp::integer<1>, mppp::integer<2>>::value));
    CHECK((!is_exponentiable<mppp::integer<2>, mppp::integer<1>>::value));
    CHECK((!is_exponentiable<integer, std::string>::value));
    CHECK((!is_exponentiable<b_00, b_00>::value));
    CHECK((!is_exponentiable<b_00>::value));
    CHECK((!is_exponentiable<b_01, b_01>::value));
    CHECK((!is_exponentiable<b_01>::value));
}
