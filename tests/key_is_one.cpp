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

#include <piranha/key/key_is_one.hpp>

#include <string>

#include <piranha/symbol_utils.hpp>

#include "catch.hpp"

using namespace piranha;

struct bar {
};

struct baz {
};

struct foo {
};

namespace piranha
{

template <>
class key_is_one_impl<bar>
{
public:
    bool operator()(const bar &, const symbol_fset &) const
    {
        return true;
    }
};

template <>
class key_is_one_impl<baz>
{
public:
    std::string operator()(const baz &, const symbol_fset &) const;
};

template <>
class key_is_one_impl<foo>
{
public:
    bool operator()(const foo &, const symbol_fset &) const;
    bool operator()(foo &, const symbol_fset &) const = delete;
};
}

TEST_CASE("key_is_one_test_00")
{
    CHECK(!is_key_is_one_type<int>::value);
    CHECK(!is_key_is_one_type<const int>::value);
    CHECK(!is_key_is_one_type<const int &>::value);
    CHECK(!is_key_is_one_type<int &&>::value);
    CHECK(is_key_is_one_type<bar>::value);
    CHECK(is_key_is_one_type<const bar>::value);
    CHECK(is_key_is_one_type<const bar &>::value);
    CHECK(is_key_is_one_type<bar &&>::value);
    CHECK(!is_key_is_one_type<baz>::value);
    CHECK(!is_key_is_one_type<const baz>::value);
    CHECK(!is_key_is_one_type<const baz &>::value);
    CHECK(!is_key_is_one_type<baz &&>::value);
    CHECK(!is_key_is_one_type<void>::value);
    CHECK(is_key_is_one_type<foo>::value);
    CHECK(is_key_is_one_type<const foo>::value);
    CHECK(is_key_is_one_type<const foo &>::value);
    CHECK(is_key_is_one_type<foo &&>::value);
    CHECK(!is_key_is_one_type<foo &>::value);
    CHECK(!is_key_is_one_type<void>::value);
}
