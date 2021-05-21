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

#include <piranha/monomial.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <mp++/config.hpp>

#include <piranha/exceptions.hpp>
#include <piranha/integer.hpp>
#include <piranha/key/key_degree.hpp>
#include <piranha/key/key_is_one.hpp>
#include <piranha/key/key_is_zero.hpp>
#include <piranha/key/key_ldegree.hpp>
#include <piranha/key_is_convertible.hpp>
#include <piranha/key_is_multipliable.hpp>
#include <piranha/kronecker_monomial.hpp>
#include <piranha/math.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/symbol_utils.hpp>
#include <piranha/term.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

using expo_types = std::tuple<signed char, int, integer, rational>;
using size_types = std::tuple<std::integral_constant<std::size_t, 0u>, std::integral_constant<std::size_t, 1u>,
                              std::integral_constant<std::size_t, 5u>, std::integral_constant<std::size_t, 10u>>;

// Constructors, assignments and element access.
struct constructor_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> monomial_type;
            CHECK(is_key<monomial_type>::value);
            monomial_type m0;
            monomial_type tmp;
            CHECK_NOTHROW(tmp = monomial_type());
            CHECK_NOTHROW(tmp = monomial_type(monomial_type()));
            CHECK_NOTHROW(tmp = monomial_type(m0));
            // From init list.
            monomial_type m1{T(0), T(1), T(2), T(3)};
            CHECK(m1.size() == static_cast<decltype(m1.size())>(4));
            for (typename monomial_type::size_type i = 0; i < m1.size(); ++i) {
                CHECK(m1[i] == T(i));
                CHECK_NOTHROW(m1[i] = static_cast<T>(T(i) + T(1)));
                CHECK(m1[i] == T(i) + T(1));
            }
            monomial_type m1a{0, 1, 2, 3};
            CHECK(m1a.size() == static_cast<decltype(m1a.size())>(4));
            for (typename monomial_type::size_type i = 0; i < m1a.size(); ++i) {
                CHECK(m1a[i] == T(i));
                CHECK_NOTHROW(m1a[i] = static_cast<T>(T(i) + T(1)));
                CHECK(m1a[i] == T(i) + T(1));
            }
            CHECK_NOTHROW(m0 = m1);
            CHECK_NOTHROW(m0 = std::move(m1));
            // From range and symbol set.
            std::vector<int> v1;
            m0 = monomial_type(v1.begin(), v1.end(), symbol_fset{});
            CHECK(m0.size() == 0u);
            v1 = {-1};
            m0 = monomial_type(v1.begin(), v1.end(), symbol_fset{"x"});
            CHECK(m0.size() == 1u);
            CHECK(m0[0] == -1);
            v1 = {-1, 2};
            m0 = monomial_type(v1.begin(), v1.end(), symbol_fset{"x", "y"});
            CHECK(m0.size() == 2u);
            CHECK(m0[0] == -1);
            CHECK(m0[1] == 2);
            CHECK_THROWS_MATCHES(
                m0 = monomial_type(v1.begin(), v1.end(), symbol_fset{"x"}), std::invalid_argument,
                test::ExceptionMatcher<std::invalid_argument>(std::string("the monomial constructor from range and symbol set "
                        "yielded an invalid monomial: the final size is 2, while the size of the symbol set is 1"))
            );
            std::list<int> l1;
            m0 = monomial_type(l1.begin(), l1.end(), symbol_fset{});
            CHECK(m0.size() == 0u);
            l1 = {-1};
            m0 = monomial_type(l1.begin(), l1.end(), symbol_fset{"x"});
            CHECK(m0.size() == 1u);
            CHECK(m0[0] == -1);
            l1 = {-1, 2};
            m0 = monomial_type(l1.begin(), l1.end(), symbol_fset{"x", "y"});
            CHECK(m0.size() == 2u);
            CHECK(m0[0] == -1);
            CHECK(m0[1] == 2);
            CHECK_THROWS_MATCHES(
                m0 = monomial_type(l1.begin(), l1.end(), symbol_fset{"x"}), std::invalid_argument,
                test::ExceptionMatcher<std::invalid_argument>(std::string("the monomial constructor from range and symbol set "
                        "yielded an invalid monomial: the final size is 2, while the size of the symbol set is 1"))
            );
            struct foo {
            };
            CHECK((!std::is_constructible<monomial_type, foo *, foo *, symbol_fset>::value));
            CHECK((std::is_constructible<monomial_type, int *, int *, symbol_fset>::value));
            // From range and symbol set.
            v1.clear();
            m0 = monomial_type(v1.begin(), v1.end());
            CHECK(m0.size() == 0u);
            v1 = {-1};
            m0 = monomial_type(v1.begin(), v1.end());
            CHECK(m0.size() == 1u);
            CHECK(m0[0] == -1);
            v1 = {-1, 2};
            m0 = monomial_type(v1.begin(), v1.end());
            CHECK(m0.size() == 2u);
            CHECK(m0[0] == -1);
            CHECK(m0[1] == 2);
            l1.clear();
            m0 = monomial_type(l1.begin(), l1.end());
            CHECK(m0.size() == 0u);
            l1 = {-1};
            m0 = monomial_type(l1.begin(), l1.end());
            CHECK(m0.size() == 1u);
            CHECK(m0[0] == -1);
            l1 = {-1, 2};
            m0 = monomial_type(l1.begin(), l1.end());
            CHECK(m0.size() == 2u);
            CHECK(m0[0] == -1);
            CHECK(m0[1] == 2);
            CHECK((!std::is_constructible<monomial_type, foo *, foo *>::value));
            CHECK((std::is_constructible<monomial_type, int *, int *>::value));
            // Constructor from arguments vector.
            monomial_type m2 = monomial_type(symbol_fset{});
            CHECK(m2.size() == unsigned(0));
            monomial_type m3 = monomial_type(symbol_fset({"a", "b", "c"}));
            CHECK(m3.size() == unsigned(3));
            symbol_fset vs({"a", "b", "c"});
            monomial_type k2(vs);
            CHECK(k2.size() == vs.size());
            CHECK(k2[0] == T(0));
            CHECK(k2[1] == T(0));
            CHECK(k2[2] == T(0));
            // Generic constructor for use in series.
            CHECK_THROWS_MATCHES(tmp = monomial_type(k2, symbol_fset{}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("inconsistent sizes in the generic array_key "
                                                                       "constructor: the size of the array (3) differs "
                                                                       "from the size of the symbol set (0)"))
            );
            monomial_type k3(k2, vs);
            CHECK(k3.size() == vs.size());
            CHECK(k3[0] == T(0));
            CHECK(k3[1] == T(0));
            CHECK(k3[2] == T(0));
            monomial_type k4(monomial_type(vs), vs);
            CHECK(k4.size() == vs.size());
            CHECK(k4[0] == T(0));
            CHECK(k4[1] == T(0));
            CHECK(k4[2] == T(0));
            typedef monomial<int, U> monomial_type2;
            monomial_type2 k5(vs);
            CHECK_THROWS_MATCHES(tmp  = monomial_type(k5, symbol_fset{}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("inconsistent sizes in the generic array_key "
                                                                       "constructor: the size of the array (3) differs "
                                                                       "from the size of the symbol set (0)"))
            );
            monomial_type k6(k5, vs);
            CHECK(k6.size() == vs.size());
            CHECK(k6[0] == T(0));
            CHECK(k6[1] == T(0));
            CHECK(k6[2] == T(0));
            monomial_type k7(monomial_type2(vs), vs);
            CHECK(k7.size() == vs.size());
            CHECK(k7[0] == T(0));
            CHECK(k7[1] == T(0));
            CHECK(k7[2] == T(0));
            // Type trait check.
            CHECK((std::is_constructible<monomial_type, monomial_type>::value));
            CHECK((std::is_constructible<monomial_type, symbol_fset>::value));
            CHECK((!std::is_constructible<monomial_type, std::string>::value));
            CHECK((!std::is_constructible<monomial_type, monomial_type, int>::value));
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_constructor_test")
{
    tuple_for_each(expo_types{}, constructor_tester{});
}

