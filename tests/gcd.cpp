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

#include <piranha/math/gcd.hpp>
#include <piranha/math/gcd3.hpp>


#include <random>
#include <type_traits>

#include <mp++/integer.hpp>

#include <piranha/integer.hpp>

#include "catch.hpp"

using namespace piranha;

static std::mt19937 rng;
static const int ntries = 1000;

// A fake GCD-enabled type.
struct mock_type {
};

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
class gcd_impl<b_00, b_00>
{
public:
    b_00 operator()(const b_00 &, const b_00 &) const;
};

template <>
class gcd_impl<b_01, b_01>
{
public:
    b_01 operator()(const b_01 &, const b_01 &) const;
};

template <>
struct gcd_impl<mock_type, mock_type> {
    mock_type operator()(const mock_type &, const mock_type &) const;
};

template <>
struct gcd_impl<foo, foo> {
    foo operator()(const foo &, const foo &) const;
    foo operator()(foo &, foo &) const = delete;
};

template <>
struct gcd3_impl<mock_type, mock_type, mock_type> {
    void operator()(mock_type &, const mock_type &, const mock_type &) const;
};
}

TEST_CASE("gcd_test_00")
{
    CHECK((!are_gcd_types<double, double>::value));
    CHECK((!are_gcd_types<double>::value));
    CHECK((!are_gcd_types<void, double>::value));
    CHECK((!are_gcd_types<double, void>::value));
    CHECK((!are_gcd_types<void, void>::value));
    CHECK((!are_gcd_types<b_00, b_00>::value));
    CHECK((!are_gcd_types<b_00>::value));
    CHECK((!are_gcd_types<b_01, b_01>::value));
    CHECK((!are_gcd_types<b_01>::value));
    CHECK((!are_gcd3_types<double &, double, double>::value));
    CHECK((!are_gcd3_types<double &, double>::value));
    CHECK((!are_gcd3_types<double &, void, int>::value));
    CHECK((!are_gcd3_types<void, void, int>::value));
    CHECK((are_gcd_types<int, int>::value));
    CHECK((are_gcd_types<int>::value));
    CHECK((are_gcd_types<const int>::value));
    CHECK((are_gcd_types<int &>::value));
    CHECK((are_gcd_types<int &&>::value));
    CHECK((are_gcd_types<int, int &>::value));
    CHECK((are_gcd_types<const int, int>::value));
    CHECK((are_gcd_types<int &&, int &>::value));
    CHECK((are_gcd_types<short, long>::value));
    CHECK((are_gcd_types<signed char, unsigned long &>::value));
    CHECK((are_gcd_types<const unsigned, long long>::value));
    CHECK((are_gcd_types<int &&, unsigned char &>::value));
    CHECK((are_gcd_types<mock_type, mock_type>::value));
    CHECK((are_gcd_types<mock_type>::value));
    CHECK((are_gcd_types<const mock_type, mock_type &>::value));
    CHECK((are_gcd_types<mock_type &&, const mock_type &>::value));
    CHECK((are_gcd_types<foo, foo>::value));
    CHECK((are_gcd_types<const foo, foo>::value));
    CHECK((are_gcd_types<const foo, const foo>::value));
    CHECK((are_gcd_types<foo, const foo>::value));
    CHECK((are_gcd_types<const foo &, foo>::value));
    CHECK((are_gcd_types<const foo &, const foo &>::value));
    CHECK((are_gcd_types<foo, const foo &>::value));
    CHECK((!are_gcd_types<foo &, foo &>::value));
    CHECK((are_gcd_types<foo &, const foo &>::value));
    CHECK((!are_gcd3_types<mock_type, mock_type, mock_type>::value));
    CHECK((are_gcd3_types<mock_type &, mock_type, mock_type>::value));
    CHECK((are_gcd3_types<mock_type &, const mock_type, mock_type &>::value));
    CHECK((are_gcd3_types<mock_type &, const mock_type &, mock_type &&>::value));
    CHECK((!are_gcd3_types<const mock_type, mock_type, mock_type>::value));
    CHECK((!are_gcd3_types<const mock_type &, mock_type, mock_type>::value));
    CHECK((!are_gcd3_types<mock_type &&, mock_type, mock_type>::value));
    CHECK((!are_gcd3_types<b_00, b_00, b_00>::value));
    CHECK((!are_gcd3_types<b_01, b_01, b_01>::value));
    CHECK((!are_gcd3_types<b_01, b_01, b_01>::value));
    CHECK((!are_gcd3_types<std::string &, const std::string>::value));
    CHECK((!are_gcd3_types<int, int>::value));
    CHECK((are_gcd3_types<int &, int, int>::value));
    CHECK((are_gcd3_types<int &, int &&, const int &&>::value));
    CHECK((are_gcd3_types<int &, const int &, int &&>::value));
    CHECK((!are_gcd3_types<const int, const int &, int &&>::value));
    CHECK((!are_gcd3_types<const int &, const int &, int &&>::value));
    CHECK((!are_gcd3_types<int &&, const int &, int &&>::value));
    CHECK((!are_gcd3_types<const int &&, const int &, int &&>::value));
    CHECK(piranha::gcd(0, 0) == 0);
    CHECK(piranha::gcd(0, 12) == 12);
    CHECK(piranha::gcd(14, 0) == 14);
    CHECK(piranha::gcd(4, 3) == 1);
    CHECK(piranha::gcd(4, 3l) == 1);
    CHECK(piranha::gcd(4l, 3l) == 1);
    CHECK(piranha::gcd(4l, 3) == 1);
    CHECK((std::is_same<int, decltype(piranha::gcd(0, 0))>::value));
    CHECK((std::is_same<long, decltype(piranha::gcd(0l, 0))>::value));
    CHECK((std::is_same<long, decltype(piranha::gcd(0, 0l))>::value));
    CHECK((std::is_same<long, decltype(piranha::gcd(0l, 0l))>::value));
    CHECK(piranha::gcd(3, 4) == 1);
    CHECK(piranha::gcd(4, 6) == 2);
    CHECK(piranha::gcd(6, 4) == 2);
    CHECK(piranha::gcd(4, 25) == 1);
    CHECK(piranha::gcd(25, 4) == 1);
    CHECK(piranha::gcd(27, 54) == 27);
    CHECK(piranha::gcd(54, 27) == 27);
    CHECK(piranha::gcd(1, 54) == 1);
    CHECK(piranha::gcd(54, 1) == 1);
    CHECK(piranha::gcd(36, 24) == 12);
    CHECK(piranha::gcd(24, 36) == 12);
    // Check with short ints.
    CHECK(
        (std::is_same<decltype(piranha::gcd(short(54), short(27))), std::common_type<short, short>::type>::value));
    CHECK(
        (std::is_same<decltype(piranha::gcd(short(54), char(27))), std::common_type<short, char>::type>::value));
    CHECK(piranha::gcd(short(54), short(27)) == 27);
    CHECK(piranha::gcd(short(27), short(53)) == 1);
    CHECK(piranha::gcd(short(27), short(-54)) == short(27));
    CHECK(piranha::gcd(short(-54), short(27)) == short(27));
    // Couple of tests with bools.
    CHECK(!piranha::gcd(false, false));
    CHECK(piranha::gcd(true, false));
    CHECK(piranha::gcd(false, true));
    CHECK(piranha::gcd(true, true));
    CHECK((std::is_same<bool, decltype(piranha::gcd(true, true))>::value));
    CHECK(piranha::gcd(true, 45) == 1);
    CHECK(piranha::gcd(-45, true) == 1);
    CHECK(piranha::gcd(false, 45) == 45);
    CHECK(piranha::gcd(-45, false) == 45);
    CHECK((std::is_same<int, decltype(piranha::gcd(45, true))>::value));
    CHECK((std::is_same<int, decltype(piranha::gcd(true, 45))>::value));
    // Check with different signs.
    CHECK(piranha::gcd(27, -54) == 27);
    CHECK(piranha::gcd(-54, 27) == 27);
    CHECK(piranha::gcd(4, -25) == 1);
    CHECK(piranha::gcd(-25, 4) == 1);
    CHECK(piranha::gcd(-25, 1) == 1);
    CHECK(piranha::gcd(25, -1) == 1);
    CHECK(piranha::gcd(-24, 36) == 12);
    CHECK(piranha::gcd(24, -36) == 12);
    // Check with zeroes.
    CHECK(piranha::gcd(54, 0) == 54);
    CHECK(piranha::gcd(0, 54) == 54);
    CHECK(piranha::gcd(0, 0) == 0);
    // The ternary form, check particularily with respect to short ints.
    int out;
    piranha::gcd3(out, 12, -9);
    CHECK(out == 3);
    piranha::gcd3(out, 12, 0);
    CHECK(out == 12);
    piranha::gcd3(out, 0, 12);
    CHECK(out == 12);
    piranha::gcd3(out, 0, 0);
    CHECK(out == 0);
    short s_out;
    piranha::gcd3(s_out, short(-12), short(9));
    CHECK(s_out == 3);
    char c_out;
    piranha::gcd3(c_out, char(-12), char(-9));
    CHECK(c_out == char(3));
    // Random testing.
    std::uniform_int_distribution<long> int_dist(-100000, 100000);
    for (int i = 0; i < ntries; ++i) {
        long a(int_dist(rng)), b(int_dist(rng));
        long g = piranha::gcd(a, b);
        long c;
        piranha::gcd3(c, a, b);
        if (g == 0) {
            continue;
        }
        CHECK(a % g == 0);
        CHECK(b % g == 0);
        CHECK(g == mppp::gcd(integer(a), integer(b)));
        CHECK(g == c);
    }
}
