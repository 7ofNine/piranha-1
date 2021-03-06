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

#include <piranha/kronecker_monomial.hpp>

#include <array>
#include <boost/algorithm/string/predicate.hpp>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include <mp++/config.hpp>

#include <piranha/detail/demangle.hpp>
#include <piranha/exceptions.hpp>
#include <piranha/integer.hpp>
#include <piranha/is_key.hpp>
#include <piranha/key/key_degree.hpp>
#include <piranha/key/key_is_one.hpp>
#include <piranha/key/key_ldegree.hpp>
#include <piranha/key_is_convertible.hpp>
#include <piranha/key_is_multipliable.hpp>
#include <piranha/kronecker_array.hpp>
#include <piranha/math.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/safe_cast.hpp>
#include <piranha/symbol_utils.hpp>
#include <piranha/term.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

using int_types = std::tuple<signed char, int, long, long long>;

// Constructors, assignments, getters, setters, etc.
struct constructor_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        typedef kronecker_array<T> ka;
        k_type k1;
        CHECK(k1.get_int() == 0);
        k_type k2({-1, -1});
        std::vector<T> v2(2);
        ka::decode(v2, k2.get_int());
        CHECK(v2[0] == -1);
        CHECK(v2[1] == -1);
        k_type k3({});
        CHECK(k3.get_int() == 0);
        k_type k4({10});
        CHECK(k4.get_int() == 10);
        // Ctor from container.
        k1 = k_type(std::vector<int>{});
        CHECK(k1.get_int() == 0);
        k1 = k_type(std::vector<int>{12});
        CHECK(k1.get_int() == 12);
        k1 = k_type(std::vector<int>{-1, 2});
        ka::decode(v2, k1.get_int());
        CHECK(v2[0] == -1);
        CHECK(v2[1] == 2);
        k1 = k_type(std::list<int>{});
        CHECK(k1.get_int() == 0);
        k1 = k_type(std::list<int>{12});
        CHECK(k1.get_int() == 12);
        k1 = k_type(std::list<int>{-1, 2});
        ka::decode(v2, k1.get_int());
        CHECK(v2[0] == -1);
        CHECK(v2[1] == 2);
        // Ctor from symbol set.
        k_type k5(symbol_fset{});
        CHECK(k5.get_int() == 0);
        k_type k6(symbol_fset{"a"});
        CHECK(k6.get_int() == 0);
        k_type k7(symbol_fset{"a", "b"});
        CHECK(k7.get_int() == 0);
        k_type k8(0);
        CHECK(k8.get_int() == 0);
        k_type k9(1);
        CHECK(k9.get_int() == 1);
        k_type k10;
        k10.set_int(10);
        CHECK(k10.get_int() == 10);
        k_type k11;
        k11 = k10;
        CHECK(k11.get_int() == 10);
        k11 = std::move(k9);
        CHECK(k9.get_int() == 1);
        // Constructor from iterators.
        v2 = {};
        k_type k12(v2.begin(), v2.end());
        CHECK(k12.get_int() == 0);
        v2 = {21};
        k_type k13(v2.begin(), v2.end());
        CHECK(k13.get_int() == 21);
        v2 = {-21};
        k_type k14(v2.begin(), v2.end());
        CHECK(k14.get_int() == -21);
        v2 = {1, -2};
        k_type k15(v2.begin(), v2.end());
        auto v = k15.unpack(symbol_fset{"a", "b"});
        CHECK(v.size() == 2u);
        CHECK(v[0u] == 1);
        CHECK(v[1u] == -2);
        CHECK((std::is_constructible<k_type, T *, T *>::value));
        // Ctor from range and symbol set.
        v2 = {};
        k1 = k_type(v2.begin(), v2.end(), symbol_fset{});
        CHECK(k1.get_int() == 0);
        v2 = {-3};
        k1 = k_type(v2.begin(), v2.end(), symbol_fset{"x"});
        CHECK(k1.get_int() == -3);
        CHECK_THROWS_MATCHES(
            k1 = k_type(v2.begin(), v2.end(), symbol_fset{}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the Kronecker monomial constructor from range and symbol set "
                    "yielded an invalid monomial: the range length (1) differs from the size of the symbol set (0)"))
        );
        v2 = {-1, 0};
        k1 = k_type(v2.begin(), v2.end(), symbol_fset{"x", "y"});
        ka::decode(v2, k1.get_int());
        CHECK(v2[0] == -1);
        CHECK(v2[1] == 0);
        std::list<int> l2;
        k1 = k_type(l2.begin(), l2.end(), symbol_fset{});
        CHECK(k1.get_int() == 0);
        l2 = {-3};
        k1 = k_type(l2.begin(), l2.end(), symbol_fset{"x"});
        CHECK(k1.get_int() == -3);
        CHECK_THROWS_MATCHES(
            k1 = k_type(l2.begin(), l2.end(), symbol_fset{}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the Kronecker monomial constructor from range and symbol set "
                    "yielded an invalid monomial: the range length (1) differs from the size of the symbol set (0)"))
        );
        l2 = {-1, 0};
        k1 = k_type(l2.begin(), l2.end(), symbol_fset{"x", "y"});
        ka::decode(v2, k1.get_int());
        CHECK(v2[0] == -1);
        CHECK(v2[1] == 0);
        struct foo {
        };
        CHECK((std::is_constructible<k_type, int *, int *, symbol_fset>::value));
        CHECK((!std::is_constructible<k_type, foo *, foo *, symbol_fset>::value));
        // Iterators have to be of homogeneous type.
        CHECK((!std::is_constructible<k_type, T *, T const *>::value));
        CHECK((std::is_constructible<k_type, typename std::vector<T>::iterator,
                                           typename std::vector<T>::iterator>::value));
        CHECK((!std::is_constructible<k_type, typename std::vector<T>::const_iterator,
                                            typename std::vector<T>::iterator>::value));
        CHECK((!std::is_constructible<k_type, typename std::vector<T>::iterator, int>::value));
        // Converting constructor.
        k_type k16, k17(k16, symbol_fset{});
        CHECK(k16 == k17);
        k16.set_int(10);
        k_type k18(k16, symbol_fset{"a"});
        CHECK(k16 == k18);
    }
};

