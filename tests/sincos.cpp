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

#include <piranha/math/cos.hpp>
#include <piranha/math/sin.hpp>

#include <boost/algorithm/string/predicate.hpp>

#include <cmath>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

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
class sin_impl<b_00>
{
public:
    b_00 operator()(const b_00 &) const;
};

template <>
class sin_impl<b_01>
{
public:
    b_01 operator()(const b_01 &) const;
};

template <>
class cos_impl<b_00>
{
public:
    b_00 operator()(const b_00 &) const;
};

template <>
class cos_impl<b_01>
{
public:
    b_01 operator()(const b_01 &) const;
};

template <>
class cos_impl<foo>
{
public:
    void operator()(const foo &) const;
    void operator()(foo &) const = delete;
};

template <>
class sin_impl<foo>
{
public:
    void operator()(const foo &) const;
    void operator()(foo &) const = delete;
};
}

TEST_CASE("sin_test_00")
{
    CHECK(!is_sine_type<std::string>::value);
    CHECK(!is_sine_type<void>::value);
    CHECK(!is_sine_type<void>::value);
    CHECK(!is_sine_type<b_00>::value);
    CHECK(!is_sine_type<b_01>::value);
    CHECK(is_sine_type<float>::value);
    CHECK(is_sine_type<double>::value);
    CHECK(is_sine_type<long double>::value);
    CHECK(is_sine_type<float &>::value);
    CHECK(is_sine_type<const double>::value);
    CHECK(is_sine_type<const long double &&>::value);
    CHECK(is_sine_type<int>::value);
    CHECK(is_sine_type<const unsigned>::value);
    CHECK(is_sine_type<long>::value);
    CHECK(is_sine_type<long long &>::value);
    CHECK(is_sine_type<const char &>::value);
    CHECK(piranha::sin(4.5f) == std::sin(4.5f));
    CHECK(piranha::sin(4.5) == std::sin(4.5));
    CHECK(piranha::sin(4.5l) == std::sin(4.5l));
    CHECK((std::is_same<decltype(piranha::sin(4.5f)), float>::value));
    CHECK((std::is_same<decltype(piranha::sin(4.5)), double>::value));
    CHECK((std::is_same<decltype(piranha::sin(4.5l)), long double>::value));
    CHECK(piranha::sin(0) == 0);
    CHECK(piranha::sin(0u) == 0u);
    CHECK(piranha::sin(static_cast<signed char>(0)) == static_cast<signed char>(0));
    CHECK((std::is_same<decltype(piranha::sin(0)), int>::value));
    CHECK((std::is_same<decltype(piranha::sin(0u)), unsigned>::value));
    CHECK((std::is_same<decltype(piranha::sin(static_cast<signed char>(0))), signed char>::value));
    CHECK_THROWS_MATCHES(piranha::sin(42), std::domain_error,
        test::ExceptionMatcher<std::domain_error>(std::string("cannot compute the sine of the non-zero C++ integral 42"))
    );
    CHECK(is_sine_type<foo>::value);
    CHECK(is_sine_type<const foo &>::value);
    CHECK(is_sine_type<const foo &&>::value);
    CHECK(is_sine_type<foo &&>::value);
    CHECK(!is_sine_type<foo &>::value);
}

TEST_CASE("cos_test_00")
{
    CHECK(!is_cosine_type<std::string>::value);
    CHECK(!is_cosine_type<void>::value);
    CHECK(!is_cosine_type<void>::value);
    CHECK(!is_cosine_type<b_00>::value);
    CHECK(!is_cosine_type<b_01>::value);
    CHECK(is_cosine_type<float>::value);
    CHECK(is_cosine_type<double>::value);
    CHECK(is_cosine_type<long double>::value);
    CHECK(is_cosine_type<float &>::value);
    CHECK(is_cosine_type<const double>::value);
    CHECK(is_cosine_type<const long double &&>::value);
    CHECK(is_cosine_type<int>::value);
    CHECK(is_cosine_type<const unsigned>::value);
    CHECK(is_cosine_type<long>::value);
    CHECK(is_cosine_type<long long &>::value);
    CHECK(is_cosine_type<const char &>::value);
    CHECK(piranha::cos(4.5f) == std::cos(4.5f));
    CHECK(piranha::cos(4.5) == std::cos(4.5));
    CHECK(piranha::cos(4.5l) == std::cos(4.5l));
    CHECK((std::is_same<decltype(piranha::cos(4.5f)), float>::value));
    CHECK((std::is_same<decltype(piranha::cos(4.5)), double>::value));
    CHECK((std::is_same<decltype(piranha::cos(4.5l)), long double>::value));
    CHECK(piranha::cos(0) == 1);
    CHECK(piranha::cos(0u) == 1u);
    CHECK(piranha::cos(static_cast<signed char>(0)) == static_cast<signed char>(1));
    CHECK((std::is_same<decltype(piranha::cos(0)), int>::value));
    CHECK((std::is_same<decltype(piranha::cos(0u)), unsigned>::value));
    CHECK((std::is_same<decltype(piranha::cos(static_cast<signed char>(0))), signed char>::value));
    CHECK_THROWS_MATCHES(piranha::cos(42), std::domain_error,
        test::ExceptionMatcher<std::domain_error>(std::string("cannot compute the cosine of the non-zero C++ integral 42"))
    );
    CHECK(is_cosine_type<foo>::value);
    CHECK(is_cosine_type<const foo &>::value);
    CHECK(is_cosine_type<const foo &&>::value);
    CHECK(is_cosine_type<foo &&>::value);
    CHECK(!is_cosine_type<foo &>::value);
}
