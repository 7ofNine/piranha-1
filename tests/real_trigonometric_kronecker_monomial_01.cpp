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

#include <piranha/real_trigonometric_kronecker_monomial.hpp>

#include <array>
#include <boost/algorithm/string/predicate.hpp>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <mp++/config.hpp>
#if defined(MPPP_WITH_MPFR)
#include <mp++/real.hpp>
#endif

#include <piranha/integer.hpp>
#include <piranha/key/key_degree.hpp>
#include <piranha/key/key_is_one.hpp>
#include <piranha/key/key_is_zero.hpp>
#include <piranha/key/key_ldegree.hpp>
#include <piranha/key_is_convertible.hpp>
#include <piranha/key_is_multipliable.hpp>
#include <piranha/kronecker_array.hpp>
#include <piranha/math.hpp>
#include <piranha/math/cos.hpp>
#include <piranha/math/sin.hpp>
#include <piranha/monomial.hpp>
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

using int_types = std::tuple<signed char, int, long, long long>;

// Constructors, assignments, getters, setters, etc.
struct constructor_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        typedef kronecker_array<T> ka;
        CHECK((std::is_constructible<k_type, std::initializer_list<int>>::value));
        CHECK((std::is_constructible<k_type, std::initializer_list<integer>>::value));
        CHECK((std::is_constructible<k_type, std::initializer_list<rational>>::value));
        CHECK((std::is_constructible<k_type, std::initializer_list<double>>::value));
        k_type k1;
        CHECK(k1.get_int() == 0);
        CHECK(k1.get_flavour() == true);
        k_type k2({-1, -1});
        std::vector<T> v2(2);
        ka::decode(v2, k2.get_int());
        CHECK(v2[0] == -1);
        CHECK(v2[1] == -1);
        CHECK(k2.get_flavour() == true);
        k_type k2a({-1_z, -1_z});
        ka::decode(v2, k2.get_int());
        CHECK(v2[0] == -1);
        CHECK(v2[1] == -1);
        CHECK_THROWS_AS(k_type({-1 / 3_q, -1_q}), std::invalid_argument);
        k_type k3;
        CHECK(k3.get_int() == 0);
        CHECK(k3.get_flavour() == true);
        k_type k4({10});
        CHECK(k4.get_int() == 10);
        CHECK(k4.get_flavour());
        k_type k5(symbol_fset({}));
        CHECK(k5.get_int() == 0);
        CHECK(k5.get_flavour());
        k_type k6(symbol_fset({"a"}));
        CHECK(k6.get_int() == 0);
        CHECK(k6.get_flavour() == true);
        k_type k7(symbol_fset({"a", "b"}));
        CHECK(k7.get_int() == 0);
        CHECK(k7.get_flavour());
        k_type k8(0, true);
        CHECK(k8.get_int() == 0);
        CHECK(k8.get_flavour());
        k_type k9(1, true);
        CHECK(k9.get_int() == 1);
        CHECK(k9.get_flavour());
        CHECK((k_type(1, false).get_int()) == 1);
        CHECK(!(k_type(1, false).get_flavour()));
        k_type k10;
        k10.set_int(10);
        CHECK(k10.get_int() == 10);
        CHECK(k10.get_flavour());
        k10.set_flavour(false);
        CHECK(!k10.get_flavour());
        k_type k11;
        CHECK(k11.get_flavour());
        k11 = k10;
        CHECK(k11.get_int() == 10);
        CHECK(!k11.get_flavour());
        k11 = std::move(k9);
        CHECK(k9.get_int() == 1);
        CHECK(k11.get_flavour());
        // Constructor from iterators.
        CHECK((std::is_constructible<k_type, int *, int *>::value));
        CHECK((std::is_constructible<k_type, integer *, integer *>::value));
        CHECK((std::is_constructible<k_type, double *, double *>::value));
        v2 = {};
        k_type k12(v2.begin(), v2.end());
        CHECK(k12.get_int() == 0);
        CHECK(k12.get_flavour() == true);
        v2 = {21};
        k_type k13(v2.begin(), v2.end());
        CHECK(k13.get_int() == 21);
        v2 = {-21};
        k_type k14(v2.begin(), v2.end());
        CHECK(k14.get_int() == -21);
        v2 = {1, -2};
        k_type k15(v2.begin(), v2.end());
        auto v = k15.unpack(symbol_fset({"a", "b"}));
        CHECK(v.size() == 2u);
        CHECK(v[0u] == 1);
        CHECK(v[1u] == -2);
        double tmp_d[] = {1., -1.};
        k_type k15a(&tmp_d[0], &tmp_d[0] + 2);
        v = k15a.unpack(symbol_fset({"a", "b"}));
        CHECK(v.size() == 2u);
        CHECK(v[0u] == 1);
        CHECK(v[1u] == -1);
        tmp_d[0] = -.5;
        CHECK_THROWS_AS(k_type(&tmp_d[0], &tmp_d[0] + 1), std::invalid_argument);
        CHECK((std::is_constructible<k_type, T *, T *>::value));
        // Iterators have to be of homogeneous type.
        CHECK((!std::is_constructible<k_type, T *, T const *>::value));
        CHECK((std::is_constructible<k_type, typename std::vector<T>::iterator,
                                           typename std::vector<T>::iterator>::value));
        CHECK((!std::is_constructible<k_type, typename std::vector<T>::const_iterator,
                                            typename std::vector<T>::iterator>::value));
        CHECK((!std::is_constructible<k_type, typename std::vector<T>::iterator, int>::value));
        // Converting constructor.
        k_type k16, k17(k16, symbol_fset{});
        CHECK(k17.get_flavour());
        CHECK(!(k_type(k_type(0, false), symbol_fset{}).get_flavour()));
        CHECK(k16 == k17);
        k16.set_int(10);
        k_type k18(k16, symbol_fset({"a"}));
        CHECK(k16 == k18);
    }
};

TEST_CASE("rtkm_constructor_test")
{
#if defined(MPPP_WITH_MPFR)
    //mppp::real_set_default_prec(100); is that really used. is there a mppp::real used ???
#endif
    tuple_for_each(int_types{}, constructor_tester{});
}

struct compatibility_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        typedef kronecker_array<T> ka;
        const auto &limits = ka::get_limits();
        k_type k1;
        CHECK(k1.is_compatible(symbol_fset({})));
        k1.set_int(1);
        CHECK(!k1.is_compatible(symbol_fset({})));
        if (limits.size() < 255u) {
            symbol_fset v2;
            for (auto i = 0u; i < 255; ++i) {
                v2.emplace_hint(v2.end(), std::string(1u, static_cast<char>(i)));
            }
            CHECK(!k1.is_compatible(v2));
        }
        k1.set_int(std::numeric_limits<T>::max());
        CHECK(!k1.is_compatible(symbol_fset({"a", "b"})));
        k1.set_int(1);
        CHECK(k1.is_compatible(symbol_fset({"a", "b"})));
        // Negative first element.
        k1 = k_type{-1, 0};
        CHECK(!k1.is_compatible(symbol_fset({"a", "b"})));
        // Negative first nonzero element.
        k1 = k_type{0, -1};
        CHECK(!k1.is_compatible(symbol_fset({"a", "b"})));
        k1 = k_type{1, 0};
        CHECK(k1.is_compatible(symbol_fset({"a", "b"})));
        k1 = k_type{0, 1};
        CHECK(k1.is_compatible(symbol_fset({"a", "b"})));
        k1 = k_type{1, -1};
        CHECK(k1.is_compatible(symbol_fset({"a", "b"})));
        k1 = k_type{0, 0};
        CHECK(k1.is_compatible(symbol_fset({"a", "b"})));
    }
};

TEST_CASE("rtkm_compatibility_test")
{
    tuple_for_each(int_types{}, compatibility_tester{});
}

struct is_zero_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        CHECK(!piranha::key_is_zero(k_type(), symbol_fset{}));
        CHECK(!piranha::key_is_zero(k_type(symbol_fset{"a"}), symbol_fset{}));
        CHECK(!piranha::key_is_zero(k_type{0, 0}, symbol_fset{}));
        CHECK(!piranha::key_is_zero(k_type(1, false), symbol_fset{"a"}));
        CHECK(piranha::key_is_zero(k_type(0, false), symbol_fset{"a"}));
        k_type k{0, -1};
        k.set_flavour(false);
        CHECK(!piranha::key_is_zero(k, symbol_fset{}));
    }
};

TEST_CASE("rtkm_is_zero_test")
{
    tuple_for_each(int_types{}, is_zero_tester{});
}