struct hash_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> monomial_type;
            monomial_type m0;
            CHECK(m0.hash() == std::size_t());
            CHECK(m0.hash() == std::hash<monomial_type>()(m0));
            monomial_type m1{T(0), T(1), T(2), T(3)};
            CHECK(m1.hash() == std::hash<monomial_type>()(m1));
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_hash_test")
{
    tuple_for_each(expo_types{}, hash_tester{});
}

struct compatibility_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> monomial_type;
            monomial_type m0;
            CHECK(m0.is_compatible(symbol_fset{}));
            symbol_fset ss({"foobarize"});
            monomial_type m1{T(0), T(1)};
            CHECK(!m1.is_compatible(ss));
            monomial_type m2{T(0)};
            CHECK(m2.is_compatible(ss));
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_compatibility_test")
{
    tuple_for_each(expo_types{}, compatibility_tester{});
}

struct is_zero_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> monomial_type;
            monomial_type m0;
            CHECK(!piranha::key_is_zero(m0, symbol_fset{}));
            monomial_type m1{T(0)};
            CHECK(!piranha::key_is_zero(m1, symbol_fset({"foobarize"})));
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_is_zero_test")
{
    tuple_for_each(expo_types{}, is_zero_tester{});
}

struct key_is_one_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> key_type;
            key_type k(symbol_fset{});
            CHECK(piranha::key_is_one(k, symbol_fset{}));
            key_type k2(symbol_fset{"a"});
            CHECK(piranha::key_is_one(k2, symbol_fset{"a"}));
            k2[0] = 1;
            CHECK(!piranha::key_is_one(k2, symbol_fset{"a"}));
            k2[0] = 0;
            CHECK(piranha::key_is_one(k2, symbol_fset{"a"}));
            CHECK_THROWS_MATCHES(
                piranha::key_is_one(k2, symbol_fset{}), std::invalid_argument, 
                    test::ExceptionMatcher<std::invalid_argument>(std::string("invalid sizes in the invocation of key_is_one() for a monomial: "
                                                     "the monomial has a size of 1, while the reference symbol set has "
                                                     "a size of 0"))
            );
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_key_is_one_test")
{
    tuple_for_each(expo_types{}, key_is_one_tester{});
}