TEST_CASE("kronecker_monomial_constructor_test")
{
    tuple_for_each(int_types{}, constructor_tester{});
}

struct compatibility_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        typedef kronecker_array<T> ka;
        const auto &limits = ka::get_limits();
        k_type k1;
        CHECK(k1.is_compatible(symbol_fset{}));
        k1.set_int(1);
        CHECK(!k1.is_compatible(symbol_fset{}));
        if (limits.size() < 255u) {
            symbol_fset v2;
            for (auto i = 0u; i < 255; ++i) {
                v2.insert(std::string(1u, static_cast<char>(i)));
            }
            CHECK(!k1.is_compatible(v2));
        }
        k1.set_int(std::numeric_limits<T>::max());
        CHECK(!k1.is_compatible(symbol_fset{"a", "b"}));
        k1.set_int(-1);
        CHECK(k1.is_compatible(symbol_fset{"a", "b"}));
    }
};

TEST_CASE("kronecker_monomial_compatibility_test")
{
    tuple_for_each(int_types{}, compatibility_tester{});
}

struct merge_args_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        k_type k1;
        CHECK_THROWS_MATCHES(
            k1.merge_symbols({}, symbol_fset{}), std::invalid_argument, 
                test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for symbol set merging: the insertion map cannot be empty"))
        );
        CHECK_THROWS_MATCHES(
            k1.merge_symbols({}, symbol_fset{"d"}), std::invalid_argument, 
                test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for symbol set merging: the insertion map cannot be empty"))
        );
        CHECK((k1.merge_symbols({{0, {"a", "b"}}}, symbol_fset{"d"}) == k_type{0, 0, 0}));
        CHECK((k_type{1}.merge_symbols({{0, {"a", "b"}}}, symbol_fset{"d"}) == k_type{0, 0, 1}));
        CHECK((k_type{1}.merge_symbols({{1, {"e", "f"}}}, symbol_fset{"d"}) == k_type{1, 0, 0}));
        CHECK((k_type{1, 1}.merge_symbols({{0, {"a", "b"}}}, symbol_fset{"d", "n"}) == k_type{0, 0, 1, 1}));
        CHECK((k_type{1, 1}.merge_symbols({{1, {"e", "f"}}}, symbol_fset{"d", "n"}) == k_type{1, 0, 0, 1}));
        CHECK((k_type{1, 1}.merge_symbols({{2, {"f", "g"}}}, symbol_fset{"d", "e"}) == k_type{1, 1, 0, 0}));
        CHECK(
            (k_type{-1, -1}.merge_symbols({{0, {"a"}}, {2, {"f"}}}, symbol_fset{"d", "e"}) == k_type{0, -1, -1, 0}));
        CHECK((k_type{-1, -1}.merge_symbols({{0, {"a"}}, {1, std::initializer_list<std::string>{}}, {2, {"f"}}},
                                                  symbol_fset{"d", "e"})
                     == k_type{0, -1, -1, 0}));
        CHECK_THROWS_MATCHES((k_type{1, 1}.merge_symbols({{3, {"f", "g"}}}, symbol_fset{"d", "e"})),
                              std::invalid_argument,
                        test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for symbol set merging: the "
                                                                   "last index of the insertion map (3) must not be "
                                                                   "greater than the key's size (2)"))
        );
        if (std::numeric_limits<T>::max() >= std::numeric_limits<int>::max()) {
            CHECK((k_type{-1, -1}.merge_symbols({{0, {"a"}}, {2, {"f"}}, {1, {"b"}}}, symbol_fset{"d", "e"})
                         == k_type{0, -1, 0, -1, 0}));
            CHECK(
                (k_type{-1, -1, 3}.merge_symbols({{0, {"a"}}, {3, {"f"}}, {1, {"b"}}}, symbol_fset{"d", "e1", "e2"})
                 == k_type{0, -1, 0, -1, 3, 0}));
        }
    }
};

TEST_CASE("kronecker_monomial_merge_args_test")
{
    tuple_for_each(int_types{}, merge_args_tester{});
}

struct key_is_one_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        k_type k1;
        CHECK(piranha::key_is_one(k1, symbol_fset{}));
        k_type k2({-1});
        CHECK(!piranha::key_is_one(k2, symbol_fset{"a"}));
        k_type k3({0});
        CHECK(piranha::key_is_one(k3, symbol_fset{"a"}));
        k_type k4({0, 0});
        CHECK(piranha::key_is_one(k4, symbol_fset{"a", "b"}));
        k_type k5({0, 1});
        CHECK(!piranha::key_is_one(k5, symbol_fset{"a", "b"}));
    }
};

TEST_CASE("kronecker_monomial_key_is_one_test")
{
    tuple_for_each(int_types{}, key_is_one_tester{});
}

