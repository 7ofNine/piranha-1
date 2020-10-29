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

#include <string>

#include <piranha/symbol_utils.hpp>
#include <piranha/key/key_is_zero.hpp>

#include "catch.hpp"

using namespace piranha;

struct foo {
};

struct bar {
};

struct baz {
};

struct nab {
};

namespace piranha
{

template <>
class key_is_zero_impl<bar>
{
public:
    bool operator()(const bar &, const symbol_fset &) const
    {
        return true;
    }
};

template <>
class key_is_zero_impl<baz>
{
public:
    std::string operator()(const baz &, const symbol_fset &) const;
};

template <>
class key_is_zero_impl<nab>
{
public:
    bool operator()(const nab &, const symbol_fset &) const;
    bool operator()(nab &, const symbol_fset &) const = delete;
};
}

TEST_CASE("key_is_zero_test_00")
{
    CHECK(!key_is_zero(0, symbol_fset{}));
    CHECK(!key_is_zero(foo{}, symbol_fset{}));
    CHECK(key_is_zero(bar{}, symbol_fset{}));
    CHECK(is_key_is_zero_type<int>::value);
    CHECK(is_key_is_zero_type<const int>::value);
    CHECK(is_key_is_zero_type<const int &>::value);
    CHECK(is_key_is_zero_type<int &&>::value);
    CHECK(is_key_is_zero_type<foo>::value);
    CHECK(is_key_is_zero_type<const foo>::value);
    CHECK(is_key_is_zero_type<const foo &>::value);
    CHECK(is_key_is_zero_type<foo &&>::value);
    CHECK(is_key_is_zero_type<bar>::value);
    CHECK(is_key_is_zero_type<const bar>::value);
    CHECK(is_key_is_zero_type<const bar &>::value);
    CHECK(is_key_is_zero_type<bar &&>::value);
    CHECK(!is_key_is_zero_type<baz>::value);
    CHECK(!is_key_is_zero_type<const baz>::value);
    CHECK(!is_key_is_zero_type<const baz &>::value);
    CHECK(!is_key_is_zero_type<baz &&>::value);
    CHECK(!is_key_is_zero_type<void>::value);
    CHECK(is_key_is_zero_type<nab>::value);
    CHECK(is_key_is_zero_type<const nab>::value);
    CHECK(is_key_is_zero_type<const nab &>::value);
    CHECK(is_key_is_zero_type<nab &&>::value);
    CHECK(!is_key_is_zero_type<nab &>::value);
}