struct degree_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> key_type;
            key_type k0;
            // Check the promotion of short signed ints.
            if (std::is_same<T, signed char>::value || std::is_same<T, short>::value) {
                CHECK((std::is_same<int, decltype(key_degree(k0, symbol_fset{}))>::value));
            } else if (std::is_integral<T>::value) {
                CHECK((std::is_same<T, decltype(key_degree(k0, symbol_fset{}))>::value));
            }
            CHECK(is_key_degree_type<key_type>::value);
            CHECK(is_key_ldegree_type<key_type>::value);
            CHECK(key_degree(k0, symbol_fset{}) == T(0));
            CHECK(key_ldegree(k0, symbol_fset{}) == T(0));
            key_type k1(symbol_fset{"a"});
            CHECK(key_degree(k1, symbol_fset{"a"}) == T(0));
            CHECK(key_ldegree(k1, symbol_fset{"a"}) == T(0));
            k1[0] = T(2);
            CHECK(key_degree(k1, symbol_fset{"a"}) == T(2));
            CHECK(key_ldegree(k1, symbol_fset{"a"}) == T(2));
            key_type k2(symbol_fset{"a", "b"});
            CHECK(key_degree(k2, symbol_fset{"a", "b"}) == T(0));
            CHECK(key_ldegree(k2, symbol_fset{"a", "b"}) == T(0));
            k2[0] = T(2);
            k2[1] = T(3);
            CHECK(key_degree(k2, symbol_fset{"a", "b"}) == T(2) + T(3));
            CHECK_THROWS_AS(key_degree(k2, symbol_fset{}), std::invalid_argument);
            // Partial degree.
            if (std::is_same<T, signed char>::value || std::is_same<T, short>::value) {
                CHECK((std::is_same<int, decltype(key_degree(k2, symbol_idx_fset{}, symbol_fset{}))>::value));
            } else if (std::is_integral<T>::value) {
                CHECK((std::is_same<T, decltype(key_degree(k2, symbol_idx_fset{}, symbol_fset{}))>::value));
            }
            CHECK(key_degree(k2, symbol_idx_fset{}, symbol_fset{"a", "b"}) == T(0));
            CHECK(key_degree(k2, symbol_idx_fset{0}, symbol_fset{"a", "b"}) == T(2));
            CHECK(key_degree(k2, symbol_idx_fset{1}, symbol_fset{"a", "b"}) == T(3));
            CHECK(key_degree(k2, symbol_idx_fset{0, 1}, symbol_fset{"a", "b"}) == T(3) + T(2));
            CHECK(key_ldegree(k2, symbol_idx_fset{}, symbol_fset{"a", "b"}) == T(0));
            CHECK(key_ldegree(k2, symbol_idx_fset{0}, symbol_fset{"a", "b"}) == T(2));
            CHECK(key_ldegree(k2, symbol_idx_fset{1}, symbol_fset{"a", "b"}) == T(3));
            CHECK(key_ldegree(k2, symbol_idx_fset{0, 1}, symbol_fset{"a", "b"}) == T(3) + T(2));
            key_type k3(symbol_fset{"a", "b", "c"});
            k3[0] = T(2);
            k3[1] = T(3);
            k3[2] = T(4);
            CHECK(key_degree(k3, symbol_idx_fset{}, symbol_fset{"a", "b", "c"}) == T(0));
            CHECK(key_degree(k3, symbol_idx_fset{0}, symbol_fset{"a", "b", "c"}) == T(2));
            CHECK(key_degree(k3, symbol_idx_fset{1}, symbol_fset{"a", "b", "c"}) == T(3));
            CHECK(key_degree(k3, symbol_idx_fset{0, 1}, symbol_fset{"a", "b", "c"}) == T(3) + T(2));
            CHECK(key_degree(k3, symbol_idx_fset{0, 2}, symbol_fset{"a", "b", "c"}) == T(4) + T(2));
            CHECK(key_degree(k3, symbol_idx_fset{1, 2}, symbol_fset{"a", "b", "c"}) == T(4) + T(3));
            CHECK(key_degree(k3, symbol_idx_fset{1, 2, 0}, symbol_fset{"a", "b", "c"}) == T(4) + T(3) + T(2));
            CHECK(key_ldegree(k3, symbol_idx_fset{}, symbol_fset{"a", "b", "c"}) == T(0));
            CHECK(key_ldegree(k3, symbol_idx_fset{0}, symbol_fset{"a", "b", "c"}) == T(2));
            CHECK(key_ldegree(k3, symbol_idx_fset{1}, symbol_fset{"a", "b", "c"}) == T(3));
            CHECK(key_ldegree(k3, symbol_idx_fset{0, 1}, symbol_fset{"a", "b", "c"}) == T(3) + T(2));
            CHECK(key_ldegree(k3, symbol_idx_fset{0, 2}, symbol_fset{"a", "b", "c"}) == T(4) + T(2));
            CHECK(key_ldegree(k3, symbol_idx_fset{1, 2}, symbol_fset{"a", "b", "c"}) == T(4) + T(3));
            CHECK(key_ldegree(k3, symbol_idx_fset{1, 2, 0}, symbol_fset{"a", "b", "c"}) == T(4) + T(3) + T(2));
            // Error checking.
            CHECK_THROWS_MATCHES(key_degree(k3, symbol_idx_fset{}, symbol_fset{"a", "b"}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("invalid symbol set for the computation of the "
                                                             "partial degree of a monomial: the size of the symbol "
                                                             "set (2) differs from the size of the monomial (3)"))
            );
            CHECK_THROWS_MATCHES(key_degree(k3, symbol_idx_fset{1, 2, 3}, symbol_fset{"a", "b", "c"}), std::invalid_argument,
                                 test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                          "partial degree of a monomial is 3, but the monomial has a size of only 3"))
            );
            CHECK_THROWS_MATCHES(key_ldegree(k3, symbol_idx_fset{}, symbol_fset{"a", "b"}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string(
                                                             "invalid symbol set for the computation of the "
                                                             "partial degree of a monomial: the size of the symbol "
                                                             "set (2) differs from the size of the monomial (3)"))
            );
            CHECK_THROWS_MATCHES(key_ldegree(k3, symbol_idx_fset{1, 2, 3}, symbol_fset{"a", "b", "c"}), std::invalid_argument,
                                 test::ExceptionMatcher<std::invalid_argument>(std::string(
                                          "the largest value in the positions set for the computation of the "
                                          "partial degree of a monomial is 3, but the monomial has a size of only 3"))
            );
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_degree_test")
{
    tuple_for_each(expo_types{}, degree_tester{});
    // Test the overflowing.
    using k_type = monomial<int>;
    k_type m{std::numeric_limits<int>::max(), 1};
    CHECK_THROWS_AS(key_ldegree(m, symbol_fset{"a", "b"}), std::overflow_error);
    m = k_type{std::numeric_limits<int>::min(), -1};
    CHECK_THROWS_AS(key_ldegree(m, symbol_fset{"a", "b"}), std::overflow_error);
    m = k_type{std::numeric_limits<int>::min(), 1};
    CHECK(key_ldegree(m, symbol_fset{"a", "b"}) == std::numeric_limits<int>::min() + 1);
    // Also for partial degree.
    m = k_type{std::numeric_limits<int>::max(), 1, 0};
    CHECK(key_ldegree(m, {0}, symbol_fset{"a", "b", "c"}) == std::numeric_limits<int>::max());
    CHECK_THROWS_AS(key_ldegree(m, {0, 1}, symbol_fset{"a", "b", "c"}), std::overflow_error);
    m = k_type{std::numeric_limits<int>::min(), 0, -1};
    CHECK(key_ldegree(m, {0}, symbol_fset{"a", "b", "c"}) == std::numeric_limits<int>::min());
    CHECK_THROWS_AS(key_ldegree(m, {0, 2}, symbol_fset{"a", "b", "c"}), std::overflow_error);
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
}
}