struct degree_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        k_type k1;
        if (std::is_same<T, signed char>::value) {
            CHECK((std::is_same<decltype(key_degree(k1, symbol_fset{})), int>::value));
        } else {
            CHECK((std::is_same<decltype(key_degree(k1, symbol_fset{})), T>::value));
        }
        CHECK(key_degree(k1, symbol_fset{}) == 0);
        CHECK(key_ldegree(k1, symbol_fset{}) == 0);
        k_type k2({0});
        CHECK(key_degree(k2, symbol_fset{"a"}) == 0);
        CHECK(key_ldegree(k2, symbol_fset{"a"}) == 0);
        k_type k3({-1});
        CHECK(key_degree(k3, symbol_fset{"a"}) == -1);
        CHECK(key_ldegree(k3, symbol_fset{"a"}) == -1);
        k_type k4({0, 0});
        CHECK(key_degree(k4, symbol_fset{"a", "b"}) == 0);
        CHECK(key_ldegree(k4, symbol_fset{"a", "b"}) == 0);
        k_type k5({-1, -1});
        CHECK(key_degree(k5, symbol_fset{"a", "b"}) == -2);
        if (std::is_same<T, signed char>::value) {
            CHECK((std::is_same<decltype(key_degree(k5, symbol_idx_fset{}, symbol_fset{})), int>::value));
        } else {
            CHECK((std::is_same<decltype(key_degree(k5, symbol_idx_fset{}, symbol_fset{})), T>::value));
        }
        CHECK(key_degree(k5, symbol_idx_fset{0}, symbol_fset{"a", "b"}) == -1);
        CHECK(key_degree(k5, symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(key_degree(k5, symbol_idx_fset{0, 1}, symbol_fset{"a", "b"}) == -2);
        CHECK(key_degree(k5, symbol_idx_fset{1}, symbol_fset{"a", "b"}) == -1);
        CHECK(key_ldegree(k5, symbol_fset{"a", "b"}) == -2);
        CHECK(key_ldegree(k5, symbol_idx_fset{0}, symbol_fset{"a", "b"}) == -1);
        CHECK(key_ldegree(k5, symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(key_ldegree(k5, symbol_idx_fset{0, 1}, symbol_fset{"a", "b"}) == -2);
        CHECK(key_ldegree(k5, symbol_idx_fset{1}, symbol_fset{"a", "b"}) == -1);
        // Try partials with bogus positions.
        CHECK_THROWS_MATCHES(
            (key_degree(k5, symbol_idx_fset{2}, symbol_fset{"a", "b"})), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                              "partial degree of a Kronecker monomial is 2, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            (key_ldegree(k5, symbol_idx_fset{4}, symbol_fset{"a", "b"})), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                              "partial degree of a Kronecker monomial is 4, but the monomial has a size of only 2"))
        );
    }
};

TEST_CASE("kronecker_monomial_degree_test")
{
    tuple_for_each(int_types{}, degree_tester{});
}

// Mock cf with wrong specialisation of mul3.
struct mock_cf3 {
    mock_cf3();
    explicit mock_cf3(const int &);
    mock_cf3(const mock_cf3 &);
    mock_cf3(mock_cf3 &&) noexcept;
    mock_cf3 &operator=(const mock_cf3 &);
    mock_cf3 &operator=(mock_cf3 &&) noexcept;
    friend std::ostream &operator<<(std::ostream &, const mock_cf3 &);
    mock_cf3 operator-() const;
    bool operator==(const mock_cf3 &) const;
    bool operator!=(const mock_cf3 &) const;
    mock_cf3 &operator+=(const mock_cf3 &);
    mock_cf3 &operator-=(const mock_cf3 &);
    mock_cf3 operator+(const mock_cf3 &) const;
    mock_cf3 operator-(const mock_cf3 &) const;
    mock_cf3 &operator*=(const mock_cf3 &);
    mock_cf3 operator*(const mock_cf3 &)const;
};

namespace piranha
{
namespace math
{

template <typename T>
struct mul3_impl<T, typename std::enable_if<std::is_same<T, mock_cf3>::value>::type> {
};
} // namespace math
} // namespace piranha

struct multiply_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        typedef kronecker_array<T> ka;
        using term_type = term<integer, k_type>;
        CHECK((key_is_multipliable<int, k_type>::value));
        CHECK((key_is_multipliable<integer, k_type>::value));
        CHECK((!key_is_multipliable<mock_cf3, k_type>::value));
        CHECK(is_key<k_type>::value);
        term_type t1, t2;
        std::array<term_type, 1u> result;
        k_type::multiply(result, t1, t2, symbol_fset{});
        CHECK(result[0u].m_cf == 0);
        CHECK(result[0u].m_key.get_int() == 0);
        t1.m_cf = 2;
        t2.m_cf = 3;
        t1.m_key = k_type({0});
        t2.m_key = k_type({0});
        k_type::multiply(result, t1, t2, symbol_fset{"a"});
        CHECK(result[0u].m_cf == 6);
        CHECK(result[0u].m_key.get_int() == 0);
        t1.m_key = k_type({1});
        t2.m_key = k_type({2});
        k_type::multiply(result, t1, t2, symbol_fset{"a"});
        CHECK(result[0u].m_cf == 6);
        CHECK(result[0u].m_key.get_int() == 3);
        t1.m_cf = 2;
        t2.m_cf = -4;
        t1.m_key = k_type({1, -1});
        t2.m_key = k_type({2, 0});
        k_type::multiply(result, t1, t2, symbol_fset{"a", "b"});
        CHECK(result[0u].m_cf == -8);
        std::vector<int> tmp(2u);
        ka::decode(tmp, result[0u].m_key.get_int());
        CHECK(tmp[0u] == 3);
        CHECK(tmp[1u] == -1);
        // Check special handling of rational coefficients.
        using term_type2 = term<rational, k_type>;
        term_type2 ta, tb;
        std::array<term_type2, 1u> result2;
        ta.m_cf = 2 / 3_q;
        tb.m_cf = -4 / 5_q;
        ta.m_key = k_type({1, -1});
        tb.m_key = k_type({2, 0});
        k_type::multiply(result2, ta, tb, symbol_fset{"a", "b"});
        CHECK(result2[0u].m_cf == -8);
        ka::decode(tmp, result2[0u].m_key.get_int());
        CHECK(tmp[0u] == 3);
        CHECK(tmp[1u] == -1);
    }
};

TEST_CASE("kronecker_monomial_multiply_test")
{
    tuple_for_each(int_types{}, multiply_tester{});
}

struct equality_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        k_type k1, k2;
        CHECK(k1 == k2);
        CHECK(!(k1 != k2));
        k1 = k_type({0});
        k2 = k_type({0});
        CHECK(k1 == k2);
        CHECK(!(k1 != k2));
        k2 = k_type({1});
        CHECK(!(k1 == k2));
        CHECK(k1 != k2);
        k1 = k_type({0, 0});
        k2 = k_type({0, 0});
        CHECK(k1 == k2);
        CHECK(!(k1 != k2));
        k1 = k_type({1, 0});
        k2 = k_type({1, 0});
        CHECK(k1 == k2);
        CHECK(!(k1 != k2));
        k1 = k_type({1, 0});
        k2 = k_type({0, 1});
        CHECK(!(k1 == k2));
        CHECK(k1 != k2);
    }
};