struct merge_symbols_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = real_trigonometric_kronecker_monomial<T>;
        k_type k1;
        CHECK_THROWS_MATCHES(
            k1.merge_symbols({}, symbol_fset{}), std::invalid_argument, 
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for symbol set merging: the insertion map cannot be empty"))
       );
        k1 = k_type(0, false);
        CHECK_THROWS_MATCHES(
            k1.merge_symbols({}, symbol_fset{}), std::invalid_argument, 
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for symbol set merging: the insertion map cannot be empty"))
        );
        CHECK_THROWS_MATCHES(
            k1.merge_symbols({}, symbol_fset{"d"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for symbol set merging: the insertion map cannot be empty"))
        );
        k1 = k_type(0, true);
        CHECK((k1.merge_symbols({{0, {"a", "b"}}}, symbol_fset{"d"}) == k_type{0, 0, 0}));
        k1 = k_type(0, false);
        CHECK((k1.merge_symbols({{0, {"a", "b"}}}, symbol_fset{"d"}) == k_type(0, false)));
        CHECK((k_type{1}.merge_symbols({{0, {"a", "b"}}}, symbol_fset{"d"}) == k_type{0, 0, 1}));
        CHECK((k_type{1, false}.merge_symbols({{0, {"a", "b"}}}, symbol_fset{"d"}) == k_type{{0, 0, 1}, false}));
        CHECK((k_type{1}.merge_symbols({{1, {"e", "f"}}}, symbol_fset{"d"}) == k_type{1, 0, 0}));
        CHECK((k_type{1, false}.merge_symbols({{1, {"e", "f"}}}, symbol_fset{"d"}) == k_type{{1, 0, 0}, false}));
        CHECK((k_type{1, 1}.merge_symbols({{0, {"a", "b"}}}, symbol_fset{"d", "n"}) == k_type{0, 0, 1, 1}));
        CHECK((k_type{{1, 1}, false}.merge_symbols({{0, {"a", "b"}}}, symbol_fset{"d", "n"})
                     == k_type{{0, 0, 1, 1}, false}));
        CHECK((k_type{1, 1}.merge_symbols({{1, {"e", "f"}}}, symbol_fset{"d", "n"}) == k_type{1, 0, 0, 1}));
        CHECK((k_type{{1, 1}, false}.merge_symbols({{1, {"e", "f"}}}, symbol_fset{"d", "n"})
                     == k_type{{1, 0, 0, 1}, false}));
        CHECK((k_type{1, 1}.merge_symbols({{2, {"f", "g"}}}, symbol_fset{"d", "e"}) == k_type{1, 1, 0, 0}));
        CHECK((k_type{{1, 1}, false}.merge_symbols({{2, {"f", "g"}}}, symbol_fset{"d", "e"})
                     == k_type{{1, 1, 0, 0}, false}));
        CHECK(
            (k_type{-1, -1}.merge_symbols({{0, {"a"}}, {2, {"f"}}}, symbol_fset{"d", "e"}) == k_type{0, -1, -1, 0}));
        CHECK((k_type{{-1, -1}, false}.merge_symbols({{0, {"a"}}, {2, {"f"}}}, symbol_fset{"d", "e"})
                     == k_type{{0, -1, -1, 0}, false}));
        CHECK((k_type{-1, -1}.merge_symbols({{0, {"a"}}, {1, std::initializer_list<std::string>{}}, {2, {"f"}}},
                                                  symbol_fset{"d", "e"})
                     == k_type{0, -1, -1, 0}));
        CHECK((k_type{{-1, -1}, false}.merge_symbols(
                         {{0, {"a"}}, {1, std::initializer_list<std::string>{}}, {2, {"f"}}}, symbol_fset{"d", "e"})
                     == k_type{{0, -1, -1, 0}, false}));
        CHECK_THROWS_MATCHES((k_type{1, 1}.merge_symbols({{3, {"f", "g"}}}, symbol_fset{"d", "e"})),
                              std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for symbol set merging: the "
                                                                   "last index of the insertion map (3) must not be "
                                                                   "greater than the key's size (2)"))
        );
        CHECK_THROWS_MATCHES((k_type{{1, 1}, false}.merge_symbols({{3, {"f", "g"}}}, symbol_fset{"d", "e"})),
                              std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for symbol set merging: the "
                                                                   "last index of the insertion map (3) must not be "
                                                                   "greater than the key's size (2)"))
        );
        if (std::numeric_limits<T>::max() >= std::numeric_limits<int>::max()) {
            CHECK((k_type{-1, -1}.merge_symbols({{0, {"a"}}, {2, {"f"}}, {1, {"b"}}}, symbol_fset{"d", "e"})
                         == k_type{0, -1, 0, -1, 0}));
            CHECK(
                (k_type{{-1, -1}, false}.merge_symbols({{0, {"a"}}, {2, {"f"}}, {1, {"b"}}}, symbol_fset{"d", "e"})
                 == k_type{{0, -1, 0, -1, 0}, false}));
            CHECK(
                (k_type{-1, -1, 3}.merge_symbols({{0, {"a"}}, {3, {"f"}}, {1, {"b"}}}, symbol_fset{"d", "e1", "e2"})
                 == k_type{0, -1, 0, -1, 3, 0}));
            CHECK((k_type{{-1, -1, 3}, false}.merge_symbols({{0, {"a"}}, {3, {"f"}}, {1, {"b"}}},
                                                                  symbol_fset{"d", "e1", "e2"})
                         == k_type{{0, -1, 0, -1, 3, 0}, false}));
        }
    }
};

TEST_CASE("rtkm_merge_symbols_test")
{
    tuple_for_each(int_types{}, merge_symbols_tester{});
}

struct key_is_one_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        typedef kronecker_array<T> ka;
        k_type k1;
        CHECK(piranha::key_is_one(k1, symbol_fset{}));
        k_type k2({1});
        CHECK(!piranha::key_is_one(k2, symbol_fset{"a"}));
        k_type k3({0});
        CHECK(piranha::key_is_one(k3, symbol_fset{"a"}));
        k_type k4({0, 0});
        CHECK(piranha::key_is_one(k4, symbol_fset{"a", "b"}));
        k_type k5({0, 1});
        CHECK(!piranha::key_is_one(k5, symbol_fset{"a", "b"}));
        symbol_fset vs2;
        const auto &l = ka::get_limits();
        typedef decltype(l.size()) size_type;
        for (size_type i = 0u; i <= l.size(); ++i) {
            vs2.emplace_hint(vs2.end(), std::to_string(i));
        }
        k2 = k_type{-1};
        vs2 = symbol_fset({"a"});
        k2 = k_type{0};
        k2.set_flavour(false);
        CHECK(!piranha::key_is_one(k2, vs2));
        k2.set_flavour(true);
        CHECK(piranha::key_is_one(k2, vs2));
        k2 = k_type{1, 1};
        CHECK(!piranha::key_is_one(k2, vs2));
    }
};

TEST_CASE("rtkm_key_is_one_test")
{
    tuple_for_each(int_types{}, key_is_one_tester{});
}