struct multiply_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            {
                // Integer coefficient.
                using key_type = monomial<T, U>;
                using term_type = term<integer, key_type>;
                term_type t1, t2;
                t1.m_cf = 2;
                t1.m_key = key_type{T(2)};
                t2.m_cf = 3;
                t2.m_key = key_type{T(3)};
                std::array<term_type, 1u> res;
                key_type::multiply(res, t1, t2, symbol_fset{"x"});
                CHECK(res[0].m_cf == t1.m_cf * t2.m_cf);
                CHECK(res[0].m_key[0] == T(5));
            }
            {
                // Try with rational as well, check special handling.
                using key_type = monomial<T, U>;
                using term_type = term<rational, key_type>;
                term_type t1, t2;
                t1.m_cf = 2 / 3_q;
                t1.m_key = key_type{T(2), T(-1)};
                t2.m_cf = -3;
                t2.m_key = key_type{T(3), T(7)};
                std::array<term_type, 1u> res;
                key_type::multiply(res, t1, t2, symbol_fset{"x", "y"});
                CHECK(res[0].m_cf == -6);
                CHECK(res[0].m_key[0] == T(5));
                CHECK(res[0].m_key[1] == T(6));
            }
            // Check throwing as well.
            using key_type = monomial<T, U>;
            using term_type = term<rational, key_type>;
            term_type t1, t2;
            t1.m_cf = 2 / 3_q;
            t1.m_key = key_type{T(2), T(-1)};
            t2.m_cf = -3;
            t2.m_key = key_type{T(3), T(7)};
            std::array<term_type, 1u> res;
            CHECK_THROWS_MATCHES(key_type::multiply(res, t1, t2, symbol_fset{"x"}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("cannot multiply terms with monomial keys: the "
                                                                       "size of the symbol set (1) differs from the "
                                                                       "size of the first monomial (2)"))
            );
            // Check the type trait.
            CHECK((key_is_multipliable<rational, key_type>::value));
            CHECK((key_is_multipliable<integer, key_type>::value));
            CHECK((key_is_multipliable<double, key_type>::value));
            CHECK((!key_is_multipliable<mock_cf3, key_type>::value));
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_multiply_test")
{
    tuple_for_each(expo_types{}, multiply_tester{});
}

struct print_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> k_type;
            k_type k1;
            std::ostringstream oss;
            k1.print(oss, symbol_fset{});
            CHECK(oss.str().empty());
            k_type k2(symbol_fset{"x"});
            k2.print(oss, symbol_fset{"x"});
            CHECK(oss.str() == "");
            oss.str("");
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
            k_type k7;
            CHECK_THROWS_MATCHES(
                k7.print(oss, symbol_fset{"x", "y"}), std::invalid_argument,
                        test::ExceptionMatcher<std::invalid_argument>(std::string("cannot print monomial: the size of the symbol set (2) differs "
                                                     "from the size of the monomial (0)"))
            );
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_print_test")
{
    tuple_for_each(expo_types{}, print_tester{});
    // Tests with rational coefficients.
    using m_type = monomial<rational>;
    m_type m1{rational{2}};
    std::ostringstream oss;
    m1.print(oss, symbol_fset{"x"});
    CHECK(oss.str() == "x**2");
    oss.str("");
    m1 = m_type{rational{-2, 3}};
    m1.print(oss, symbol_fset{"x"});
    CHECK(oss.str() == "x**(-2/3)");
}

struct is_linear_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> k_type;
            CHECK(!k_type{}.is_linear(symbol_fset{}).first);
            CHECK_THROWS_MATCHES(
                k_type{}.is_linear(symbol_fset{"x"}), std::invalid_argument,
                test::ExceptionMatcher<std::invalid_argument>(std::string("invalid symbol set for the identification of a linear "
                                  "monomial: the size of the symbol set (1) differs from the size of the monomial (0)"))
            );
            k_type k({T(0)});
            CHECK(!k.is_linear(symbol_fset{"x"}).first);
            k = k_type({T(2)});
            CHECK(!k.is_linear(symbol_fset{"x"}).first);
            k = k_type({T(1)});
            CHECK(k.is_linear(symbol_fset{"x"}).first);
            CHECK(k.is_linear(symbol_fset{"x"}).second == 0u);
            k = k_type({T(0), T(1)});
            CHECK(k.is_linear(symbol_fset{"x", "y"}).first);
            CHECK(k.is_linear(symbol_fset{"x", "y"}).second == 1u);
            k = k_type({T(1), T(0)});
            CHECK(k.is_linear(symbol_fset{"x", "y"}).first);
            CHECK(k.is_linear(symbol_fset{"x", "y"}).second == 0u);
            k = k_type({T(0), T(2)});
            CHECK(!k.is_linear(symbol_fset{"x", "y"}).first);
            k = k_type({T(1), T(1)});
            CHECK(!k.is_linear(symbol_fset{"x", "y"}).first);
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_is_linear_test")
{
    tuple_for_each(expo_types{}, is_linear_tester{});
    typedef monomial<rational> k_type;
    k_type k({rational(1, 2)});
    CHECK(!k.is_linear(symbol_fset{"x"}).first);
    k = k_type({rational(1), rational(0)});
    CHECK(k.is_linear(symbol_fset{"x", "y"}).first);
    CHECK(k.is_linear(symbol_fset{"x", "y"}).second == 0u);
    k = k_type({rational(2), rational(1)});
    CHECK(!k.is_linear(symbol_fset{"x", "y"}).first);
}