TEST_CASE("kronecker_monomial_equality_test")
{
    tuple_for_each(int_types{}, equality_tester{});
}

struct hash_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        k_type k1;
        CHECK(k1.hash() == static_cast<std::size_t>(k1.get_int()));
        k1 = k_type({0});
        CHECK(k1.hash() == static_cast<std::size_t>(k1.get_int()));
        k1 = k_type({0, 1});
        CHECK(k1.hash() == static_cast<std::size_t>(k1.get_int()));
        k1 = k_type({0, 1, -1});
        CHECK(k1.hash() == static_cast<std::size_t>(k1.get_int()));
        CHECK(std::hash<k_type>()(k1) == static_cast<std::size_t>(k1.get_int()));
    }
};

TEST_CASE("kronecker_monomial_hash_test")
{
    tuple_for_each(int_types{}, hash_tester{});
}

struct unpack_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        k_type k1({0});
        auto t1 = k1.unpack(symbol_fset{});
        typedef decltype(t1) s_vector_type;
        CHECK(!t1.size());
        k1.set_int(-1);
        auto t2 = k1.unpack(symbol_fset{"a"});
        CHECK(t2.size());
        CHECK(t2[0u] == -1);
        // Check for overflow condition.
        symbol_fset vs1{"a"};
        std::string tmp = "";
        for (integer i(0u); i < integer(s_vector_type::max_size) + 1; ++i) {
            tmp += "b";
            vs1.insert(vs1.end(), tmp);
        }
        CHECK_THROWS_AS(k1.unpack(vs1), std::invalid_argument);
    }
};

TEST_CASE("kronecker_monomial_unpack_test")
{
    tuple_for_each(int_types{}, unpack_tester{});
}

struct print_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        k_type k1;
        std::ostringstream oss;
        k1.print(oss, symbol_fset{});
        CHECK(oss.str().empty());
        k_type k2(symbol_fset{"x"});
        k2.print(oss, symbol_fset{"x"});
        CHECK(oss.str().empty());
        k_type k3({T(-1)});
        k3.print(oss, symbol_fset{"x"});
        CHECK(oss.str() == "x**-1");
        k_type k4({T(1)});
        oss.str("");
        k4.print(oss, symbol_fset{"x"});
        CHECK(oss.str() == "x");
        k_type k5({T(-1), T(1)});
        oss.str("");
        k5.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "x**-1*y");
        k_type k6({T(-1), T(-2)});
        oss.str("");
        k6.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "x**-1*y**-2");
    }
};

TEST_CASE("kronecker_monomial_print_test")
{
    tuple_for_each(int_types{}, print_tester{});
}

struct is_linear_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        CHECK(!k_type().is_linear(symbol_fset{}).first);
        CHECK(!k_type().is_linear(symbol_fset{"x"}).first);
        k_type k({T(1)});
        CHECK(k.is_linear(symbol_fset{"x"}).first);
        CHECK(k.is_linear(symbol_fset{"x"}).second == 0u);
        k = k_type({T(0), T(1)});
        CHECK(k.is_linear(symbol_fset{"x", "y"}).first);
        CHECK(k.is_linear(symbol_fset{"x", "y"}).second == 1u);
        k = k_type({T(0), T(2)});
        CHECK(!k.is_linear(symbol_fset{"x", "y"}).first);
        k = k_type({T(2), T(0)});
        CHECK(!k.is_linear(symbol_fset{"x", "y"}).first);
        k = k_type({T(1), T(1)});
        CHECK(!k.is_linear(symbol_fset{"x", "y"}).first);
    }
};

TEST_CASE("kronecker_monomial_is_linear_test")
{
    tuple_for_each(int_types{}, is_linear_tester{});
}

template <typename K, typename E>
using k_pow_t = decltype(std::declval<const K &>().pow(std::declval<const E &>(), symbol_fset{}));

struct pow_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        typedef kronecker_array<T> ka;
        const auto &limits = ka::get_limits();
        k_type k1;
        k1.set_int(1);
        CHECK_THROWS_MATCHES(k1.pow(42, symbol_fset{}), std::invalid_argument, 
            test::ExceptionMatcher<std::invalid_argument>(std::string("a vector of size 0 must always be encoded as 0"))
        );
        CHECK_THROWS_MATCHES(k1.pow(42.5, symbol_fset{"x"}), safe_cast_failure, 
            test::ExceptionMatcher<std::invalid_argument>(std::string("the safe conversion of a value of type"))
        );
        k1 = k_type{2};
        k_type k2({4});
        CHECK(k1.pow(2, symbol_fset{"x"}) == k2);
        CHECK_THROWS_MATCHES(
            k1.pow(std::numeric_limits<T>::max(), symbol_fset{"x"}), std::overflow_error,
            test::ExceptionMatcher<std::overflow_error>(std::string("results in overflow")));
        k1 = k_type{1};
        if (std::get<0u>(limits[1u])[0u] < std::numeric_limits<T>::max()) {
            CHECK_THROWS_MATCHES(k1.pow(std::get<0u>(limits[1u])[0u] + T(1), symbol_fset{"x"}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("a component of the vector to be encoded is out of bounds"))
            );
        }
        CHECK((is_detected<k_pow_t, k_type, int>::value));
        CHECK((!is_detected<k_pow_t, k_type, std::string>::value));
    }
};

TEST_CASE("kronecker_monomial_pow_test")
{
    tuple_for_each(int_types{}, pow_tester{});
}