struct t_degree_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        k_type k1;
        if (std::is_same<signed char, T>::value) {
            CHECK((std::is_same<decltype(k1.t_degree(symbol_fset{})), int>::value));
            CHECK((std::is_same<decltype(k1.t_ldegree(symbol_fset{})), int>::value));
            CHECK((std::is_same<decltype(k1.t_degree(symbol_idx_fset{}, symbol_fset{})), int>::value));
            CHECK((std::is_same<decltype(k1.t_ldegree(symbol_idx_fset{}, symbol_fset{})), int>::value));
        } else {
            CHECK((std::is_same<decltype(k1.t_degree(symbol_fset{})), T>::value));
            CHECK((std::is_same<decltype(k1.t_ldegree(symbol_fset{})), T>::value));
            CHECK((std::is_same<decltype(k1.t_degree(symbol_idx_fset{}, symbol_fset{})), T>::value));
            CHECK((std::is_same<decltype(k1.t_ldegree(symbol_idx_fset{}, symbol_fset{})), T>::value));
        }
        CHECK(k1.t_degree(symbol_fset{}) == 0);
        CHECK(k1.t_ldegree(symbol_fset{}) == 0);
        k_type k2({0});
        CHECK(k2.t_degree(symbol_fset{"a"}) == 0);
        CHECK(k2.t_ldegree(symbol_fset{"a"}) == 0);
        k_type k3({-1});
        CHECK(k3.t_degree(symbol_fset{"a"}) == -1);
        CHECK(k3.t_ldegree(symbol_fset{"a"}) == -1);
        k_type k4({0, 0});
        CHECK(k4.t_degree(symbol_fset{"a", "b"}) == 0);
        CHECK(k4.t_ldegree(symbol_fset{"a", "b"}) == 0);
        k_type k5({-1, -1});
        CHECK(k5.t_degree(symbol_fset{"a", "b"}) == -2);
        CHECK(k5.t_degree({0}, symbol_fset{"a", "b"}) == -1);
        CHECK(k5.t_degree(symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(k5.t_degree({0, 1}, symbol_fset{"a", "b"}) == -2);
        CHECK(k5.t_degree({1}, symbol_fset{"a", "b"}) == -1);
        CHECK(k5.t_ldegree(symbol_fset{"a", "b"}) == -2);
        CHECK(k5.t_ldegree({0}, symbol_fset{"a", "b"}) == -1);
        CHECK(k5.t_ldegree(symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(k5.t_ldegree({0, 1}, symbol_fset{"a", "b"}) == -2);
        CHECK(k5.t_ldegree({1}, symbol_fset{"a", "b"}) == -1);
        // Error checking.
        CHECK_THROWS_MATCHES(
            k5.t_degree({1, 2}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric degree of a real trigonometric Kronecker "
                                                 "monomial is 2, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_degree({3}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric degree of a real trigonometric Kronecker "
                                                 "monomial is 3, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_degree({1, 2, 3}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric degree of a real trigonometric Kronecker "
                                                 "monomial is 3, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_ldegree({1, 2}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric degree of a real trigonometric Kronecker "
                                                 "monomial is 2, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_ldegree({3}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric degree of a real trigonometric Kronecker "
                                                 "monomial is 3, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_ldegree({1, 2, 3}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric degree of a real trigonometric Kronecker "
                                                 "monomial is 3, but the monomial has a size of only 2"))
        );
    }
};

TEST_CASE("rtkm_t_degree_test")
{
    tuple_for_each(int_types{}, t_degree_tester{});
}

struct t_order_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        k_type k1;
        if (std::is_same<T, signed char>::value) {
            CHECK((std::is_same<decltype(k1.t_order(symbol_fset{})), int>::value));
            CHECK((std::is_same<decltype(k1.t_lorder(symbol_fset{})), int>::value));
            CHECK((std::is_same<decltype(k1.t_order(symbol_idx_fset{}, symbol_fset{})), int>::value));
            CHECK((std::is_same<decltype(k1.t_lorder(symbol_idx_fset{}, symbol_fset{})), int>::value));
        } else {
            CHECK((std::is_same<decltype(k1.t_order(symbol_fset{})), T>::value));
            CHECK((std::is_same<decltype(k1.t_lorder(symbol_fset{})), T>::value));
            CHECK((std::is_same<decltype(k1.t_order(symbol_idx_fset{}, symbol_fset{})), T>::value));
            CHECK((std::is_same<decltype(k1.t_lorder(symbol_idx_fset{}, symbol_fset{})), T>::value));
        }
        CHECK(k1.t_order(symbol_fset{}) == 0);
        CHECK(k1.t_lorder(symbol_fset{}) == 0);
        k_type k2({0});
        CHECK(k2.t_order(symbol_fset{"a"}) == 0);
        CHECK(k2.t_lorder(symbol_fset{"a"}) == 0);
        k_type k3({-1});
        CHECK(k3.t_order(symbol_fset{"a"}) == 1);
        CHECK(k3.t_lorder(symbol_fset{"a"}) == 1);
        k_type k4({0, 0});
        CHECK(k4.t_order(symbol_fset{"a"}) == 0);
        CHECK(k4.t_lorder(symbol_fset{"a"}) == 0);
        k_type k5({-1, -1});
        CHECK(k5.t_order(symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_order({0}, symbol_fset{"a", "b"}) == 1);
        CHECK(k5.t_order(symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(k5.t_order({0, 1}, symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_order({1}, symbol_fset{"a", "b"}) == 1);
        CHECK(k5.t_lorder(symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_lorder({0}, symbol_fset{"a", "b"}) == 1);
        CHECK(k5.t_lorder(symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(k5.t_lorder({0, 1}, symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_lorder({1}, symbol_fset{"a", "b"}) == 1);
        k5 = k_type({-1, 1});
        CHECK(k5.t_order(symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_order({0}, symbol_fset{"a", "b"}) == 1);
        CHECK(k5.t_order(symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(k5.t_order({0, 1}, symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_order({1}, symbol_fset{"a", "b"}) == 1);
        CHECK(k5.t_lorder(symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_lorder({0}, symbol_fset{"a", "b"}) == 1);
        CHECK(k5.t_lorder(symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(k5.t_lorder({0, 1}, symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_lorder({1}, symbol_fset{"a", "b"}) == 1);
        k5 = k_type({1, -1});
        CHECK(k5.t_order(symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_order({0}, symbol_fset{"a", "b"}) == 1);
        CHECK(k5.t_order(symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(k5.t_order({0, 1}, symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_order({1}, symbol_fset{"a", "b"}) == 1);
        CHECK(k5.t_lorder(symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_lorder({0}, symbol_fset{"a", "b"}) == 1);
        CHECK(k5.t_lorder(symbol_idx_fset{}, symbol_fset{"a", "b"}) == 0);
        CHECK(k5.t_lorder({0, 1}, symbol_fset{"a", "b"}) == 2);
        CHECK(k5.t_lorder({1}, symbol_fset{"a", "b"}) == 1);
        // Error checking.
        CHECK_THROWS_MATCHES(
            k5.t_order({1, 2}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric order of a real trigonometric Kronecker "
                                                 "monomial is 2, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_order({3}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric order of a real trigonometric Kronecker "
                                                 "monomial is 3, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_order({1, 2, 3}, symbol_fset{"a", "b"}), std::invalid_argument, 
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric order of a real trigonometric Kronecker "
                                                 "monomial is 3, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_lorder({1, 2}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric order of a real trigonometric Kronecker "
                                                 "monomial is 2, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_lorder({3}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric order of a real trigonometric Kronecker "
                                                 "monomial is 3, but the monomial has a size of only 2"))
        );
        CHECK_THROWS_MATCHES(
            k5.t_lorder({1, 2, 3}, symbol_fset{"a", "b"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the largest value in the positions set for the computation of the "
                                                 "partial trigonometric order of a real trigonometric Kronecker "
                                                 "monomial is 3, but the monomial has a size of only 2"))
        );
    }
};

TEST_CASE("rtkm_t_order_test")
{
    tuple_for_each(int_types{}, t_order_tester{});
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
    void operator()(const T &) const
    {
        using key_type = real_trigonometric_kronecker_monomial<T>;
        using ka = kronecker_array<T>;
        // Check the type trait.
        CHECK((key_is_multipliable<int, key_type>::value));
        CHECK((key_is_multipliable<rational, key_type>::value));
        CHECK((!key_is_multipliable<mock_cf3, key_type>::value));
        // Test handling of coefficients.
        using term_type = term<rational, key_type>;
        term_type t1, t2;
        t1.m_cf = 2 / 3_q;
        t1.m_key = key_type{T(2)};
        t2.m_cf = 3 / 5_q;
        t2.m_key = key_type{T(3)};
        std::array<term_type, 2u> retval;
        key_type::multiply(retval, t1, t2, symbol_fset{"x"});
        CHECK(retval[0u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[1u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[0u].m_key.get_int() == T(5));
        CHECK(retval[1u].m_key.get_int() == T(1));
        CHECK(retval[0u].m_key.get_flavour());
        CHECK(retval[1u].m_key.get_flavour());
        t1.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"x"});
        CHECK(retval[0u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[1u].m_cf == -(t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[0u].m_key.get_int() == T(5));
        CHECK(retval[1u].m_key.get_int() == T(1));
        CHECK(!retval[0u].m_key.get_flavour());
        CHECK(!retval[1u].m_key.get_flavour());
        t2.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"x"});
        CHECK(retval[0u].m_cf == -(t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[1u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[0u].m_key.get_int() == T(5));
        CHECK(retval[1u].m_key.get_int() == T(1));
        CHECK(retval[0u].m_key.get_flavour());
        CHECK(retval[1u].m_key.get_flavour());
        t1.m_key.set_flavour(true);
        key_type::multiply(retval, t1, t2, symbol_fset{"x"});
        CHECK(retval[0u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[1u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[0u].m_key.get_int() == T(5));
        CHECK(retval[1u].m_key.get_int() == T(1));
        CHECK(!retval[0u].m_key.get_flavour());
        CHECK(!retval[1u].m_key.get_flavour());
        // Test change in sign for sine result and first multiplier negative.
        t1.m_key = key_type{T(1)};
        t2.m_key = key_type{T(-2)};
        t1.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"x"});
        CHECK(retval[0u].m_cf == -(t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[1u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[0u].m_key.get_int() == T(1));
        CHECK(retval[1u].m_key.get_int() == T(3));
        CHECK(!retval[0u].m_key.get_flavour());
        CHECK(!retval[1u].m_key.get_flavour());
        t1.m_key = key_type{T(1)};
        t2.m_key = key_type{T(2)};
        t1.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"x"});
        CHECK(retval[0u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[1u].m_cf == -(t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[0u].m_key.get_int() == T(3));
        CHECK(retval[1u].m_key.get_int() == T(1));
        CHECK(!retval[0u].m_key.get_flavour());
        CHECK(!retval[1u].m_key.get_flavour());
        t1.m_key = key_type{T(1)};
        t2.m_key = key_type{T(-2)};
        t2.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"x"});
        CHECK(retval[0u].m_cf == -(t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[1u].m_cf == -(t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[0u].m_key.get_int() == T(1));
        CHECK(retval[1u].m_key.get_int() == T(3));
        CHECK(!retval[0u].m_key.get_flavour());
        CHECK(!retval[1u].m_key.get_flavour());
        t1.m_key = key_type{T(1)};
        t2.m_key = key_type{T(2)};
        t2.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"x"});
        CHECK(retval[0u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[1u].m_cf == (t1.m_cf.get_num() * t2.m_cf.get_num()));
        CHECK(retval[0u].m_key.get_int() == T(3));
        CHECK(retval[1u].m_key.get_int() == T(1));
        CHECK(!retval[0u].m_key.get_flavour());
        CHECK(!retval[1u].m_key.get_flavour());
        // Test handling of keys.
        t1 = term_type{};
        t2 = term_type{};
        key_type::multiply(retval, t1, t2, symbol_fset{});
        CHECK(retval[0u].m_key.get_int() == 0);
        CHECK(retval[1u].m_key.get_int() == 0);
        CHECK(retval[0u].m_key.get_flavour() == true);
        CHECK(retval[1u].m_key.get_flavour() == true);
        t1 = term_type{1, key_type({0})};
        t2 = term_type{1, key_type({0})};
        key_type::multiply(retval, t1, t2, symbol_fset{"a"});
        CHECK(retval[0u].m_key.get_int() == 0);
        CHECK(retval[1u].m_key.get_int() == 0);
        CHECK(retval[0u].m_key.get_flavour() == true);
        CHECK(retval[1u].m_key.get_flavour() == true);
        t1 = term_type{1, key_type({1})};
        t2 = term_type{1, key_type({2})};
        key_type::multiply(retval, t1, t2, symbol_fset{"a"});
        CHECK(retval[0u].m_key.get_int() == 3);
        CHECK(retval[1u].m_key.get_int() == 1);
        CHECK(retval[0u].m_key.get_flavour() == true);
        CHECK(retval[1u].m_key.get_flavour() == true);
        t1 = term_type{1, key_type({1, -1})};
        t2 = term_type{1, key_type({2, 0})};
        key_type::multiply(retval, t1, t2, symbol_fset{"a", "b"});
        CHECK(retval[0u].m_key.get_flavour() == true);
        CHECK(retval[1u].m_key.get_flavour() == true);
        std::vector<int> tmp(2u);
        ka::decode(tmp, retval[0u].m_key.get_int());
        CHECK(tmp[0u] == 3);
        CHECK(tmp[1u] == -1);
        ka::decode(tmp, retval[1u].m_key.get_int());
        CHECK(tmp[0u] == 1);
        CHECK(tmp[1u] == 1);
        t1.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"a", "b"});
        CHECK(retval[0u].m_key.get_flavour() == false);
        CHECK(retval[1u].m_key.get_flavour() == false);
        ka::decode(tmp, retval[0u].m_key.get_int());
        CHECK(tmp[0u] == 3);
        CHECK(tmp[1u] == -1);
        ka::decode(tmp, retval[1u].m_key.get_int());
        CHECK(tmp[0u] == 1);
        CHECK(tmp[1u] == 1);
        t1.m_key.set_flavour(true);
        t2.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"a", "b"});
        CHECK(retval[0u].m_key.get_flavour() == false);
        CHECK(retval[1u].m_key.get_flavour() == false);
        ka::decode(tmp, retval[0u].m_key.get_int());
        CHECK(tmp[0u] == 3);
        CHECK(tmp[1u] == -1);
        ka::decode(tmp, retval[1u].m_key.get_int());
        CHECK(tmp[0u] == 1);
        CHECK(tmp[1u] == 1);
        t1.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"a", "b"});
        CHECK(retval[0u].m_key.get_flavour() == true);
        CHECK(retval[1u].m_key.get_flavour() == true);
        ka::decode(tmp, retval[0u].m_key.get_int());
        CHECK(tmp[0u] == 3);
        CHECK(tmp[1u] == -1);
        ka::decode(tmp, retval[1u].m_key.get_int());
        CHECK(tmp[0u] == 1);
        CHECK(tmp[1u] == 1);
        t1 = term_type{1, key_type({1, -1})};
        t2 = term_type{1, key_type({-2, -2})};
        key_type::multiply(retval, t1, t2, symbol_fset{"a", "b"});
        CHECK(retval[0u].m_key.get_flavour() == true);
        CHECK(retval[1u].m_key.get_flavour() == true);
        ka::decode(tmp, retval[0u].m_key.get_int());
        CHECK(tmp[0u] == 1);
        CHECK(tmp[1u] == 3);
        ka::decode(tmp, retval[1u].m_key.get_int());
        CHECK(tmp[0u] == 3);
        CHECK(tmp[1u] == 1);
        t1.m_key.set_flavour(false);
        key_type::multiply(retval, t1, t2, symbol_fset{"a", "b"});
        CHECK(retval[0u].m_key.get_flavour() == false);
        CHECK(retval[1u].m_key.get_flavour() == false);
        ka::decode(tmp, retval[0u].m_key.get_int());
        CHECK(tmp[0u] == 1);
        CHECK(tmp[1u] == 3);
        ka::decode(tmp, retval[1u].m_key.get_int());
        CHECK(tmp[0u] == 3);
        CHECK(tmp[1u] == 1);
        // Multiplication that produces first multiplier zero, second negative, in the plus.
        t1 = term_type{1, key_type({1, -1})};
        t2 = term_type{1, key_type({-1, -2})};
        key_type::multiply(retval, t1, t2, symbol_fset{"a", "b"});
        CHECK(retval[0u].m_key.get_flavour() == true);
        CHECK(retval[1u].m_key.get_flavour() == true);
        ka::decode(tmp, retval[0u].m_key.get_int());
        CHECK(tmp[0u] == 0);
        CHECK(tmp[1u] == 3);
        ka::decode(tmp, retval[1u].m_key.get_int());
        CHECK(tmp[0u] == 2);
        CHECK(tmp[1u] == 1);
        // Multiplication that produces first multiplier zero, second negative, in the minus.
        t1 = term_type{1, key_type({1, -2})};
        t2 = term_type{1, key_type({1, -1})};
        key_type::multiply(retval, t1, t2, symbol_fset{"a", "b"});
        CHECK(retval[0u].m_key.get_flavour() == true);
        CHECK(retval[1u].m_key.get_flavour() == true);
        ka::decode(tmp, retval[0u].m_key.get_int());
        CHECK(tmp[0u] == 2);
        CHECK(tmp[1u] == -3);
        ka::decode(tmp, retval[1u].m_key.get_int());
        CHECK(tmp[0u] == 0);
        CHECK(tmp[1u] == 1);
    }
};

TEST_CASE("rtkm_multiply_test")
{
    tuple_for_each(int_types{}, multiply_tester{});
}

struct equality_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
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
        k1 = k_type{1, 2};
        k2 = k_type{1, 2};
        k2.set_flavour(false);
        CHECK(k1 != k2);
        CHECK(!(k1 == k2));
        k1.set_flavour(false);
        CHECK(k1 == k2);
        CHECK(!(k1 != k2));
    }
};

TEST_CASE("rtkm_equality_test")
{
    tuple_for_each(int_types{}, equality_tester{});
}

struct hash_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
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

TEST_CASE("rtkm_hash_test")
{
    tuple_for_each(int_types{}, hash_tester{});
}

struct unpack_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
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
            vs1.emplace_hint(vs1.end(), tmp);
        }
        CHECK_THROWS_MATCHES(k1.unpack(vs1), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("the size of the input arguments set (" + std::to_string(vs1.size())
                                                 + ") is larger than the maximum allowed size (" ))
        );
    }
};

TEST_CASE("rtkm_unpack_test")
{
    tuple_for_each(int_types{}, unpack_tester{});
}

struct print_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        k_type k1;
        std::ostringstream oss;
        k1.print(oss, symbol_fset{});
        CHECK(oss.str().empty());
        k_type k2(symbol_fset{"x"});
        k2.print(oss, symbol_fset{"x"});
        CHECK(oss.str().empty());
        k_type k3({T(1)});
        k3.print(oss, symbol_fset{"x"});
        CHECK(oss.str() == "cos(x)");
        k3.set_flavour(false);
        oss.str("");
        k3.print(oss, symbol_fset{"x"});
        CHECK(oss.str() == "sin(x)");
        k_type k5({T(1), T(-1)});
        oss.str("");
        k5.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "cos(x-y)");
        oss.str("");
        k5 = k_type{T(1), T(1)};
        k5.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "cos(x+y)");
        oss.str("");
        k5 = k_type{T(1), T(2)};
        k5.set_flavour(false);
        k5.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "sin(x+2*y)");
        oss.str("");
        k5 = k_type{T(1), T(-2)};
        k5.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "cos(x-2*y)");
        oss.str("");
        k5 = k_type{T(-1), T(-2)};
        k5.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "cos(-x-2*y)");
        oss.str("");
        k5 = k_type{T(-2), T(1)};
        k5.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "cos(-2*x+y)");
        oss.str("");
        // Representation bug: would display cos(+y).
        k5 = k_type{T(0), T(1)};
        k5.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "cos(y)");
        oss.str("");
        k5 = k_type{T(0), T(-1)};
        k5.print(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "cos(-y)");
    }
};

TEST_CASE("rtkm_print_test")
{
    tuple_for_each(int_types{}, print_tester{});
}

struct partial_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        CHECK(key_is_differentiable<k_type>::value);
        typedef kronecker_array<T> ka;
        const auto &limits = ka::get_limits();
        k_type k1{T(1)};
        // Empty symbol must be associated to zero internal value.
        CHECK_THROWS_AS(k1.partial(0, symbol_fset{}), std::invalid_argument);
        // Check a decode outside the bounds.
        if (std::get<0u>(limits[1u])[0u] < std::numeric_limits<T>::max()) {
            k1.set_int(std::numeric_limits<T>::max());
            CHECK_THROWS_AS(k1.partial(0, symbol_fset{"x"}), std::invalid_argument);
        }
        k1 = k_type{T(1), T(2)};
        auto ret = k1.partial(0, symbol_fset{"x", "y"});
        CHECK(ret.first == -1);
        CHECK(ret.second.get_flavour() == false);
        CHECK(ret.second.get_int() == k1.get_int());
        k1.set_flavour(false);
        ret = k1.partial(1, symbol_fset{"x", "y"});
        CHECK(ret.first == 2);
        CHECK(ret.second.get_flavour() == true);
        CHECK(ret.second.get_int() == k1.get_int());
        k1 = k_type{T(0), T(2)};
        ret = k1.partial(0, symbol_fset{"x", "y"});
        CHECK(ret.first == 0);
        CHECK(ret.second.get_flavour() == true);
        CHECK(ret.second.get_int() == 0);
        k1 = k_type{T(1), T(2)};
        ret = k1.partial(2, symbol_fset{"x", "y"});
        CHECK(ret.first == 0);
        CHECK(ret.second.get_flavour() == true);
        CHECK(ret.second.get_int() == 0);
        k1 = k_type{T(1), T(2)};
        ret = k1.partial(1, symbol_fset{"x", "y"});
        CHECK(ret.first == -2);
        CHECK(ret.second.get_flavour() == false);
        CHECK(ret.second.get_int() == k1.get_int());
    }
};

