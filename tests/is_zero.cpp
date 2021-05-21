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

#include <piranha/math/is_zero.hpp>

#include <complex>
#include <string>

#include "catch.hpp"

struct trivial {
};

struct trivial_a {
};

struct trivial_b {
};

struct trivial_c {
};

struct trivial_d {
};

struct foo {
};

namespace piranha
{

template <>
class is_zero_impl<trivial_a>
{
public:
    char operator()(const trivial_a &) const;
};

template <>
class is_zero_impl<trivial_b>
{
public:
    char operator()(const trivial_a &) const;
};

template <>
class is_zero_impl<trivial_c>
{
public:
    std::string operator()(const trivial_c &) const;
};

template <>
class is_zero_impl<trivial_d>
{
public:
    trivial_d operator()(const trivial_d &) const;
};

template <>
class is_zero_impl<foo>
{
public:
    bool operator()(const foo &) const;
    bool operator()(foo &) const = delete;
};
}

using namespace piranha;

TEST_CASE("is_zero_test_00")
{
    CHECK(!is_is_zero_type<void>::value);
    CHECK(!is_is_zero_type<std::string>::value);
    CHECK(is_is_zero_type<int>::value);
    CHECK(is_is_zero_type<const long>::value);
    CHECK(is_is_zero_type<const unsigned long &>::value);
    CHECK(is_is_zero_type<float &&>::value);
    CHECK(is_is_zero_type<std::complex<float>>::value);
    CHECK(is_is_zero_type<std::complex<double> &>::value);
    CHECK(is_is_zero_type<const std::complex<long double> &>::value);
    CHECK(piranha::is_zero(0));
    CHECK(!piranha::is_zero(1u));
    CHECK(piranha::is_zero(0.));
    CHECK(!piranha::is_zero(1.23l));
    CHECK(piranha::is_zero(std::complex<float>{0, 0}));
    CHECK(!piranha::is_zero(std::complex<double>{-1, 0}));
    CHECK(!piranha::is_zero(std::complex<double>{0, -1}));
    CHECK(!piranha::is_zero(std::complex<double>{1, 1}));
    CHECK((!is_is_zero_type<trivial>::value));
    CHECK((!is_is_zero_type<trivial &>::value));
    CHECK((!is_is_zero_type<trivial &&>::value));
    CHECK((!is_is_zero_type<const trivial &&>::value));
    CHECK(is_is_zero_type<trivial_a>::value);
    CHECK(is_is_zero_type<trivial_a &>::value);
    CHECK(!is_is_zero_type<trivial_b>::value);
    CHECK(!is_is_zero_type<trivial_c>::value);
    CHECK(!is_is_zero_type<trivial_d>::value);
    CHECK((is_is_zero_type<foo>::value));
    CHECK((is_is_zero_type<const foo>::value));
    CHECK((is_is_zero_type<const foo &>::value));
    CHECK((!is_is_zero_type<foo &>::value));
}