struct pow_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> k_type;
            k_type k1;
            CHECK(k1 == k1.pow(42, symbol_fset{}));
            CHECK_THROWS_MATCHES(
                k1.pow(42, symbol_fset{"x"}), std::invalid_argument,
                    test::ExceptionMatcher<std::invalid_argument>(std::string("invalid symbol set for the exponentiation of a "
                                  "monomial: the size of the symbol set (1) differs from the size of the monomial (0)"))
            );
            k1 = k_type({T(1), T(2), T(3)});
            CHECK(k1.pow(2, symbol_fset{"x", "y", "z"}) == k_type({T(2), T(4), T(6)}));
            CHECK(k1.pow(-2, symbol_fset{"x", "y", "z"}) == k_type({T(-2), T(-4), T(-6)}));
            CHECK(k1.pow(0, symbol_fset{"x", "y", "z"}) == k_type({T(0), T(0), T(0)}));
            CHECK_THROWS_MATCHES(
                k1.pow(42, symbol_fset{"x", "y", "z", "a"}), std::invalid_argument,
                       test::ExceptionMatcher<std::invalid_argument>(std::string("invalid symbol set for the exponentiation of a "
                                  "monomial: the size of the symbol set (4) differs from the size of the monomial (3)"))
            );
            T tmp;
            check_overflow(k1, tmp);
        }
    };
    template <typename KType, typename U, typename std::enable_if<std::is_integral<U>::value, int>::type = 0>
    static void check_overflow(const KType &, const U &)
    {
        KType k2({2});
        CHECK_THROWS_AS(k2.pow(std::numeric_limits<U>::max(), symbol_fset{"x"}), std::overflow_error);
    }
    template <typename KType, typename U, typename std::enable_if<!std::is_integral<U>::value, int>::type = 0>
    static void check_overflow(const KType &, const U &)
    {
    }
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_pow_test")
{
    tuple_for_each(expo_types{}, pow_tester{});
}

// NOTE: this does not have the pre-decrement operator.
struct fake_int {
    fake_int();
    explicit fake_int(int);
    fake_int(const fake_int &);
    fake_int(fake_int &&) noexcept;
    fake_int &operator=(const fake_int &);
    fake_int &operator=(fake_int &&) noexcept;
    ~fake_int();
    bool operator==(const fake_int &) const;
    bool operator!=(const fake_int &) const;
    bool operator<(const fake_int &) const;
    fake_int operator+(const fake_int &) const;
    fake_int &operator+=(const fake_int &);
    friend std::ostream &operator<<(std::ostream &, const fake_int &);
};

struct fake_int_01 {
    fake_int_01();
    explicit fake_int_01(int);
    fake_int_01(const fake_int_01 &);
    fake_int_01(fake_int_01 &&) noexcept;
    fake_int_01 &operator=(const fake_int_01 &);
    fake_int_01 &operator=(fake_int_01 &&) noexcept;
    ~fake_int_01();
    bool operator==(const fake_int_01 &) const;
    bool operator!=(const fake_int_01 &) const;
    bool operator<(const fake_int_01 &) const;
    fake_int_01 operator+(const fake_int_01 &) const;
    fake_int_01 &operator+=(const fake_int_01 &);
    fake_int_01 &operator--();
    friend std::ostream &operator<<(std::ostream &, const fake_int_01 &);
};

namespace piranha
{

namespace math
{

template <>
struct negate_impl<fake_int> {
    void operator()(fake_int &) const;
};

template <>
struct negate_impl<fake_int_01> {
    void operator()(fake_int_01 &) const;
};
}
}

namespace std
{

template <>
struct hash<fake_int> {
    typedef size_t result_type;
    typedef fake_int argument_type;
    result_type operator()(const argument_type &) const;
};

template <>
struct hash<fake_int_01> {
    typedef size_t result_type;
    typedef fake_int_01 argument_type;
    result_type operator()(const argument_type &) const;
};
}

struct partial_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> k_type;
            CHECK(key_is_differentiable<k_type>::value);
            k_type k1;
            CHECK_THROWS_MATCHES(
                k1.partial(0, symbol_fset{"x"}), std::invalid_argument, 
                test::ExceptionMatcher<std::invalid_argument>(std::string("invalid symbol set for the computation of the partial derivative of a "
                                  "monomial: the size of the symbol set (1) differs from the size of the monomial (0)"))
            );
            k1 = k_type({T(2)});
            auto ret = k1.partial(0, symbol_fset{"x"});
            CHECK(ret.first == T(2));
            CHECK(ret.second == k_type({T(1)}));
            // Derivative wrt a variable not in the monomial.
            ret = k1.partial(1, symbol_fset{"x"});
            CHECK(ret.first == T(0));
            CHECK(ret.second == k_type{symbol_fset{"x"}});
            // Derivative wrt a variable which has zero exponent.
            k1 = k_type({T(0)});
            ret = k1.partial(0, symbol_fset{"x"});
            CHECK(ret.first == T(0));
            CHECK(ret.second == k_type{symbol_fset{"x"}});
            k1 = k_type({T(-1), T(0)});
            ret = k1.partial(1, symbol_fset{"x", "y"});
            // y has zero exponent.
            CHECK(ret.first == T(0));
            CHECK((ret.second == k_type{symbol_fset{"x", "y"}}));
            ret = k1.partial(0, symbol_fset{"x", "y"});
            CHECK(ret.first == T(-1));
            CHECK(ret.second == k_type({T(-2), T(0)}));
            // Check the overflow check.
            overflow_check(k1);
        }
        template <typename T2, typename U2, typename std::enable_if<std::is_integral<T2>::value, int>::type = 0>
        static void overflow_check(const monomial<T2, U2> &)
        {
            monomial<T2, U2> k({std::numeric_limits<T2>::min()});
            CHECK_THROWS_AS(k.partial(0, symbol_fset{"x"}), std::overflow_error);
        }
        template <typename T2, typename U2, typename std::enable_if<!std::is_integral<T2>::value, int>::type = 0>
        static void overflow_check(const monomial<T2, U2> &)
        {
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
        // fake_int has no subtraction operators.
        CHECK((!key_is_differentiable<monomial<fake_int>>::value));
        CHECK((key_is_differentiable<monomial<fake_int_01>>::value));
    }
};

