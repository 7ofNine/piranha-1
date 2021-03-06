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

#include <piranha/math/ldegree.hpp>


#include <utility>

#include <piranha/symbol_utils.hpp>

#include "catch.hpp"

using namespace piranha;

class foo
{
};

class bar
{
};

struct mbar {
    mbar() = default;
    mbar(mbar &&m)
    {
        m.value = 1;
    }
    int value = 0;
};

namespace piranha
{

template <>
class ldegree_impl<foo>
{
public:
    int operator()(const foo &) const
    {
        return 0;
    }
    int operator()(const foo &, const symbol_fset &) const
    {
        return 1;
    }
};

// bar is missing the partial degree overload.
template <>
class ldegree_impl<bar>
{
public:
    int operator()(const bar &) const
    {
        return 0;
    }
};

template <>
class ldegree_impl<mbar>
{
public:
    template <typename T, typename... Args>
    int operator()(T &&x, const Args &...) const
    {
        T other(std::forward<T>(x));
        (void)other;
        return sizeof...(Args);
    }
};
}

TEST_CASE("ldegree_test_00")
{
    CHECK(!is_ldegree_type<void>::value);
    CHECK(!is_ldegree_type<int>::value);
    CHECK(!is_ldegree_type<const int>::value);
    CHECK(!is_ldegree_type<const int &&>::value);
    CHECK(!is_ldegree_type<int &&>::value);
    CHECK(is_ldegree_type<foo>::value);
    CHECK(is_ldegree_type<foo &>::value);
    CHECK(is_ldegree_type<const foo>::value);
    CHECK(is_ldegree_type<const foo &>::value);
    CHECK(is_ldegree_type<foo &&>::value);
    CHECK(piranha::ldegree(foo{}) == 0);
    CHECK(piranha::ldegree(foo{}, symbol_fset{}) == 1);
    CHECK(!is_ldegree_type<bar>::value);
    CHECK(!is_ldegree_type<bar &>::value);
    CHECK(!is_ldegree_type<const bar>::value);
    CHECK(!is_ldegree_type<const bar &>::value);
    CHECK(!is_ldegree_type<bar &&>::value);
    CHECK(is_ldegree_type<mbar>::value);
    CHECK(piranha::ldegree(mbar{}) == 0);
    CHECK(piranha::ldegree(mbar{}, symbol_fset{}) == 1);
    mbar m1, m2;
    CHECK(m1.value == 0);
    CHECK(m2.value == 0);
    piranha::ldegree(std::move(m1));
    piranha::ldegree(std::move(m2), symbol_fset{});
    CHECK(m1.value == 1);
    CHECK(m2.value == 1);
}