TEST_CASE("rtkm_partial_test")
{
    tuple_for_each(int_types{}, partial_tester{});
}

struct evaluate_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = real_trigonometric_kronecker_monomial<T>;
        k_type k1;
        CHECK(k1.template evaluate<integer>({}, symbol_fset{}) == integer(1));
        k1.set_flavour(false);
        CHECK(k1.template evaluate<integer>({}, symbol_fset{}) == integer(0));
        k1.set_flavour(true);
        CHECK_THROWS_MATCHES(k1.template evaluate<integer>({}, symbol_fset{"x"}), std::invalid_argument,
              test::ExceptionMatcher<std::invalid_argument>(std::string("invalid vector of values for real trigonometric Kronecker monomial "
                                                "evaluation: the size of the vector of values (0) differs from the "
                                                "size of the reference set of symbols (1)"))
        );
        k1 = k_type({T(1)});
        CHECK_THROWS_MATCHES(k1.template evaluate<integer>({}, symbol_fset{"x"}), std::invalid_argument,
              test::ExceptionMatcher<std::invalid_argument>(std::string("invalid vector of values for real trigonometric Kronecker monomial "
                                                "evaluation: the size of the vector of values (0) differs from the "
                                                "size of the reference set of symbols (1)"))
        );
        CHECK(k1.template evaluate<integer>({0_z}, symbol_fset{"x"}) == 1);
        CHECK((std::is_same<integer, decltype(k1.template evaluate<integer>({0_z}, symbol_fset{"x"}))>::value));
        // NOTE: here the return type depends on the integral type considered, char * char for instance gives int as
        // result according to the standard integral promotions.
        CHECK((std::is_same<T, decltype(k1.template evaluate<int>({0}, symbol_fset{"x"}))>::value
                     || std::is_same<int, decltype(k1.template evaluate<int>({0}, symbol_fset{"x"}))>::value));