TEST_CASE("monomial_partial_test")
{
    tuple_for_each(expo_types{}, partial_tester{});
}

struct evaluate_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            using k_type = monomial<T, U>;
            CHECK((key_is_evaluable<k_type, integer>::value));
            k_type k1;
            CHECK((std::is_same<decltype(k1.evaluate(std::vector<integer>{}, symbol_fset{})),
                                      decltype(piranha::pow(integer{}, T{}))>::value));
            CHECK(k1.evaluate(std::vector<integer>{}, symbol_fset{}) == 1);
            CHECK_THROWS_MATCHES(k1.evaluate(std::vector<integer>{}, symbol_fset{"x"}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("cannot evaluate monomial: the size of the "
                                                                       "symbol set (1) differs from the size of the "
                                                                       "monomial (0)"))
            );
            CHECK_THROWS_MATCHES(k1.evaluate(std::vector<integer>{1_z}, symbol_fset{}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("cannot evaluate monomial: the size of the "
                                                                       "vector of values (1) differs from the size of "
                                                                       "the monomial (0)"))
            );
            k1 = k_type({T(1)});
            CHECK_THROWS_MATCHES(k1.evaluate(std::vector<integer>{}, symbol_fset{}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("cannot evaluate monomial: the size of the "
                                                                       "symbol set (0) differs from the size of the "
                                                                       "monomial (1)"))
            );
            CHECK_THROWS_MATCHES(k1.evaluate(std::vector<integer>{}, symbol_fset{"x"}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("cannot evaluate monomial: the size of the "
                                                                       "vector of values (0) differs from the size of "
                                                                       "the monomial (1)"))
            );
            CHECK(k1.evaluate(std::vector<integer>{-4_z}, symbol_fset{"x"}) == -4);
            k1 = k_type({T(2)});
            CHECK(k1.evaluate(std::vector<integer>{-4_z}, symbol_fset{"x"}) == 16);
            k1 = k_type({T(2), T(4)});
            CHECK(k1.evaluate(std::vector<integer>{3_z, 4_z}, symbol_fset{"x", "y"}) == 2304);
            CHECK((std::is_same<decltype(k1.evaluate(std::vector<double>{3.2, -4.3}, symbol_fset{"x", "y"})),
                                      decltype(piranha::pow(double{}, T{}))>::value));
            CHECK(k1.evaluate(std::vector<double>{3.2, -4.3}, symbol_fset{"x", "y"}) ==
                              piranha::pow(3.2, 2) * piranha::pow(-4.3, 4));
            CHECK((std::is_same<decltype(k1.evaluate(std::vector<rational>{rational(4, -3), rational(-1, -2)},
                                                           symbol_fset{"x", "y"})),
                                      decltype(piranha::pow(rational{}, T{}))>::value));
            CHECK(
                k1.evaluate(std::vector<rational>{rational(4, -3), rational(-1, -2)}, symbol_fset{"x", "y"}) ==
                piranha::pow(rational(4, -3), 2) * piranha::pow(rational(-1, -2),4));
            k1 = k_type({T(-2), T(-4)});
            CHECK(
                k1.evaluate(std::vector<rational>{rational(4, -3), rational(-1, -2)}, symbol_fset{"x", "y"}) ==
                piranha::pow(rational(4, -3), -2) * piranha::pow(rational(-1, -2), -4));
#if defined(MPPP_WITH_MPFR)
            CHECK(
                (std::is_same<decltype(k1.evaluate(std::vector<real>{real(5.678), real(1.234)}, symbol_fset{"x", "y"})),
                              decltype(piranha::pow(real{}, T{}))>::value));
            CHECK(k1.evaluate(std::vector<real>{real(5.678), real(1.234)}, symbol_fset{"x", "y"}) ==
                              piranha::pow(real(5.678), T(-2)) * piranha::pow(real(1.234), T(-4)));
#endif
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_evaluate_test")
{
    tuple_for_each(expo_types{}, evaluate_tester{});
    CHECK((key_is_evaluable<monomial<rational>, double>::value));
#if defined(MPPP_WITH_MPFR)
    CHECK((key_is_evaluable<monomial<rational>, real>::value));
#endif
    CHECK((!key_is_evaluable<monomial<rational>, std::string>::value));
    CHECK((!key_is_evaluable<monomial<rational>, void *>::value));
    CHECK((!key_is_evaluable<monomial<rational>, void>::value));
}

struct subs_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> k_type;
            k_type k1;
            // Test the type trait.
            CHECK((key_has_subs<k_type, integer>::value));
            CHECK((key_has_subs<k_type, rational>::value));
#if defined(MPPP_WITH_MPFR)
            CHECK((key_has_subs<k_type, real>::value));
#endif
            CHECK((key_has_subs<k_type, double>::value));
            CHECK((!key_has_subs<k_type, std::string>::value));
            CHECK((!key_has_subs<k_type, std::vector<std::string>>::value));
            CHECK((!key_has_subs<k_type, void>::value));
            auto ret = k1.template subs<integer>({}, symbol_fset{});
            CHECK(ret.size() == 1u);
            CHECK(ret[0].first == 1u);
            CHECK(ret[0].second == k1);
            CHECK((std::is_same<decltype(ret[0].first), decltype(piranha::pow(integer{}, T{}))>::value));
            CHECK_THROWS_MATCHES(k1.template subs<integer>({}, symbol_fset{"x"}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("cannot perform substitution in a monomial: the "
                                                                       "size of the symbol set (1) differs from the "
                                                                       "size of the monomial (0)"))
            );
            CHECK_THROWS_MATCHES(k1.template subs<integer>({{0, 1_z}}, symbol_fset{}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for substitution in a "
                                                                       "monomial: the last index of the substitution "
                                                                       "map (0) must be smaller than the monomial's "
                                                                       "size (0)"))
            );
            k1 = k_type({T(2)});
            ret = k1.template subs<integer>({{0, 4_z}}, symbol_fset{"x"});
            CHECK(ret.size() == 1u);
            CHECK(ret[0].first == 16);
            CHECK(ret[0].second == k_type({T(0)}));
            k1 = k_type({T(2), T(3)});
            ret = k1.template subs<integer>({{1, -2_z}}, symbol_fset{"x", "y"});
            CHECK(ret.size() == 1u);
            CHECK(ret[0u].first == -8);
            CHECK((ret[0u].second == k_type{T(2), T(0)}));