struct partial_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        CHECK(key_is_differentiable<k_type>::value);
        k_type k1;
        k1.set_int(T(1));
        // An empty symbol set must always be related to a zero encoded value.
        CHECK_THROWS_MATCHES(k1.partial(5, symbol_fset{}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string("a vector of size 0 must always be encoded as 0"))
        );
        k1 = k_type({T(2)});
        auto ret = k1.partial(0, symbol_fset{"x"});
        CHECK(ret.first == 2);
        CHECK(ret.second == k_type({T(1)}));
        // y is not in the monomial.
        ret = k1.partial(1, symbol_fset{"x"});
        CHECK(ret.first == 0);
        CHECK(ret.second == k_type(symbol_fset{"x"}));
        // x is in the monomial but it is zero.
        k1 = k_type({T(0)});
        ret = k1.partial(0, symbol_fset{"x"});
        CHECK(ret.first == 0);
        CHECK(ret.second == k_type(symbol_fset{"x"}));
        // y in the monomial but zero.
        k1 = k_type({T(-1), T(0)});
        ret = k1.partial(1, symbol_fset{"x", "y"});
        CHECK(ret.first == 0);
        CHECK(ret.second == k_type(symbol_fset{"x", "y"}));
        ret = k1.partial(0, symbol_fset{"x", "y"});
        CHECK(ret.first == -1);
        CHECK(ret.second == k_type({T(-2), T(0)}));
        // Check limits violation.
        typedef kronecker_array<T> ka;
        const auto &limits = ka::get_limits();
        k1 = k_type{-std::get<0u>(limits[2u])[0u], -std::get<0u>(limits[2u])[0u]};
        CHECK_THROWS_MATCHES(
            ret = k1.partial(0, symbol_fset{"x", "y"}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string("a component of the vector to be encoded is out of bounds"))
        );
    }
};

TEST_CASE("kronecker_monomial_partial_test")
{
    tuple_for_each(int_types{}, partial_tester{});
}

struct evaluate_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        CHECK((key_is_evaluable<k_type, integer>::value));
        k_type k1;
        CHECK(k1.template evaluate<integer>({}, symbol_fset{}) == integer(1));
        CHECK_THROWS_MATCHES(k1.template evaluate<integer>({}, symbol_fset{"x"}), std::invalid_argument,
                              test::ExceptionMatcher<std::invalid_argument>(std::string("invalid vector of values for Kronecker monomial "
                                                                   "evaluation: the size of the vector of values (0) "
                                                                   "differs from the size of the reference set of "
                                                                   "symbols (1)"))
        );
        k1 = k_type({T(1)});
        CHECK_THROWS_MATCHES(k1.template evaluate<integer>({}, symbol_fset{"x"}), std::invalid_argument,
                              test::ExceptionMatcher<std::invalid_argument>(std::string("invalid vector of values for Kronecker monomial "
                                                                   "evaluation: the size of the vector of values (0) "
                                                                   "differs from the size of the reference set of "
                                                                   "symbols (1)"))
        );
        CHECK(k1.template evaluate<integer>({1_z}, symbol_fset{"x"}) == 1);
        CHECK_THROWS_MATCHES(k1.template evaluate<integer>({1_z, 2_z}, symbol_fset{"x"}), std::invalid_argument,
                              test::ExceptionMatcher<std::invalid_argument>(std::string("invalid vector of values for Kronecker monomial "
                                                                   "evaluation: the size of the vector of values (2) "
                                                                   "differs from the size of the reference set of "
                                                                   "symbols (1)"))
        );
        k1 = k_type({T(2)});
        CHECK(k1.template evaluate<integer>({3_z}, symbol_fset{"x"}) == 9);
        k1 = k_type({T(2), T(3)});
        CHECK(k1.template evaluate<integer>({3_z, 4_z}, symbol_fset{"x", "y"}) == 576);
        CHECK(k1.template evaluate<double>({-4.3, 3.2}, symbol_fset{"x", "y"}) ==
                          piranha::pow(-4.3, 2) * piranha::pow(3.2, 3));
        CHECK(k1.template evaluate<rational>({-4_q / 3, 1_q / 2}, symbol_fset{"x", "y"}) ==
                          piranha::pow(rational(4, -3), 2) * piranha::pow(rational(-1, -2), 3));
        k1 = k_type({T(-2), T(-3)});
        CHECK(k1.template evaluate<rational>({-4_q / 3, 1_q / 2}, symbol_fset{"x", "y"}) ==
                          piranha::pow(rational(4, -3), -2) * piranha::pow(rational(-1, -2), -3));
#if defined(MPPP_WITH_MPFR)
        CHECK(k1.template evaluate<real>({real(1.234), real(5.678)}, symbol_fset{"x", "y"}) ==
                          piranha::pow(real(5.678), T(-3)) * piranha::pow(real(1.234), T(-2)));
#endif
    }
};

TEST_CASE("kronecker_monomial_evaluate_test")
{
    tuple_for_each(int_types{}, evaluate_tester{});
    CHECK((!key_is_evaluable<kronecker_monomial<>, std::vector<int>>::value));
    CHECK((!key_is_evaluable<kronecker_monomial<>, char *>::value));
    CHECK((!key_is_evaluable<kronecker_monomial<>, std::string>::value));
    CHECK((!key_is_evaluable<kronecker_monomial<>, void *>::value));
}

struct subs_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        // Test the type trait.
        CHECK((key_has_subs<k_type, integer>::value));
        CHECK((key_has_subs<k_type, rational>::value));
#if defined(MPPP_WITH_MPFR)
        CHECK((key_has_subs<k_type, real>::value));