#if defined(MPPP_WITH_MPFR)
        CHECK(
            (std::is_same<real, decltype(k1.template evaluate<real>(std::vector<real>{}, symbol_fset{}))>::value));
        CHECK((
            std::is_same<double, decltype(k1.template evaluate<double>(std::vector<double>{}, symbol_fset{}))>::value));
        k1.set_flavour(false);
        CHECK(k1.template evaluate<integer>({0_z}, symbol_fset{"x"}) == 0);
        k1 = k_type({T(2), T(-3)});
        CHECK(k1.template evaluate<real>({real(3.2), real(-4.3)}, symbol_fset{"x", "y"}) ==    //TODO:: fails due to precission issue
                          piranha::cos((0. + (real(3.2) * 2)) + (real(-4.3) * -3)));
        k1.set_flavour(false);
        CHECK(k1.template evaluate<real>({real(3.2), real(-4.3)}, symbol_fset{"x", "y"}) ==    //TODO:: fails due to precission issue
                          piranha::sin((0. + (real(3.2) * 2)) + (real(-4.3) * -3)));
        k1 = k_type({T(-2), T(-3)});
        CHECK(k1.template evaluate<real>({real(3.2), real(-4.3)}, symbol_fset{"x", "y"}) ==    //TODO:: fails due to precission issue
                          piranha::cos((0. + (real(3.2) * -2)) + (real(-4.3) * -3)));
        k1.set_flavour(false);
        CHECK(k1.template evaluate<real>({real(3.2), real(-4.3)}, symbol_fset{"x", "y"}) ==    //TODO:: fails due to precission issue
                          piranha::sin((0. + (real(3.2) * -2)) + (real(-4.3) * -3)));
        k1 = k_type({T(3), T(-2)});
        CHECK(k1.template evaluate<rational>({2_q / 3, 1_q}, symbol_fset{"x", "y"}) == 1);
        k1.set_flavour(false);
        CHECK(k1.template evaluate<rational>({2_q / 3, 1_q}, symbol_fset{"x", "y"}) == 0);
#endif
    }
};

TEST_CASE("rtkm_evaluate_test")
{
    tuple_for_each(int_types{}, evaluate_tester{});
    CHECK((!key_is_evaluable<real_trigonometric_kronecker_monomial<>, std::vector<int>>::value));
    CHECK((!key_is_evaluable<real_trigonometric_kronecker_monomial<>, char *>::value));
    CHECK((!key_is_evaluable<real_trigonometric_kronecker_monomial<>, std::string>::value));
    CHECK((!key_is_evaluable<real_trigonometric_kronecker_monomial<>, void *>::value));
    CHECK((!key_is_evaluable<real_trigonometric_kronecker_monomial<>, void>::value));
}

struct subs_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        // Test the type trait.
#if defined(MPPP_WITH_MPFR)
        CHECK((key_has_subs<k_type, real>::value));
#endif
        CHECK((key_has_subs<k_type, double>::value));
        CHECK((!key_has_subs<k_type, std::string>::value));
        CHECK((!key_has_subs<k_type, std::vector<std::string>>::value));
        CHECK((!key_has_subs<k_type, void>::value));
        k_type k1;
        auto ret = k1.template subs<integer>({}, symbol_fset{});
        CHECK(ret.size() == 2u);
        CHECK(ret[0u].first == 1);
        CHECK(ret[0u].second == k1);
        CHECK(ret[1u].first == 0);
        CHECK((ret[1u].second == k_type(T(0), false)));
        k1.set_flavour(false);
        ret = k1.template subs<integer>({}, symbol_fset{});
        CHECK(ret.size() == 2u);
        CHECK(ret[0u].first == 0);
        CHECK((ret[0u].second == k_type(T(0), true)));
        CHECK(ret[1u].first == 1);
        CHECK((ret[1u].second == k1));
        k1 = k_type{T(1)};
        CHECK_THROWS_AS(k1.template subs<integer>({{0, 5_z}}, symbol_fset{"x"}), std::domain_error);
        k1 = k_type(T(1), false);
        CHECK_THROWS_AS(k1.template subs<integer>({{0, 5_z}}, symbol_fset{"x"}), std::domain_error);
        // Subs with no sign changes.
        k1 = k_type({T(2), T(3)});