#if defined(MPPP_WITH_MPFR)
            auto ret2 = k1.template subs<real>({{0, real(-2.345)}}, symbol_fset{"x", "y"});
            CHECK((std::is_same<real, decltype(ret2[0u].first)>::value));
            CHECK(ret2.size() == 1u);
            CHECK(ret2[0u].first == piranha::pow(real(-2.345), T(2)));
            CHECK((ret2[0u].second == k_type{T(0), T(3)}));
#endif
            auto ret3 = k1.template subs<rational>({{0, -1_q / 2}}, symbol_fset{"x", "y"});
            CHECK(ret3.size() == 1u);
            CHECK(ret3[0u].first == rational(1, 4));
            CHECK((ret3[0u].second == k_type{T(0), T(3)}));
            CHECK((std::is_same<rational, decltype(ret3[0u].first)>::value));
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_subs_test")
{
    tuple_for_each(expo_types{}, subs_tester{});
}

struct print_tex_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> k_type;
            k_type k1;
            std::ostringstream oss;
            k1.print_tex(oss, symbol_fset{});
            CHECK(oss.str().empty());
            k1 = k_type({T(0)});
            CHECK_THROWS_AS(k1.print_tex(oss, symbol_fset{}), std::invalid_argument);
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
            CHECK_THROWS_MATCHES(
                k1.print_tex(oss, symbol_fset{"x"}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string(
                    "cannot print monomial in TeX mode: the size of the symbol set "
                    "(1) differs from the size of the monomial (2)"))
            );
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
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_print_tex_test")
{
    tuple_for_each(expo_types{}, print_tex_tester{});
}

struct integrate_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> k_type;
            CHECK(key_is_integrable<k_type>::value);
            k_type k1;
            auto ret = k1.integrate("a", symbol_fset{});
            CHECK(ret.first == T(1));
            CHECK(ret.second == k_type({T(1)}));
            CHECK_THROWS_MATCHES(
                k1.integrate("b", symbol_fset{"b"}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string(
                    "invalid symbol set for the computation of the antiderivative of a "
                    "monomial: the size of the symbol set (1) differs from the size of the monomial (0)"))
            );
            k1 = k_type{T(1)};
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
            k1 = k_type{T(2), T(3)};
            ret = k1.integrate("a", symbol_fset{"b", "d"});
            CHECK(ret.first == T(1));
            CHECK(ret.second == k_type({T(1), T(2), T(3)}));
            ret = k1.integrate("b", symbol_fset{"b", "d"});
            CHECK(ret.first == T(3));
            CHECK(ret.second == k_type({T(3), T(3)}));
            ret = k1.integrate("c", symbol_fset{"b", "d"});
            CHECK(ret.first == T(1));
            CHECK(ret.second == k_type({T(2), T(1), T(3)}));
            ret = k1.integrate("d", symbol_fset{"b", "d"});
            CHECK(ret.first == T(4));
            CHECK(ret.second == k_type({T(2), T(4)}));
            ret = k1.integrate("e", symbol_fset{"b", "d"});
            CHECK(ret.first == T(1));
            CHECK(ret.second == k_type({T(2), T(3), T(1)}));
            k1 = k_type{T(-1), T(3)};
            CHECK_THROWS_MATCHES(
                k1.integrate("b", symbol_fset{"b", "d"}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string(
                                           "unable to perform monomial integration: a negative "
                                           "unitary exponent was encountered in correspondence of the variable 'b'"))
            );
            k1 = k_type{T(2), T(-1)};
            CHECK_THROWS_MATCHES(
                k1.integrate("d", symbol_fset{"b", "d"}), std::invalid_argument, test::ExceptionMatcher<std::invalid_argument>(std::string(
                                           "unable to perform monomial integration: a negative "
                                           "unitary exponent was encountered in correspondence of the variable 'd'"))
            );
            // Overflow check.
            overflow_check(k1);
        }
    };
    template <typename U, typename std::enable_if<std::is_integral<typename U::value_type>::value, int>::type = 0>
    static void overflow_check(const U &)
    {
        using k_type = U;
        using T = typename k_type::value_type;
        k_type k1{T(1), std::numeric_limits<T>::max()};
        auto ret = k1.integrate("a", symbol_fset{"a", "b"});
        CHECK(ret.first == T(2));
        CHECK((ret.second == k_type{T(2), std::numeric_limits<T>::max()}));
        CHECK_THROWS_AS(k1.integrate("b", symbol_fset{"a", "b"}), std::overflow_error);
    }
    template <typename U, typename std::enable_if<!std::is_integral<typename U::value_type>::value, int>::type = 0>
    static void overflow_check(const U &)
    {
    }
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_integrate_test")
{
    tuple_for_each(expo_types{}, integrate_tester{});
}

struct ipow_subs_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> k_type;
            // Test the type trait.
            CHECK((key_has_ipow_subs<k_type, integer>::value));
            CHECK((key_has_ipow_subs<k_type, double>::value));
#if defined(MPPP_WITH_MPFR)
            CHECK((key_has_ipow_subs<k_type, real>::value));
