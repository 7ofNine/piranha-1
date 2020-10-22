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

#include <piranha/detail/demangle.hpp>


#include <iostream>
#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_set>
#include <vector>

#include "catch.hpp"
using namespace piranha;

struct base_foo {
    virtual void f() const {}
    virtual ~base_foo() {}
};

struct foo : base_foo {
};

namespace myns
{

template <typename T>
struct bar {
};
}

TEST_CASE("demangle_test")
{
    // A few valid types.
    std::cout << type_name<int>() << '\n';
    std::cout << type_name<std::vector<int>>() << '\n';
    std::cout << type_name<base_foo>() << '\n';
    std::cout << type_name<foo>() << '\n';
    std::cout << type_name<myns::bar<int>>() << '\n';
    // Check with dynamic polymorphism.
    std::unique_ptr<base_foo> foo_ptr(new foo{});
    auto ptr = foo_ptr.get();
}