#if defined(MPPP_WITH_MPFR)
        auto ret2 = k1.template subs<real>({{0, real(5)}}, symbol_fset{"x", "y"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::cos(real(5) * T(2)));
        CHECK(ret2[1u].first == -piranha::sin(real(5) * T(2)));
        CHECK((ret2[0u].second == k_type({T(0), T(3)})));
        k_type tmp({T(0), T(3)});
        tmp.set_flavour(false);
        CHECK((ret2[1u].second == tmp));
        k1.set_flavour(false);
        ret2 = k1.template subs<real>({{0, real(5)}}, symbol_fset{"x", "y"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::sin(real(5) * T(2)));
        CHECK(ret2[1u].first == piranha::cos(real(5) * T(2)));
        CHECK((ret2[0u].second == k_type({T(0), T(3)})));
        CHECK((ret2[1u].second == tmp));
        // Subs with no actual sub.
        k1.set_flavour(true);
        ret2 = k1.template subs<real>({}, symbol_fset{"x", "y"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == real(1));
        CHECK(ret2[1u].first == real(0));
        CHECK((ret2[0u].second == k1));
        k1.set_flavour(false);
        CHECK((ret2[1u].second == k1));
        ret2 = k1.template subs<real>({}, symbol_fset{"x", "y"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == real(0));
        CHECK(ret2[1u].first == real(1));
        k1.set_flavour(true);
        CHECK((ret2[0u].second == k1));
        k1.set_flavour(false);
        CHECK((ret2[1u].second == k1));
        // Subs with sign change.
        k1 = k_type({T(2), T(-3)});
        ret2 = k1.template subs<real>({{0, real(6)}}, symbol_fset{"x", "y"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::cos(real(6) * T(2)));
        CHECK(ret2[1u].first == piranha::sin(real(6) * T(2)));
        CHECK((ret2[0u].second == k_type({T(0), T(3)})));
        CHECK((ret2[1u].second == tmp));
        k1.set_flavour(false);
        ret2 = k1.template subs<real>({{0, real(6)}}, symbol_fset{"x", "y"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::sin(real(6) * T(2)));
        CHECK(ret2[1u].first == -piranha::cos(real(6) * T(2)));
        CHECK((ret2[0u].second == k_type({T(0), T(3)})));
        CHECK((ret2[1u].second == tmp));
        if (std::is_same<signed char, T>::value) {
            return;
        }
        // Another with sign change.
        k1 = k_type({T(2), T(-2), T(1)});
        ret2 = k1.template subs<real>({{0, real(7)}}, symbol_fset{"x", "y", "z"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::cos(real(7) * T(2)));
        CHECK(ret2[1u].first == piranha::sin(real(7) * T(2)));
        tmp = k_type({T(0), T(2), T(-1)});
        CHECK((ret2[0u].second == tmp));
        tmp.set_flavour(false);
        CHECK((ret2[1u].second == tmp));
        k1.set_flavour(false);
        ret2 = k1.template subs<real>({{0, real(7)}}, symbol_fset{"x", "y", "z"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::sin(real(7) * T(2)));
        CHECK(ret2[1u].first == -piranha::cos(real(7) * T(2)));
        CHECK((ret2[1u].second == tmp));
        tmp.set_flavour(true);
        CHECK((ret2[0u].second == tmp));
        // Sign change with leading zero multiplier after substitution.
        k1 = k_type({T(2), T(0), T(-1)});
        ret2 = k1.template subs<real>({{0, real(7)}}, symbol_fset{"x", "y", "z"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::cos(real(7) * T(2)));
        CHECK(ret2[1u].first == piranha::sin(real(7) * T(2)));
        tmp = k_type({T(0), T(0), T(1)});
        CHECK((ret2[0u].second == tmp));
        tmp.set_flavour(false);
        CHECK((ret2[1u].second == tmp));
        k1.set_flavour(false);
        ret2 = k1.template subs<real>({{0, real(7)}}, symbol_fset{"x", "y", "z"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::sin(real(7) * T(2)));
        CHECK(ret2[1u].first == -piranha::cos(real(7) * T(2)));
        CHECK((ret2[1u].second == tmp));
        tmp.set_flavour(true);
        CHECK((ret2[0u].second == tmp));
        // Leading zero and subsequent canonicalisation.
        k1 = k_type({T(0), T(-1), T(1)});
        ret2 = k1.template subs<real>({{0, real(7)}}, symbol_fset{"x", "y", "z"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::cos(real(7) * T(0)));
        CHECK(ret2[1u].first == piranha::sin(real(7) * T(0)));
        tmp = k_type({T(0), T(1), T(-1)});
        CHECK((ret2[0u].second == tmp));
        tmp.set_flavour(false);
        CHECK((ret2[1u].second == tmp));
        k1.set_flavour(false);
        ret2 = k1.template subs<real>({{0, real(7)}}, symbol_fset{"x", "y", "z"});
        CHECK(ret2.size() == 2u);
        CHECK(ret2[0u].first == piranha::sin(real(7) * T(0)));
        CHECK(ret2[1u].first == -piranha::cos(real(7) * T(0)));
        CHECK((ret2[1u].second == tmp));
        tmp.set_flavour(true);
        CHECK((ret2[0u].second == tmp));
        // Failure modes.
        CHECK_THROWS_MATCHES(k1.template subs<real>({{0, real(7)}, {6, real(1)}}, symbol_fset{"x", "y", "z"}),
                              std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid argument(s) for substitution in a real trigonometric "
                                                         "Kronecker monomial: the last index of the substitution map "
                                                         "(6) must be smaller than the monomial's size (3)"))
        );
#endif
    }
};

TEST_CASE("rtkm_subs_test")
{
    tuple_for_each(int_types{}, subs_tester{});
}

struct print_tex_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        k_type k1;
        std::ostringstream oss;
        k1.print_tex(oss, symbol_fset{});
        CHECK(oss.str().empty());
        k_type k2(symbol_fset{"x"});
        k2.print_tex(oss, symbol_fset{"x"});
        CHECK(oss.str().empty());
        k_type k3({T(1)});
        k3.print_tex(oss, symbol_fset{"x"});
        CHECK(oss.str() == "\\cos{\\left({x}\\right)}");
        k3.set_flavour(false);
        oss.str("");
        k3.print_tex(oss, symbol_fset{"x"});
        CHECK(oss.str() == "\\sin{\\left({x}\\right)}");
        k_type k5({T(1), T(-1)});
        oss.str("");
        k5.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\cos{\\left({x}-{y}\\right)}");
        oss.str("");
        k5 = k_type{T(1), T(1)};
        k5.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\cos{\\left({x}+{y}\\right)}");
        oss.str("");
        k5 = k_type{T(1), T(2)};
        k5.set_flavour(false);
        k5.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\sin{\\left({x}+2{y}\\right)}");
        oss.str("");
        k5 = k_type{T(1), T(-2)};
        k5.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\cos{\\left({x}-2{y}\\right)}");
        oss.str("");
        k5 = k_type{T(-1), T(-2)};
        k5.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\cos{\\left(-{x}-2{y}\\right)}");
        oss.str("");
        k5 = k_type{T(-2), T(1)};
        k5.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\cos{\\left(-2{x}+{y}\\right)}");
        // Representation bug: would display cos(+y).
        oss.str("");
        k5 = k_type{T(0), T(1)};
        k5.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\cos{\\left({y}\\right)}");
        oss.str("");
        k5 = k_type{T(0), T(-1)};
        k5.print_tex(oss, symbol_fset{"x", "y"});
        CHECK(oss.str() == "\\cos{\\left(-{y}\\right)}");
    }
};

TEST_CASE("rtkm_print_tex_test")
{
    tuple_for_each(int_types{}, print_tex_tester{});
}
struct integrate_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        CHECK(key_is_integrable<k_type>::value);
        typedef kronecker_array<T> ka;
        const auto &limits = ka::get_limits();
        k_type k1{T(1)};
        CHECK_THROWS_AS(k1.integrate("x", symbol_fset{}), std::invalid_argument);
        if (std::get<0u>(limits[1u])[0u] < std::numeric_limits<T>::max()) {
            k1.set_int(std::numeric_limits<T>::max());
            CHECK_THROWS_AS(k1.integrate("x", symbol_fset{}), std::invalid_argument);
        }
        k1 = k_type{T(1), T(2)};
        auto ret = k1.integrate("x", symbol_fset{"x", "y"});
        CHECK(ret.first == 1);
        CHECK(ret.second.get_flavour() == false);
        CHECK(ret.second.get_int() == k1.get_int());
        k1.set_flavour(false);
        ret = k1.integrate("y", symbol_fset{"x", "y"});
        CHECK(ret.first == -2);
        CHECK(ret.second.get_flavour() == true);
        CHECK(ret.second.get_int() == k1.get_int());
        k1 = k_type{T(0), T(2)};
        ret = k1.integrate("x", symbol_fset{"x", "y"});
        CHECK(ret.first == 0);
        CHECK(ret.second.get_flavour() == true);
        CHECK(ret.second.get_int() == 0);
        k1 = k_type{T(1), T(2)};
        ret = k1.integrate("z", symbol_fset{"x", "y"});
        CHECK(ret.first == 0);
        CHECK(ret.second.get_flavour() == true);
        CHECK(ret.second.get_int() == 0);
        k1 = k_type{T(1), T(2)};
        ret = k1.integrate("y", symbol_fset{"x", "y"});
        CHECK(ret.first == 2);
        CHECK(ret.second.get_flavour() == false);
        CHECK(ret.second.get_int() == k1.get_int());
    }
};

TEST_CASE("rtkm_integrate_test")
{
    tuple_for_each(int_types{}, integrate_tester{});
}

struct canonicalise_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        k_type k1;
        CHECK(!k1.canonicalise(symbol_fset{}));
        k1 = k_type{T(1)};
        CHECK_THROWS_AS(k1.canonicalise(symbol_fset{}), std::invalid_argument);
        k1 = k_type{T(0)};
        CHECK(!k1.canonicalise(symbol_fset{"x"}));
        k1 = k_type{T(1)};
        CHECK(!k1.canonicalise(symbol_fset{"x"}));
        k1 = k_type{T(-1)};
        CHECK(k1.canonicalise(symbol_fset{"x"}));
        CHECK(k1 == k_type{T(1)});
        k1 = k_type{T(0), T(0)};
        CHECK(!k1.canonicalise(symbol_fset{"x", "y"}));
        CHECK((k1 == k_type{T(0), T(0)}));
        k1 = k_type{T(1), T(0)};
        CHECK(!k1.canonicalise(symbol_fset{"x", "y"}));
        CHECK((k1 == k_type{T(1), T(0)}));
        k1 = k_type{T(-1), T(0)};
        CHECK(k1.canonicalise(symbol_fset{"x", "y"}));
        CHECK((k1 == k_type{T(1), T(0)}));
        k1 = k_type{T(1), T(-1)};
        CHECK(!k1.canonicalise(symbol_fset{"x", "y"}));
        CHECK((k1 == k_type{T(1), T(-1)}));
        k1 = k_type{T(0), T(-1)};
        CHECK(k1.canonicalise(symbol_fset{"x", "y"}));
        CHECK((k1 == k_type{T(0), T(1)}));
        k1 = k_type{T(0), T(1)};
        CHECK(!k1.canonicalise(symbol_fset{"x", "y"}));
        CHECK((k1 == k_type{T(0), T(1)}));
        k1 = k_type{T(0), T(1), T(-1)};
        CHECK(!k1.canonicalise(symbol_fset{"x", "y", "z"}));
        CHECK((k1 == k_type{T(0), T(1), T(-1)}));
        k1 = k_type{T(0), T(-1), T(-1)};
        CHECK(k1.canonicalise(symbol_fset{"x", "y", "z"}));
        CHECK((k1 == k_type{T(0), T(1), T(1)}));
        k1 = k_type{T(0), T(0), T(-1)};
        CHECK(k1.canonicalise(symbol_fset{"x", "y", "z"}));
        CHECK((k1 == k_type{T(0), T(0), T(1)}));
        k1 = k_type{T(1), T(-1), T(-1)};
        CHECK(!k1.canonicalise(symbol_fset{"x", "y", "z"}));
        CHECK((k1 == k_type{T(1), T(-1), T(-1)}));
    }
};

TEST_CASE("rtkm_canonicalise_test")
{
    tuple_for_each(int_types{}, canonicalise_tester{});
}

struct trim_identify_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = real_trigonometric_kronecker_monomial<T>;
        k_type k0;
        std::vector<char> mask;
        CHECK_NOTHROW(k0.trim_identify(mask, symbol_fset{}));
        CHECK(mask.size() == 0u);
        k0.set_int(1);
        CHECK_THROWS_MATCHES(
            k0.trim_identify(mask, symbol_fset{"x"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid mask for trim_identify(): the size of the mask (0) differs "
                                                 "from the size of the reference symbol set (1)"))
        );
        mask = {1};
        CHECK_THROWS_MATCHES(
            k0.trim_identify(mask, symbol_fset{}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid mask for trim_identify(): the size of the mask (1) differs "
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

TEST_CASE("rtkm_trim_identify_test")
{
    tuple_for_each(int_types{}, trim_identify_tester{});
}

struct trim_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = real_trigonometric_kronecker_monomial<T>;
        k_type k0;
        CHECK(k0.trim({}, symbol_fset{}) == k0);
        k0.set_flavour(false);
        CHECK(k0.trim({}, symbol_fset{}) == k_type(0, false));
        k0.set_int(1);
        CHECK_THROWS_MATCHES(k0.trim({}, symbol_fset{"x"}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid mask for trim(): the size of the mask (0) differs from the size "
                                             "of the reference symbol set (1)"))
        );
        CHECK_THROWS_MATCHES(k0.trim({1}, symbol_fset{}), std::invalid_argument,
            test::ExceptionMatcher<std::invalid_argument>(std::string("invalid mask for trim(): the size of the mask (1) differs from the size "
                                             "of the reference symbol set (0)"))
        );
        k0 = k_type{T(1), T(0), T(-1)};
        CHECK((k0.trim({0, 1, 0}, symbol_fset{"x", "y", "z"}) == k_type{1, -1}));
        k0.set_flavour(false);
        CHECK((k0.trim({0, 1, 0}, symbol_fset{"x", "y", "z"}) == k_type({1, -1}, false)));
        k0.set_flavour(true);
        CHECK((k0.trim({1, 0, 0}, symbol_fset{"x", "y", "z"}) == k_type{0, -1}));
        k0.set_flavour(false);
        CHECK((k0.trim({1, 0, 0}, symbol_fset{"x", "y", "z"}) == k_type{{0, -1}, false}));
        k0.set_flavour(true);
        CHECK((k0.trim({0, 0, 0}, symbol_fset{"x", "y", "z"}) == k0));
        k0.set_flavour(false);
        CHECK((k0.trim({0, 0, 0}, symbol_fset{"x", "y", "z"}) == k_type{{T(1), T(0), T(-1)}, false}));
        k0.set_flavour(true);
        CHECK((k0.trim({1, 0, 1}, symbol_fset{"x", "y", "z"}) == k_type{0}));
        k0.set_flavour(false);
        CHECK((k0.trim({1, 0, 1}, symbol_fset{"x", "y", "z"}) == k_type{std::initializer_list<T>{0}, false}));
        k0.set_flavour(true);
        CHECK((k0.trim({1, 1, 0}, symbol_fset{"x", "y", "z"}) == k_type{-1}));
        k0.set_flavour(false);
        CHECK((k0.trim({1, 1, 0}, symbol_fset{"x", "y", "z"}) == k_type{std::initializer_list<T>{-1}, false}));
        k0.set_flavour(true);
        CHECK((k0.trim({0, 1, 1}, symbol_fset{"x", "y", "z"}) == k_type{1}));
        k0.set_flavour(false);
        CHECK((k0.trim({0, 1, 1}, symbol_fset{"x", "y", "z"}) == k_type{std::initializer_list<T>{1}, false}));
        k0.set_flavour(true);
        CHECK((k0.trim({1, 1, 1}, symbol_fset{"x", "y", "z"}) == k_type{}));
        k0.set_flavour(false);
        CHECK((k0.trim({1, 1, 1}, symbol_fset{"x", "y", "z"}) == k_type{std::initializer_list<T>{}, false}));
    }
};