#endif
        CHECK((key_has_subs<k_type, double>::value));
        CHECK((!key_has_subs<k_type, std::string>::value));
        CHECK((!key_has_subs<k_type, std::vector<std::string>>::value));
        k_type k1;
        auto ret = k1.template subs<integer>({}, symbol_fset{});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK((std::is_same<integer, decltype(ret[0u].first)>::value));
        CHECK(ret[0u].second == k1);
        k1 = k_type({T(1)});
        CHECK_THROWS_MATCHES(k1.template subs<integer>({{0, 4_z}}, symbol_fset{}), std::invalid_argument,
                              test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for substitution in a "
                                                "Kronecker monomial: the last index of the "
                                                "substitution map (0) must be smaller than the monomial's size (0)"))
        );
        CHECK_THROWS_MATCHES(
            k1.template subs<integer>({{0, 4_z}, {1, 4_z}, {2, 4_z}, {7, 4_z}}, symbol_fset{"x", "y"}),
            std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for substitution in a "
                                                 "Kronecker monomial: the last index of the "
                                                 "substitution map (7) must be smaller than the monomial's size (2)"))
        );
        k1 = k_type({T(2)});
        ret = k1.template subs<integer>({}, symbol_fset{"x"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK(ret[0u].second == k1);
        ret = k1.template subs<integer>({{0, 4_z}}, symbol_fset{"x"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == piranha::pow(integer(4), T(2)));
        CHECK(ret[0u].second == k_type{T(0)});
        k1 = k_type({T(2), T(3)});
        ret = k1.template subs<integer>({{1, -2_z}}, symbol_fset{"x", "y"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == piranha::pow(integer(-2), T(3)));
        CHECK((ret[0u].second == k_type{T(2), T(0)}));
#if defined(MPPP_WITH_MPFR)
        auto ret2 = k1.template subs<real>({{0, real(-2.345)}}, symbol_fset{"x", "y"});
        CHECK(ret2.size() == 1u);
        CHECK(ret2[0u].first == piranha::pow(real(-2.345), T(2)));
        CHECK((ret2[0u].second == k_type{T(0), T(3)}));
        CHECK((std::is_same<real, decltype(ret2[0u].first)>::value));
#endif
        auto ret3 = k1.template subs<rational>({{0, -1_q / 2}}, symbol_fset{"x", "y"});
        CHECK(ret3.size() == 1u);
        CHECK(ret3[0u].first == rational(1, 4));
        CHECK((ret3[0u].second == k_type{T(0), T(3)}));
        CHECK((std::is_same<rational, decltype(ret3[0u].first)>::value));
        ret3 = k1.template subs<rational>({{1, 3_q / 2}, {0, -1_q / 2}}, symbol_fset{"x", "y"});
        CHECK(ret3.size() == 1u);
        CHECK(ret3[0u].first == rational(27, 32));
        CHECK((ret3[0u].second == k_type{T(0), T(0)}));
        if (std::numeric_limits<T>::max() >= std::numeric_limits<int>::max()) {
            k1 = k_type({T(-2), T(2), T(3)});
            ret3 = k1.template subs<rational>({{2, 3_q / 2}, {0, -1_q / 2}}, symbol_fset{"x", "y", "z"});
            CHECK(ret3.size() == 1u);
            CHECK(ret3[0u].first == rational(27, 2));
            CHECK((ret3[0u].second == k_type{T(0), T(2), T(0)}));
            ret3 = k1.template subs<rational>({{2, 3_q / 2}, {0, -1_q / 2}, {1, 2_q / 3}}, symbol_fset{"x", "y", "z"});
            CHECK(ret3.size() == 1u);
            CHECK(ret3[0u].first == rational(6, 1));
            CHECK((ret3[0u].second == k_type{T(0), T(0), T(0)}));
            k1 = k_type({T(2), T(3), T(4)});
            ret3 = k1.template subs<rational>({{0, 1_q}, {2, -3_q}}, symbol_fset{"x", "y", "z"});
            CHECK(ret3.size() == 1u);
            CHECK(ret3[0u].first == 81);
            CHECK((ret3[0u].second == k_type{T(0), T(3), T(0)}));
        }
    }
};

TEST_CASE("kronecker_monomial_subs_test")
{
    tuple_for_each(int_types{}, subs_tester{});
}

struct print_tex_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        k_type k1;
        std::ostringstream oss;
        k1.print_tex(oss, symbol_fset{});
        CHECK(oss.str().empty());
        k1 = k_type({T(1)});
        CHECK_THROWS_MATCHES(k1.print_tex(oss, symbol_fset{}), std::invalid_argument,
                             test::ExceptionMatcher<std::invalid_argument>(std::string("a vector of size 0 must always be encoded as 0"))
        );
        k1 = k_type({T(0)});
        k1.print_tex(oss, symbol_fset{"x"});
        CHECK(oss.str() == "");
        k1 = k_type({T(1)});
        k1.print_tex(oss, symbol_fset{"x"});
        CHECK(oss.str() == "{x}");
        oss.str("");
        k1 = k_type({T(-1)});
        k1.print_tex(oss, symbol_fset{"x"});
        CHECK(oss.str() == "\\frac{1}{{x}}");
        oss.str("");
        k1 = k_type({T(2)});
        k1.print_tex(oss, symbol_fset{"x"});
        CHECK(oss.str() == "{x}^{2}");
        oss.str("");
        k1 = k_type({T(-2)});
        k1.print_tex(oss, symbol_fset{"x"});
        CHECK(oss.str() == "\\frac{1}{{x}^{2}}");
        oss.str("");
        k1 = k_type({T(-2), T(1)});
        k1.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\frac{{y}}{{x}^{2}}");
        oss.str("");
        k1 = k_type({T(-2), T(3)});
        k1.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\frac{{y}^{3}}{{x}^{2}}");
        oss.str("");
        k1 = k_type({T(-2), T(-3)});
        k1.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\frac{1}{{x}^{2}{y}^{3}}");
        oss.str("");
        k1 = k_type({T(2), T(3)});
        k1.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "{x}^{2}{y}^{3}");
        oss.str("");
        k1 = k_type({T(1), T(3)});
        k1.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "{x}{y}^{3}");
        oss.str("");
        k1 = k_type({T(0), T(3)});
        k1.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "{y}^{3}");
        oss.str("");
        k1 = k_type({T(0), T(0)});
        k1.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "");
        oss.str("");
        k1 = k_type({T(0), T(1)});
        k1.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "{y}");
        oss.str("");
        k1 = k_type({T(0), T(-1)});
        k1.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\frac{1}{{y}}");
    }
};

TEST_CASE("kronecker_monomial_print_tex_test")
{
    tuple_for_each(int_types{}, print_tex_tester{});
}

