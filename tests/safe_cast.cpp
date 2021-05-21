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

#include <piranha/safe_cast.hpp>

#include <iterator>
#include <string>
#include <type_traits>

#include <boost/algorithm/string/predicate.hpp>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

TEST_CASE("safe_cast_test_00")
{
    CHECK((is_safely_castable<int &&, int>::value));
    CHECK((is_safely_castable<const float, long>::value));
    CHECK((is_safely_castable<const double, long>::value));
    CHECK((!is_safely_castable<const double, void>::value));
    CHECK((!is_safely_castable<void, void>::value));

    CHECK(safe_cast<unsigned>(5) == 5u);
    CHECK((std::is_same<decltype(safe_cast<unsigned>(5)), unsigned>::value));
    CHECK_THROWS_MATCHES(safe_cast<unsigned>(-5), safe_cast_failure,
        test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
    );
    CHECK(safe_cast<int>(123.) == 123);
    CHECK((std::is_same<decltype(safe_cast<int>(123.)), int>::value));
    CHECK_THROWS_MATCHES(safe_cast<int>(123.456), safe_cast_failure,
        test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
    );
}

TEST_CASE("safe_cast_input_iterator")
{
    CHECK((!is_safely_castable_input_iterator<void, void>::value));
    CHECK((!is_safely_castable_input_iterator<std::vector<int>::iterator, void>::value));
    CHECK((!is_safely_castable_input_iterator<void, int>::value));
    CHECK((is_safely_castable_input_iterator<std::vector<int>::iterator, short>::value));
    CHECK((is_safely_castable_input_iterator<std::istreambuf_iterator<char>, short>::value));
    CHECK((!is_safely_castable_input_iterator<std::vector<int>::iterator, std::string>::value));
}

TEST_CASE("safe_cast_forward_iterator")
{
    CHECK((!is_safely_castable_forward_iterator<void, void>::value));
    CHECK((!is_safely_castable_forward_iterator<std::vector<int>::iterator, void>::value));
    CHECK((!is_safely_castable_forward_iterator<void, int>::value));
    CHECK((is_safely_castable_forward_iterator<std::vector<int>::iterator, short>::value));
    CHECK((!is_safely_castable_forward_iterator<std::istreambuf_iterator<char>, short>::value));
    CHECK((!is_safely_castable_forward_iterator<std::vector<int>::iterator, std::string>::value));
}

TEST_CASE("safe_cast_mutable_forward_iterator")
{
    CHECK((!is_safely_castable_mutable_forward_iterator<void, void>::value));
    CHECK((!is_safely_castable_mutable_forward_iterator<std::vector<int>::iterator, void>::value));
    CHECK((!is_safely_castable_mutable_forward_iterator<void, int>::value));
    CHECK((is_safely_castable_mutable_forward_iterator<std::vector<int>::iterator, short>::value));
    CHECK((!is_safely_castable_mutable_forward_iterator<std::vector<int>::const_iterator, short>::value));
    CHECK((!is_safely_castable_mutable_forward_iterator<std::istreambuf_iterator<char>, short>::value));
    CHECK((!is_safely_castable_mutable_forward_iterator<std::vector<int>::iterator, std::string>::value));
}

TEST_CASE("safe_cast_input_range")
{
    CHECK((!is_safely_castable_input_range<void, void>::value));
    CHECK((!is_safely_castable_input_range<std::vector<int> &, void>::value));
    CHECK((!is_safely_castable_input_range<void, int>::value));
    CHECK((is_safely_castable_input_range<std::vector<int> &, short>::value));
    CHECK((is_safely_castable_input_range<int(&)[3], short>::value));
    CHECK((!is_safely_castable_input_range<std::vector<int> &, std::string>::value));
}

struct foo0 {
};

std::istreambuf_iterator<char> begin(const foo0 &);
std::istreambuf_iterator<char> end(const foo0 &);

TEST_CASE("safe_cast_forward_range")
{
    CHECK((!is_safely_castable_forward_range<void, void>::value));
    CHECK((!is_safely_castable_forward_range<std::vector<int> &, void>::value));
    CHECK((!is_safely_castable_forward_range<void, int>::value));
    CHECK((is_safely_castable_forward_range<std::vector<int> &, short>::value));
    CHECK((is_safely_castable_forward_range<int(&)[3], short>::value));
    CHECK((!is_safely_castable_forward_range<std::vector<int> &, std::string>::value));
    CHECK((is_safely_castable_input_range<foo0 &, int>::value));
    CHECK((!is_safely_castable_forward_range<foo0 &, int>::value));
}

TEST_CASE("safe_cast_mutable_forward_range")
{
    CHECK((!is_safely_castable_mutable_forward_range<void, void>::value));
    CHECK((!is_safely_castable_mutable_forward_range<std::vector<int> &, void>::value));
    CHECK((!is_safely_castable_mutable_forward_range<void, int>::value));
    CHECK((is_safely_castable_mutable_forward_range<std::vector<int> &, short>::value));
    CHECK((is_safely_castable_mutable_forward_range<int(&)[3], short>::value));
    CHECK((!is_safely_castable_mutable_forward_range<const int(&)[3], short>::value));
    CHECK((!is_safely_castable_mutable_forward_range<const std::vector<int> &, short>::value));
    CHECK((!is_safely_castable_mutable_forward_range<std::vector<int> &, std::string>::value));
    CHECK((!is_safely_castable_mutable_forward_range<foo0 &, int>::value));
}