TEST_CASE("rtkm_trim_test")
{
    tuple_for_each(int_types{}, trim_tester{});
}

struct tt_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        CHECK(key_has_t_degree<k_type>::value);
        CHECK(key_has_t_ldegree<k_type>::value);
        CHECK(key_has_t_order<k_type>::value);
        CHECK(key_has_t_lorder<k_type>::value);
        CHECK(is_hashable<k_type>::value);
    }
};

TEST_CASE("rtkm_tt_test")
{
    tuple_for_each(int_types{}, tt_tester{});
}

struct fake_int_01 {
    fake_int_01();
    explicit fake_int_01(int);
    fake_int_01(const fake_int_01 &);
    fake_int_01(fake_int_01 &&) noexcept;
    fake_int_01 &operator=(const fake_int_01 &);
    fake_int_01 &operator=(fake_int_01 &&) noexcept;
    ~fake_int_01();
    fake_int_01 operator+(const fake_int_01 &) const;
    fake_int_01 operator*(const fake_int_01 &)const;
    fake_int_01 &operator+=(const fake_int_01 &);
    fake_int_01 &operator+=(const integer &);
    bool operator==(const fake_int_01 &) const;
    bool operator!=(const fake_int_01 &) const;
};

integer &operator+=(integer &, const fake_int_01 &);
integer operator*(const integer &, const fake_int_01 &);

// Missing math operators.
struct fake_int_02 {
    fake_int_02();
    explicit fake_int_02(int);
    fake_int_02(const fake_int_02 &);
    fake_int_02(fake_int_02 &&) noexcept;
    fake_int_02 &operator=(const fake_int_02 &);
    fake_int_02 &operator=(fake_int_02 &&) noexcept;
    ~fake_int_02();
    fake_int_02 operator+(const fake_int_02 &) const;
    fake_int_02 operator*(const fake_int_02 &)const;
    fake_int_02 &operator+=(const fake_int_02 &);
    fake_int_02 &operator+=(const integer &);
    bool operator==(const fake_int_02 &) const;
    bool operator!=(const fake_int_02 &) const;
};

TEST_CASE("rtkm_key_has_t_subs_test")
{
    CHECK((key_has_t_subs<real_trigonometric_kronecker_monomial<int>, int, int>::value));
    CHECK((key_has_t_subs<real_trigonometric_kronecker_monomial<int>, int, int>::value));
    CHECK((key_has_t_subs<real_trigonometric_kronecker_monomial<int>, fake_int_01, fake_int_01>::value));
    CHECK((!key_has_t_subs<real_trigonometric_kronecker_monomial<int>, fake_int_02, fake_int_02>::value));
    // This fails because the cos and sin replacements must be the same type.
    CHECK((!key_has_t_subs<real_trigonometric_kronecker_monomial<short>, int, long>::value));
    CHECK((key_has_t_subs<real_trigonometric_kronecker_monomial<short>, long, long>::value));
    CHECK((key_has_t_subs<real_trigonometric_kronecker_monomial<long> &, long, const long &>::value));
    CHECK((key_has_t_subs<const real_trigonometric_kronecker_monomial<short> &, char, const char &>::value));
    CHECK((!key_has_t_subs<const real_trigonometric_kronecker_monomial<long long> &, char, int>::value));
    CHECK((!key_has_t_subs<const real_trigonometric_kronecker_monomial<short> &, char, void>::value));
    CHECK((!key_has_t_subs<const real_trigonometric_kronecker_monomial<short> &, void, char>::value));
    CHECK((!key_has_t_subs<const real_trigonometric_kronecker_monomial<short> &, void, void>::value));
    CHECK(!is_key_degree_type<real_trigonometric_kronecker_monomial<int>>::value);
    CHECK(!is_key_ldegree_type<real_trigonometric_kronecker_monomial<int>>::value);
    CHECK(key_has_t_degree<real_trigonometric_kronecker_monomial<int>>::value);
    CHECK(key_has_t_ldegree<real_trigonometric_kronecker_monomial<int>>::value);
    CHECK(key_has_t_order<real_trigonometric_kronecker_monomial<int>>::value);
    CHECK(key_has_t_lorder<real_trigonometric_kronecker_monomial<int>>::value);
}