struct integrate_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        CHECK(key_is_integrable<k_type>::value);
        k_type k1;
        auto ret = k1.integrate("a", symbol_fset{});
        CHECK(ret.first == T(1));
        CHECK(ret.second == k_type({T(1)}));
        k1 = k_type{T(1)};
        CHECK_THROWS_AS(k1.integrate("b", symbol_fset{}), std::invalid_argument);
        ret = k1.integrate("b", symbol_fset{"b"});
        CHECK(ret.first == T(2));
        CHECK(ret.second == k_type({T(2)}));
        k1 = k_type{T(2)};
        ret = k1.integrate("c", symbol_fset{"b"});
        CHECK(ret.first == T(1));
        CHECK(ret.second == k_type({T(2), T(1)}));
        ret = k1.integrate("a", symbol_fset{"b"});
        CHECK(ret.first == T(1));
        CHECK(ret.second == k_type({T(1), T(2)}));
        k1 = k_type{T(0), T(1)};
        ret = k1.integrate("a", symbol_fset{"b", "d"});
        CHECK(ret.first == T(1));
        CHECK(ret.second == k_type({T(1), T(0), T(1)}));
        ret = k1.integrate("b", symbol_fset{"b", "d"});
        CHECK(ret.first == T(1));
        CHECK(ret.second == k_type({T(1), T(1)}));
        ret = k1.integrate("c", symbol_fset{"b", "d"});
        CHECK(ret.first == T(1));
        CHECK(ret.second == k_type({T(0), T(1), T(1)}));
        ret = k1.integrate("d", symbol_fset{"b", "d"});
        CHECK(ret.first == T(2));
        CHECK(ret.second == k_type({T(0), T(2)}));
        ret = k1.integrate("e", symbol_fset{"b", "d"});
        CHECK(ret.first == T(1));
        CHECK(ret.second == k_type({T(0), T(1), T(1)}));
        k1 = k_type{T(-1), T(0)};
        CHECK_THROWS_AS(k1.integrate("b", symbol_fset{"b", "d"}), std::invalid_argument);
        k1 = k_type{T(0), T(-1)};
        CHECK_THROWS_AS(k1.integrate("d", symbol_fset{"b", "d"}), std::invalid_argument);
        // Check limits violation.
        typedef kronecker_array<T> ka;
        const auto &limits = ka::get_limits();
        k1 = k_type{std::get<0u>(limits[2u])[0u], std::get<0u>(limits[2u])[0u]};
        CHECK_THROWS_AS(ret = k1.integrate("b", symbol_fset{"b", "d"}), std::invalid_argument);
    }
};

TEST_CASE("kronecker_monomial_integrate_test")
{
    tuple_for_each(int_types{}, integrate_tester{});
}

struct trim_identify_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        k_type k0;
        std::vector<char> mask;
        CHECK_NOTHROW(k0.trim_identify(mask, symbol_fset{}));
        CHECK(mask.size() == 0u);
        k0.set_int(1);
        CHECK_THROWS_MATCHES(
            k0.trim_identify(mask, symbol_fset{"x"}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string(
                "invalid mask for trim_identify(): the size of the mask (0) differs "
                "from the size of the reference symbol set (1)"))
        );
        mask = {1};
        CHECK_THROWS_MATCHES(
            k0.trim_identify(mask, symbol_fset{}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string(
                "invalid mask for trim_identify(): the size of the mask (1) differs "
                "from the size of the reference symbol set (0)"))
        );
        k0.trim_identify(mask, symbol_fset{"x"});
        CHECK(!mask[0]);
        mask = {1};
        k0 = k_type{0};
        k0.trim_identify(mask, symbol_fset{"x"});
        CHECK(mask[0]);
        k0 = k_type({T(1), T(2)});
        mask = {1, 1};
        k0.trim_identify(mask, symbol_fset{"x", "y"});
        CHECK((mask == std::vector<char>{0, 0}));
        k0 = k_type({T(0), T(2)});
        mask = {1, 1};
        k0.trim_identify(mask, symbol_fset{"x", "y"});
        CHECK((mask == std::vector<char>{1, 0}));
        k0 = k_type({T(0), T(0)});
        mask = {1, 1};
        k0.trim_identify(mask, symbol_fset{"x", "y"});
        CHECK((mask == std::vector<char>{1, 1}));
        k0 = k_type({T(1), T(0)});
        mask = {1, 1};
        k0.trim_identify(mask, symbol_fset{"x", "y"});
        CHECK((mask == std::vector<char>{0, 1}));
    }
};

TEST_CASE("kronecker_monomial_trim_identify_test")
{
    tuple_for_each(int_types{}, trim_identify_tester{});
}

struct trim_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        k_type k0;
        CHECK(k0.trim({}, symbol_fset{}) == k0);
        k0.set_int(1);
        CHECK_THROWS_MATCHES(k0.trim({}, symbol_fset{"x"}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string(
            "invalid mask for trim(): the size of the mask (0) differs from the size "
            "of the reference symbol set (1)"))
        );
        CHECK_THROWS_MATCHES(k0.trim({1}, symbol_fset{}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string(
            "invalid mask for trim(): the size of the mask (1) differs from the size "
            "of the reference symbol set (0)"))
        );
        k0 = k_type{T(1), T(0), T(-1)};
        CHECK((k0.trim({0, 1, 0}, symbol_fset{"x", "y", "z"}) == k_type{1, -1}));
        CHECK((k0.trim({1, 0, 0}, symbol_fset{"x", "y", "z"}) == k_type{0, -1}));
        CHECK((k0.trim({0, 0, 0}, symbol_fset{"x", "y", "z"}) == k0));
        CHECK((k0.trim({1, 0, 1}, symbol_fset{"x", "y", "z"}) == k_type{0}));
        CHECK((k0.trim({1, 1, 0}, symbol_fset{"x", "y", "z"}) == k_type{-1}));
        CHECK((k0.trim({0, 1, 1}, symbol_fset{"x", "y", "z"}) == k_type{1}));
        CHECK((k0.trim({1, 1, 1}, symbol_fset{"x", "y", "z"}) == k_type{}));
    }
};

TEST_CASE("kronecker_monomial_trim_test")
{
    tuple_for_each(int_types{}, trim_tester{});
}

struct ipow_subs_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = kronecker_monomial<T>;
        // Test the type trait.
        CHECK((key_has_ipow_subs<k_type, integer>::value));
        CHECK((key_has_ipow_subs<k_type, double>::value));
#if defined(MPPP_WITH_MPFR)
        CHECK((key_has_ipow_subs<k_type, real>::value));