#endif
            CHECK((key_has_ipow_subs<k_type, rational>::value));
            CHECK((!key_has_ipow_subs<k_type, std::string>::value));
            CHECK((!key_has_ipow_subs<k_type, void>::value));
            k_type k1;
            auto ret = k1.ipow_subs(0, 45_z, 4_z, symbol_fset{});
            CHECK(ret.size() == 1u);
            CHECK(ret[0u].first == 1);
            CHECK((std::is_same<integer, decltype(ret[0u].first)>::value));
            CHECK(ret[0u].second == k1);
            CHECK_THROWS_MATCHES(k1.ipow_subs(0, 35_z, 4_z, symbol_fset{"x"}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("cannot perform integral power substitution in "
                                                                       "a monomial: the size of the symbol set (1) "
                                                                       "differs from the size of the monomial (0)"))
            );
            k1 = k_type({T(2)});
            ret = k1.ipow_subs(1u, integer(2), integer(4), symbol_fset{"x"});
            CHECK(ret.size() == 1u);
            CHECK(ret[0u].first == 1);
            CHECK(ret[0u].second == k1);
            ret = k1.ipow_subs(0, integer(1), integer(4), symbol_fset{"x"});
            CHECK(ret.size() == 1u);
            CHECK(ret[0u].first == piranha::pow(integer(4), T(2)));
            CHECK(ret[0u].second == k_type({T(0)}));
            ret = k1.ipow_subs(0, integer(2), integer(4), symbol_fset{"x"});
            CHECK(ret.size() == 1u);
            CHECK(ret[0u].first == piranha::pow(integer(4), T(1)));
            CHECK(ret[0u].second == k_type({T(0)}));
            ret = k1.ipow_subs(0, integer(-1), integer(4), symbol_fset{"x"});
            CHECK(ret.size() == 1u);
            CHECK(ret[0u].first == 1);
            CHECK(ret[0u].second == k_type({T(2)}));
            ret = k1.ipow_subs(0, integer(4), integer(4), symbol_fset{"x"});
            CHECK(ret.size() == 1u);
            CHECK(ret[0u].first == 1);
            CHECK(ret[0u].second == k_type({T(2)}));
            k1 = k_type({T(7), T(2)});
            CHECK_THROWS_MATCHES(k1.ipow_subs(0, integer(4), integer(4), symbol_fset{"x"}), std::invalid_argument,
                                  test::ExceptionMatcher<std::invalid_argument>(std::string("cannot perform integral power substitution in "
                                                                       "a monomial: the size of the symbol set (1) "
                                                                       "differs from the size of the monomial (2)"))
            );
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
            CHECK_THROWS_MATCHES(
                k1.ipow_subs(1, integer(0), integer(2), symbol_fset{"x", "y"}), std::invalid_argument,
                test::ExceptionMatcher<std::invalid_argument>(std::string("invalid integral power for ipow_subs() in a monomial: the power must be nonzero"))
            );
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
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(size_types{}, runner<T>{});
    }
};

TEST_CASE("monomial_ipow_subs_test")
{
    tuple_for_each(expo_types{}, ipow_subs_tester{});
}

struct tt_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            typedef monomial<T, U> k_type;
            CHECK((!key_has_t_subs<k_type, int, int>::value));
            CHECK((!key_has_t_subs<k_type &, int, int>::value));
            CHECK((!key_has_t_subs<k_type &&, int, int>::value));
            CHECK((!key_has_t_subs<const k_type &, int, int>::value));
            CHECK(is_container_element<k_type>::value);
            CHECK(is_hashable<k_type>::value);
            CHECK(is_key_degree_type<k_type>::value);
            CHECK(is_key_ldegree_type<k_type>::value);
            CHECK(!key_has_t_degree<k_type>::value);
            CHECK(!key_has_t_ldegree<k_type>::value);
            CHECK(!key_has_t_order<k_type>::value);
            CHECK(!key_has_t_lorder<k_type>::value);
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(expo_types{}, ipow_subs_tester{});
    }
};

TEST_CASE("monomial_type_traits_test")
{
    tuple_for_each(expo_types{}, tt_tester{});
}

TEST_CASE("monomial_kic_test")
{
    using k_type_00 = monomial<int>;
    using k_type_01 = monomial<long>;
    using k_type_02 = monomial<long>;
    CHECK((key_is_convertible<k_type_00, k_type_00>::value));
    CHECK((key_is_convertible<k_type_01, k_type_01>::value));
    CHECK((key_is_convertible<k_type_02, k_type_02>::value));
    CHECK((key_is_convertible<k_type_00, k_type_01>::value));
    CHECK((key_is_convertible<k_type_01, k_type_00>::value));
    CHECK((key_is_convertible<k_type_00, k_type_02>::value));
    CHECK((key_is_convertible<k_type_02, k_type_00>::value));
    CHECK((key_is_convertible<k_type_01, k_type_02>::value));
    CHECK((key_is_convertible<k_type_02, k_type_01>::value));
    CHECK((!key_is_convertible<k_type_00, k_monomial>::value));
    CHECK((!key_is_convertible<k_monomial, k_type_00>::value));
}

TEST_CASE("monomial_comparison_test")
{
    using k_type_00 = monomial<int>;
    CHECK(is_less_than_comparable<k_type_00>::value);
    CHECK(!(k_type_00{} < k_type_00{}));
    CHECK(!(k_type_00{3} < k_type_00{2}));
    CHECK(!(k_type_00{3} < k_type_00{3}));
    CHECK(k_type_00{2} < k_type_00{3});
    CHECK((k_type_00{2, 3} < k_type_00{2, 4}));
    CHECK(!(k_type_00{2, 2} < k_type_00{2, 2}));
    CHECK((k_type_00{1, 3} < k_type_00{2, 1}));
    CHECK(!(k_type_00{1, 2, 3, 4} < k_type_00{1, 2, 3, 4}));
    CHECK_THROWS_AS((void)(k_type_00{} < k_type_00{1}), std::invalid_argument);
    CHECK_THROWS_AS((void)(k_type_00{1} < k_type_00{}), std::invalid_argument);
}