struct t_subs_tester {
    template <typename T, enable_if_t<!std::is_same<T, signed char>::value, int> = 0>
    void operator()(const T &) const
    {
        // Test with no substitution.
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        k_type k;
#if defined(MPPP_WITH_MPFR)
        auto res = k.t_subs(0, real(.5), real(.0), symbol_fset{});
        typedef decltype(res) res_type1;
        CHECK((std::is_same<typename res_type1::value_type::first_type, real>::value));
        CHECK(res.size() == 2u);
        CHECK(res[0u].first == real(1));
        CHECK(res[1u].first == real(0));
        k.set_flavour(false);
        res = k.t_subs(0, real(.5), real(.0), symbol_fset{});
        CHECK(res.size() == 2u);
        CHECK(res[0u].first == real(0));
        CHECK(res[1u].first == real(1));
        k = k_type{T(3)};
        k.set_flavour(true);
        res = k.t_subs(1, real(.5), real(.0), symbol_fset{"x"});
        CHECK(res.size() == 2u);
        CHECK(res[0u].first == real(1));
        CHECK(res[1u].first == real(0));
        CHECK(res[0u].second == k);
        k.set_flavour(false);
        CHECK(res[1u].second == k);
        res = k.t_subs(1, real(.5), real(.0), symbol_fset{"x"});
        CHECK(res.size() == 2u);
        CHECK(res[0u].first == real(0));
        CHECK(res[1u].first == real(1));
        CHECK(res[1u].second == k);
        k.set_flavour(true);
        CHECK(res[0u].second == k);
#endif
        // Test substitution with no canonicalisation.
        k = k_type{T(2), T(3)};
        auto c = rational(1, 2), s = rational(4, 5);
        auto res2 = k.t_subs(1, c, s, symbol_fset{"x", "y"});
        typedef decltype(res2) res_type2;
        CHECK((std::is_same<typename res_type2::value_type::first_type, rational>::value));
        CHECK(res2.size() == 2u);
        CHECK(res2[0u].first == c * c * c - 3 * s * s * c);
        CHECK(res2[1u].first == -3 * c * c * s + s * s * s);
        k = k_type{T(2), T(0)};
        CHECK(res2[0u].second == k);
        k.set_flavour(false);
        CHECK(res2[1u].second == k);
        k = k_type{T(2), T(3)};
        k.set_flavour(false);
        res2 = k.t_subs(1, c, s, symbol_fset{"x", "y"});
        CHECK(res2.size() == 2u);
        CHECK(res2[0u].first == 3 * c * c * s - s * s * s);
        CHECK(res2[1u].first == c * c * c - 3 * s * s * c);
        k = k_type{T(2), T(0)};
        CHECK(res2[0u].second == k);
        k.set_flavour(false);
        CHECK(res2[1u].second == k);
        // Negative multiplier
        k = k_type{T(-3), T(3)};
        res2 = k.t_subs(0, c, s, symbol_fset{"x", "y"});
        CHECK(res2.size() == 2u);
        CHECK(res2[0u].first == c * c * c - 3 * s * s * c);
        CHECK(res2[1u].first == 3 * c * c * s - s * s * s);
        k = k_type{T(0), T(3)};
        CHECK(res2[0u].second == k);
        k.set_flavour(false);
        CHECK(res2[1u].second == k);
        k = k_type{T(-3), T(3)};
        k.set_flavour(false);
        res2 = k.t_subs(0, c, s, symbol_fset{"x", "y"});
        CHECK(res2.size() == 2u);
        CHECK(res2[0u].first == -3 * c * c * s + s * s * s);
        CHECK(res2[1u].first == c * c * c - 3 * s * s * c);
        k = k_type{T(0), T(3)};
        CHECK(res2[0u].second == k);
        k.set_flavour(false);
        CHECK(res2[1u].second == k);
        // Test substitution with canonicalisation.
        k = k_type{T(-2), T(3)};
        res2 = k.t_subs(1, c, s, symbol_fset{"x", "y"});
        CHECK(res2.size() == 2u);
        CHECK(res2[0u].first == c * c * c - 3 * s * s * c);
        CHECK(res2[1u].first == 3 * c * c * s - s * s * s);
        k = k_type{T(2), T(0)};
        CHECK(res2[0u].second == k);
        k.set_flavour(false);
        CHECK(res2[1u].second == k);
        k = k_type{T(-2), T(3)};
        k.set_flavour(false);
        res2 = k.t_subs(1, c, s, symbol_fset{"x", "y"});
        CHECK(res2.size() == 2u);
        CHECK(res2[0u].first == 3 * c * c * s - s * s * s);
        CHECK(res2[1u].first == -c * c * c + 3 * s * s * c);
        k = k_type{T(2), T(0)};
        CHECK(res2[0u].second == k);
        k.set_flavour(false);
        CHECK(res2[1u].second == k);
        // Negative multiplier
        k = k_type{T(-3), T(-3)};
        res2 = k.t_subs(0, c, s, symbol_fset{"x", "y"});
        CHECK(res2.size() == 2u);
        CHECK(res2[0u].first == c * c * c - 3 * s * s * c);
        CHECK(res2[1u].first == -3 * c * c * s + s * s * s);
        k = k_type{T(0), T(3)};
        CHECK(res2[0u].second == k);
        k.set_flavour(false);
        CHECK(res2[1u].second == k);
        k = k_type{T(-3), T(-3)};
        k.set_flavour(false);
        res2 = k.t_subs(0, c, s, symbol_fset{"x", "y"});
        CHECK(res2.size() == 2u);
        CHECK(res2[0u].first == -3 * c * c * s + s * s * s);
        CHECK(res2[1u].first == -c * c * c + 3 * s * s * c);
        k = k_type{T(0), T(3)};
        CHECK(res2[0u].second == k);
        k.set_flavour(false);
        CHECK(res2[1u].second == k);
    }
    template <typename T, enable_if_t<std::is_same<T, signed char>::value, int> = 0>
    void operator()(const T &) const
    {
    }
};

TEST_CASE("rtkm_t_subs_test")
{
    tuple_for_each(int_types{}, t_subs_tester{});
}

struct is_evaluable_tester {
    template <typename T>
    void operator()(const T &) const
    {
        typedef real_trigonometric_kronecker_monomial<T> k_type;
        CHECK((key_is_evaluable<k_type, float>::value));
        CHECK((key_is_evaluable<k_type, double>::value));
#if defined(MPPP_WITH_MPFR)
        CHECK((key_is_evaluable<k_type, real>::value));
#endif
        CHECK((key_is_evaluable<k_type, integer>::value));
        CHECK((key_is_evaluable<k_type, rational>::value));
        CHECK((key_is_evaluable<k_type, int>::value));
        CHECK((key_is_evaluable<k_type, long>::value));
        CHECK((key_is_evaluable<k_type, long long>::value));
        CHECK((!key_is_evaluable<k_type, std::string>::value));
        CHECK((!key_is_evaluable<k_type, void *>::value));
        CHECK((!key_is_evaluable<k_type, void>::value));
    }
};

TEST_CASE("rtkm_key_is_evaluable_test")
{
    tuple_for_each(int_types{}, is_evaluable_tester{});
}

TEST_CASE("rtkm_kic_test")
{
    CHECK((key_is_convertible<rtk_monomial, rtk_monomial>::value));
    CHECK((!key_is_convertible<rtk_monomial, monomial<int>>::value));
    CHECK((!key_is_convertible<monomial<int>, rtk_monomial>::value));
}

struct comparison_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using k_type = real_trigonometric_kronecker_monomial<T>;
        CHECK(is_less_than_comparable<k_type>::value);
        CHECK(!(k_type{} < k_type{}));
        CHECK(!(k_type{0, true} < k_type{0, true}));
        CHECK((k_type{0, false} < k_type{0, true}));
        CHECK(!(k_type{0, true} < k_type{0, false}));
        CHECK((k_type{1, true} < k_type{2, true}));
        CHECK(!(k_type{2, true} < k_type{1, true}));
        CHECK((k_type{1, false} < k_type{2, false}));
        CHECK(!(k_type{2, false} < k_type{1, false}));
        CHECK(!(k_type{2, false} < k_type{1, true}));
    }
};

TEST_CASE("rtkm_comparison_test")
{
    tuple_for_each(int_types{}, comparison_tester{});
}