#endif
        CHECK((key_has_ipow_subs<k_type, rational>::value));
        CHECK((!key_has_ipow_subs<k_type, std::string>::value));
        k_type k1;
        auto ret = k1.ipow_subs(1, integer(45), integer(4), symbol_fset{});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK((std::is_same<integer, decltype(ret[0u].first)>::value));
        CHECK(ret[0u].second == k1);
        ret = k1.ipow_subs(0, integer(45), integer(4), symbol_fset{});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK((std::is_same<integer, decltype(ret[0u].first)>::value));
        CHECK(ret[0u].second == k1);
        CHECK_THROWS_MATCHES(k1.ipow_subs(0, integer(0), integer(4), symbol_fset{}), std::invalid_argument,
                              test::ExceptionMatcher<std::invalid_argument>(std::string(
                                  "invalid integral power for ipow_subs() in a "
                                  "Kronecker monomial: the power must be nonzero"))
        );
        k1 = k_type({T(2)});
        ret = k1.ipow_subs(1, integer(2), integer(4), symbol_fset{"x"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK(ret[0u].second == k1);
        ret = k1.ipow_subs(0, integer(2), integer(4), symbol_fset{"x"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 4);
        CHECK(ret[0u].second == k_type{T(0)});
        ret = k1.ipow_subs(0, integer(1), integer(4), symbol_fset{"x"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 16);
        CHECK(ret[0u].second == k_type{T(0)});
        ret = k1.ipow_subs(0, integer(3), integer(4), symbol_fset{"x"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK(ret[0u].second == k1);
        ret = k1.ipow_subs(0, integer(-1), integer(4), symbol_fset{"x"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK(ret[0u].second == k_type({T(2)}));
        ret = k1.ipow_subs(0, integer(4), integer(4), symbol_fset{"x"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK(ret[0u].second == k_type({T(2)}));
        if (std::is_same<T, signed char>::value) {
            // Values below are too large for signed char.
            return;
        }
        k1 = k_type({T(7), T(2)});
        ret = k1.ipow_subs(0, integer(3), integer(2), symbol_fset{"x", "y"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == piranha::pow(integer(2), T(2)));
        CHECK((ret[0u].second == k_type{T(1), T(2)}));
        ret = k1.ipow_subs(0, integer(4), integer(2), symbol_fset{"x", "y"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == piranha::pow(integer(2), T(1)));
        CHECK((ret[0u].second == k_type{T(3), T(2)}));
        ret = k1.ipow_subs(0, integer(-4), integer(2), symbol_fset{"x", "y"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK((ret[0u].second == k_type{T(7), T(2)}));
        k1 = k_type({T(-7), T(2)});
        ret = k1.ipow_subs(0, integer(4), integer(2), symbol_fset{"x", "y"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == 1);
        CHECK((ret[0u].second == k_type{T(-7), T(2)}));
        ret = k1.ipow_subs(0, integer(-4), integer(2), symbol_fset{"x", "y"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == piranha::pow(integer(2), T(1)));
        CHECK((ret[0u].second == k_type{T(-3), T(2)}));
        ret = k1.ipow_subs(0, integer(-3), integer(2), symbol_fset{"x", "y"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == piranha::pow(integer(2), T(2)));
        CHECK((ret[0u].second == k_type{T(-1), T(2)}));
        k1 = k_type({T(2), T(-7)});
        ret = k1.ipow_subs(1, integer(-3), integer(2), symbol_fset{"x", "y"});
        CHECK(ret.size() == 1u);
        CHECK(ret[0u].first == piranha::pow(integer(2), T(2)));
        CHECK((ret[0u].second == k_type{T(2), T(-1)}));
        k1 = k_type({T(-7), T(2)});
#if defined(MPPP_WITH_MPFR)
        auto ret2 = k1.ipow_subs(0, integer(-4), real(-2.345), symbol_fset{"x", "y"});
        CHECK(ret2.size() == 1u);
        CHECK(ret2[0u].first == piranha::pow(real(-2.345), T(1)));
        CHECK((ret2[0u].second == k_type{T(-3), T(2)}));
        CHECK((std::is_same<real, decltype(ret2[0u].first)>::value));
#endif
        auto ret3 = k1.ipow_subs(0, integer(-3), rational(-1, 2), symbol_fset{"x", "y"});
        CHECK(ret3.size() == 1u);
        CHECK(ret3[0u].first == piranha::pow(rational(-1, 2), T(2)));
        CHECK((ret3[0u].second == k_type{T(-1), T(2)}));
        CHECK((std::is_same<rational, decltype(ret3[0u].first)>::value));
    }
};

TEST_CASE("kronecker_monomial_ipow_subs_test")
{
    tuple_for_each(int_types{}, ipow_subs_tester{});
}

struct tt_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef kronecker_monomial<T> k_type;
        CHECK((!key_has_t_subs<k_type, int, int>::value));
        CHECK((!key_has_t_subs<k_type &, int, int>::value));
        CHECK((!key_has_t_subs<k_type const &, int, int>::value));
        CHECK(is_hashable<k_type>::value);
        CHECK(is_key_degree_type<k_type>::value);
        CHECK(is_key_ldegree_type<k_type>::value);
        CHECK(!key_has_t_degree<k_type>::value);
        CHECK(!key_has_t_ldegree<k_type>::value);
        CHECK(!key_has_t_order<k_type>::value);
        CHECK(!key_has_t_lorder<k_type>::value);
    }
};

TEST_CASE("kronecker_monomial_type_traits_test")
{
    tuple_for_each(int_types{}, tt_tester{});
}

TEST_CASE("kronecker_monomial_kic_test")
{
    CHECK((key_is_convertible<k_monomial, k_monomial>::value));
    CHECK((!key_is_convertible<kronecker_monomial<int>, kronecker_monomial<long>>::value));
}

TEST_CASE("kronecker_monomial_comparison_test")
{
    CHECK((is_less_than_comparable<k_monomial>::value));
    CHECK(!(k_monomial{} < k_monomial{}));
    CHECK(!(k_monomial{1} < k_monomial{1}));
    CHECK(!(k_monomial{2} < k_monomial{1}));
    CHECK(k_monomial{1} < k_monomial{2});
}
