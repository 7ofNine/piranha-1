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

#include <piranha/key_is_multipliable.hpp>


#include <array>
#include <cstddef>
#include <functional>
#include <iostream>
#include <vector>

#include <piranha/is_key.hpp>
#include <piranha/key/key_is_one.hpp>
#include <piranha/symbol_utils.hpp>

#include "catch.hpp"

using namespace piranha;

// Mock key with no method.
struct mock_key {
    mock_key() = default;
    mock_key(const mock_key &) = default;
    mock_key(mock_key &&) noexcept;
    mock_key &operator=(const mock_key &) = default;
    mock_key &operator=(mock_key &&) noexcept;
    mock_key(const symbol_fset &);
    bool operator==(const mock_key &) const;
    bool operator!=(const mock_key &) const;
    bool is_compatible(const symbol_fset &) const noexcept;
    mock_key merge_symbols(const symbol_idx_fmap<symbol_fset> &, const symbol_fset &) const;
    void print(std::ostream &, const symbol_fset &) const;
    void print_tex(std::ostream &, const symbol_fset &) const;
    void trim_identify(std::vector<char> &, const symbol_fset &) const;
    mock_key trim(const std::vector<char> &, const symbol_fset &) const;
};

// Mock key with wrong method.
struct mock_key_00 {
    static const std::size_t multiply_arity = 1u;
    mock_key_00() = default;
    mock_key_00(const mock_key_00 &) = default;
    mock_key_00(mock_key_00 &&) noexcept;
    mock_key_00 &operator=(const mock_key_00 &) = default;
    mock_key_00 &operator=(mock_key_00 &&) noexcept;
    mock_key_00(const symbol_fset &);
    bool operator==(const mock_key_00 &) const;
    bool operator!=(const mock_key_00 &) const;
    bool is_compatible(const symbol_fset &) const noexcept;
    mock_key_00 merge_symbols(const symbol_idx_fmap<symbol_fset> &, const symbol_fset &) const;
    void print(std::ostream &, const symbol_fset &) const;
    void print_tex(std::ostream &, const symbol_fset &) const;
    template <typename Cf>
    static void multiply(std::array<term<Cf, mock_key_00>, 1u> &, term<Cf, mock_key_00> &,
                         const term<Cf, mock_key_00> &, const symbol_fset &);
    void trim_identify(std::vector<char> &, const symbol_fset &) const;
    mock_key_00 trim(const std::vector<char> &, const symbol_fset &) const;
};

// Good one, depending on coefficient.
struct mock_key_01 {
    static const std::size_t multiply_arity = 4u;
    mock_key_01() = default;
    mock_key_01(const mock_key_01 &) = default;
    mock_key_01(mock_key_01 &&) noexcept;
    mock_key_01 &operator=(const mock_key_01 &) = default;
    mock_key_01 &operator=(mock_key_01 &&) noexcept;
    mock_key_01(const symbol_fset &);
    bool operator==(const mock_key_01 &) const;
    bool operator!=(const mock_key_01 &) const;
    bool is_compatible(const symbol_fset &) const noexcept;
    mock_key_01 merge_symbols(const symbol_idx_fmap<symbol_fset> &, const symbol_fset &) const;
    void print(std::ostream &, const symbol_fset &) const;
    void print_tex(std::ostream &, const symbol_fset &) const;
    static void multiply(std::array<term<double, mock_key_01>, 4u> &, const term<double, mock_key_01> &,
                         const term<double, mock_key_01> &, const symbol_fset &);
    void trim_identify(std::vector<char> &, const symbol_fset &) const;
    mock_key_01 trim(const std::vector<char> &, const symbol_fset &) const;
};

namespace std
{

template <>
struct hash<mock_key> {
    std::size_t operator()(const mock_key &) const;
};

template <>
struct hash<mock_key_00> {
    std::size_t operator()(const mock_key_00 &) const;
};

template <>
struct hash<mock_key_01> {
    std::size_t operator()(const mock_key_01 &) const;
};
}

namespace piranha
{

template <>
class key_is_one_impl<mock_key>
{
public:
    bool operator()(const mock_key &, const symbol_fset &) const;
};

template <>
class key_is_one_impl<mock_key_00>
{
public:
    bool operator()(const mock_key_00 &, const symbol_fset &) const;
};

template <>
class key_is_one_impl<mock_key_01>
{
public:
    bool operator()(const mock_key_01 &, const symbol_fset &) const;
};
}

TEST_CASE("key_is_multipliable_test_00")
{
    CHECK(is_key<mock_key>::value);
    CHECK(!(key_is_multipliable<double, mock_key>::value));
    CHECK(is_key<mock_key_00>::value);
    CHECK(!(key_is_multipliable<double, mock_key_00>::value));
    CHECK(is_key<mock_key_01>::value);
    CHECK((key_is_multipliable<double, mock_key_01>::value));
    CHECK(!(key_is_multipliable<float, mock_key_01>::value));
}
