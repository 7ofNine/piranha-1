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

#include <piranha/series.hpp>

#include <cstddef>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <mp++/config.hpp>
#include <mp++/exceptions.hpp>
#if defined(MPPP_WITH_MPFR)
#include <mp++/real.hpp>
#endif

#include <piranha/base_series_multiplier.hpp>
#include <piranha/config.hpp>
#include <piranha/detail/debug_access.hpp>
#include <piranha/exceptions.hpp>
#include <piranha/forwarding.hpp>
#include <piranha/integer.hpp>
#include <piranha/key/key_is_one.hpp>
#include <piranha/key_is_multipliable.hpp>
#include <piranha/math.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/monomial.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/s11n.hpp>
#include <piranha/series_multiplier.hpp>
#include <piranha/symbol_utils.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"

static std::mt19937 rng;

using namespace piranha;

using cf_types = std::tuple<double, integer, rational>;
using expo_types = std::tuple<unsigned, integer>;

template <typename Cf, typename Expo>
class g_series_type : public series<Cf, monomial<Expo>, g_series_type<Cf, Expo>>
{
    typedef series<Cf, monomial<Expo>, g_series_type<Cf, Expo>> base;

public:
    template <typename Cf2>
    using rebind = g_series_type<Cf2, Expo>;
    g_series_type() = default;
    g_series_type(const g_series_type &) = default;
    g_series_type(g_series_type &&) = default;
    explicit g_series_type(const char *name) : base()
    {
        typedef typename base::term_type term_type;
        // Insert the symbol.
        this->m_symbol_set = symbol_fset{name};
        // Construct and insert the term.
        this->insert(term_type(Cf(1), typename term_type::key_type{Expo(1)}));
    }
    g_series_type &operator=(const g_series_type &) = default;
    g_series_type &operator=(g_series_type &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type, base)
};

// This is essentially the same as above, just a different type.
template <typename Cf, typename Expo>
class g_series_type2 : public series<Cf, monomial<Expo>, g_series_type2<Cf, Expo>>
{
public:
    typedef series<Cf, monomial<Expo>, g_series_type2<Cf, Expo>> base;
    g_series_type2() = default;
    g_series_type2(const g_series_type2 &) = default;
    g_series_type2(g_series_type2 &&) = default;
    explicit g_series_type2(const char *name) : base()
    {
        typedef typename base::term_type term_type;
        // Insert the symbol.
        this->m_symbol_set = symbol_fset{name};
        // Construct and insert the term.
        this->insert(term_type(Cf(1), typename term_type::key_type{Expo(1)}));
    }
    g_series_type2 &operator=(const g_series_type2 &) = default;
    g_series_type2 &operator=(g_series_type2 &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type2, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type2, base)
    // Provide fake sin/cos methods to test math overloads.
    g_series_type2 sin() const
    {
        return g_series_type2(42);
    }
    g_series_type2 cos() const
    {
        return g_series_type2(-42);
    }
};

template <typename Cf, typename Key>
class g_series_type3 : public series<Cf, Key, g_series_type3<Cf, Key>>
{
    typedef series<Cf, Key, g_series_type3<Cf, Key>> base;

public:
    template <typename Cf2>
    using rebind = g_series_type3<Cf2, Key>;
    g_series_type3() = default;
    g_series_type3(const g_series_type3 &) = default;
    g_series_type3(g_series_type3 &&) = default;
    g_series_type3 &operator=(const g_series_type3 &) = default;
    g_series_type3 &operator=(g_series_type3 &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type3, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type3, base)
};

namespace piranha
{

template <typename Cf, typename Key>
class series_multiplier<g_series_type<Cf, Key>, void> : public base_series_multiplier<g_series_type<Cf, Key>>
{
    using base = base_series_multiplier<g_series_type<Cf, Key>>;
    template <typename T>
    using call_enabler = typename std::enable_if<
        key_is_multipliable<typename T::term_type::cf_type, typename T::term_type::key_type>::value, int>::type;

public:
    using base::base;
    template <typename T = g_series_type<Cf, Key>, call_enabler<T> = 0>
    g_series_type<Cf, Key> operator()() const
    {
        return this->plain_multiplication();
    }
};

template <typename Cf, typename Key>
class series_multiplier<g_series_type2<Cf, Key>, void> : public base_series_multiplier<g_series_type2<Cf, Key>>
{
    using base = base_series_multiplier<g_series_type2<Cf, Key>>;
    template <typename T>
    using call_enabler = typename std::enable_if<
        key_is_multipliable<typename T::term_type::cf_type, typename T::term_type::key_type>::value, int>::type;

public:
    using base::base;
    template <typename T = g_series_type2<Cf, Key>, call_enabler<T> = 0>
    g_series_type2<Cf, Key> operator()() const
    {
        return this->plain_multiplication();
    }
};

template <typename Cf, typename Key>
class series_multiplier<g_series_type3<Cf, Key>, void> : public base_series_multiplier<g_series_type3<Cf, Key>>
{
    using base = base_series_multiplier<g_series_type3<Cf, Key>>;
    template <typename T>
    using call_enabler = typename std::enable_if<
        key_is_multipliable<typename T::term_type::cf_type, typename T::term_type::key_type>::value, int>::type;

public:
    using base::base;
    template <typename T = g_series_type3<Cf, Key>, call_enabler<T> = 0>
    g_series_type3<Cf, Key> operator()() const
    {
        return this->plain_multiplication();
    }
};
}

// Mock coefficient, not differentiable.
struct mock_cf {
    mock_cf();
    explicit mock_cf(const int &);
    mock_cf(const mock_cf &);
    mock_cf(mock_cf &&) noexcept;
    mock_cf &operator=(const mock_cf &);
    mock_cf &operator=(mock_cf &&) noexcept;
    friend std::ostream &operator<<(std::ostream &, const mock_cf &);
    mock_cf operator-() const;
    bool operator==(const mock_cf &) const;
    bool operator!=(const mock_cf &) const;
    mock_cf &operator+=(const mock_cf &);
    mock_cf &operator-=(const mock_cf &);
    mock_cf operator+(const mock_cf &) const;
    mock_cf operator-(const mock_cf &) const;
    mock_cf &operator*=(const mock_cf &);
    mock_cf operator*(const mock_cf &)const;
};

TEST_CASE("series_partial_test")
{
#if defined(MPPP_WITH_MPFR)
    //mppp::real_set_default_prec(100); // is that really used below . Anyhere a mppp::real???
#endif
    {
        typedef g_series_type<rational, int> p_type1;
        p_type1 x1{"x"};
        CHECK(is_differentiable<p_type1>::value);
        CHECK((std::is_same<decltype(x1.partial("foo")), p_type1>::value));
        p_type1 x{"x"}, y{"y"};
        CHECK(math::partial(x, "x") == 1);
        CHECK(math::partial(x, "y") == 0);
        CHECK(math::partial(-4 * x.pow(2), "x") == -8 * x);
        CHECK(math::partial(-4 * x.pow(2) + y * x, "y") == x);
        CHECK(math::partial(math::partial(-4 * x.pow(2), "x"), "x") == -8);
        CHECK(math::partial(math::partial(math::partial(-4 * x.pow(2), "x"), "x"), "x") == 0);
        CHECK(math::partial(-x + 1, "x") == -1);
        CHECK(math::partial((1 + 2 * x).pow(10), "x") == 20 * (1 + 2 * x).pow(9));
        CHECK(math::partial((1 + 2 * x + y).pow(10), "x") == 20 * (1 + 2 * x + y).pow(9));
        CHECK(math::partial(x * (1 + 2 * x + y).pow(10), "x") ==
                          20 * x * (1 + 2 * x + y).pow(9) + (1 + 2 * x + y).pow(10));
        CHECK(math::partial((1 + 2 * x + y).pow(0), "x").empty());
        // Custom derivatives.
        p_type1::register_custom_derivative("x", [](const p_type1 &) { return p_type1{rational(1, 314)}; });
        CHECK(math::partial(x, "x") == rational(1, 314));
        p_type1::register_custom_derivative("x", [](const p_type1 &) { return p_type1{rational(1, 315)}; });
        CHECK(math::partial(x, "x") == rational(1, 315));
        p_type1::unregister_custom_derivative("x");
        p_type1::unregister_custom_derivative("x");
        CHECK(math::partial(x, "x") == 1);
        // y as implicit function of x: y = x**2.
        p_type1::register_custom_derivative(
            "x", [x](const p_type1 &p) -> p_type1 { return p.partial("x") + math::partial(p, "y") * 2 * x; });
        CHECK(math::partial(x + y, "x") == 1 + 2 * x);
        p_type1::unregister_custom_derivative("y");
        p_type1::unregister_custom_derivative("x");
        CHECK(math::partial(x + y, "x") == 1);
        CHECK(math::partial(x + 2 * y, "y") == 2);
        p_type1::register_custom_derivative("x", [](const p_type1 &p) { return p.partial("x"); });
        CHECK(math::partial(x + y, "x") == 1);
        CHECK(math::partial(x + y * x, "x") == y + 1);
        p_type1::register_custom_derivative(
            "x", [x](const p_type1 &p) -> p_type1 { return p.partial("x") + math::partial(p, "y") * 2 * x; });
        p_type1::register_custom_derivative("y", [](const p_type1 &p) -> p_type1 { return 2 * p; });
        CHECK(math::partial(x + y, "x") == 1 + 4 * x * (x + y));
        CHECK(math::partial(x + y, "y") == 2 * (x + y));
        p_type1::unregister_all_custom_derivatives();
        CHECK(math::partial(x + y, "x") == 1);
        CHECK(math::partial(x + 3 * y, "y") == 3);
    }
    {
        typedef g_series_type<integer, rational> p_type2;
        using p_type2_diff = g_series_type<rational, rational>;
        p_type2 x2{"x"};
        CHECK(is_differentiable<p_type2>::value);
        CHECK((std::is_same<decltype(x2.partial("foo")), p_type2_diff>::value));
        p_type2 x{"x"}, y{"y"};
        CHECK(math::partial(x, "x") == 1);
        CHECK(math::partial(x, "y") == 0);
        CHECK(math::partial(-4 * x.pow(2), "x") == -8 * x);
        CHECK(math::partial(-4 * x.pow(2) + y * x, "y") == x);
        CHECK(math::partial(math::partial(-4 * x.pow(2), "x"), "x") == -8);
        CHECK(math::partial(math::partial(math::partial(-4 * x.pow(2), "x"), "x"), "x") == 0);
        CHECK(math::partial(-x + 1, "x") == -1);
        CHECK(math::partial((1 + 2 * x).pow(10), "x") == 20 * (1 + 2 * x).pow(9));
        CHECK(math::partial((1 + 2 * x + y).pow(10), "x") == 20 * (1 + 2 * x + y).pow(9));
        CHECK(math::partial(x * (1 + 2 * x + y).pow(10), "x") ==
                          20 * x * (1 + 2 * x + y).pow(9) + (1 + 2 * x + y).pow(10));
        CHECK(math::partial((1 + 2 * x + y).pow(0), "x").empty());
        // Custom derivatives.
        p_type2::register_custom_derivative("x", [](const p_type2 &) { return p_type2_diff{rational(1, 314)}; });
        CHECK(math::partial(x, "x") == rational(1, 314));
        p_type2::register_custom_derivative("x", [](const p_type2 &) { return p_type2_diff{rational(1, 315)}; });
        CHECK(math::partial(x, "x") == rational(1, 315));
        p_type2::unregister_custom_derivative("x");
        CHECK(math::partial(x, "x") == 1);
        // y as implicit function of x: y = x**2.
        p_type2::register_custom_derivative(
            "x", [x](const p_type2 &p) { return p.partial("x") + math::partial(p, "y") * 2 * x; });
        CHECK(math::partial(x + y, "x") == 1 + 2 * x);
        p_type2::unregister_custom_derivative("y");
        p_type2::unregister_custom_derivative("x");
        CHECK(math::partial(x + y, "x") == 1);
        CHECK(math::partial(x + 2 * y, "y") == 2);
        p_type2::register_custom_derivative("x", [](const p_type2 &p) { return p.partial("x"); });
        CHECK(math::partial(x + y, "x") == 1);
        CHECK(math::partial(x + y * x, "x") == y + 1);
        p_type2::register_custom_derivative(
            "x", [x](const p_type2 &p) { return p.partial("x") + math::partial(p, "y") * 2 * x; });
        p_type2::register_custom_derivative("y", [](const p_type2 &p) { return 2 * p_type2_diff{p}; });
        CHECK(math::partial(x + y, "x") == 1 + 4 * x * (x + y));
        CHECK(math::partial(x + y, "y") == 2 * (x + y));
        p_type2::unregister_all_custom_derivatives();
        CHECK(math::partial(x + y, "x") == 1);
        CHECK(math::partial(x + 3 * y, "y") == 3);
    }
    // Check with mock_cf.
    CHECK((!is_differentiable<g_series_type<mock_cf, rational>>::value));
    {
        using s0 = g_series_type<double, rational>;
        using ss0 = g_series_type<s0, rational>;
        // Series as coefficient.
        CHECK((is_differentiable<ss0>::value));
        CHECK(math::partial(s0{"y"} * ss0{"x"}, "y") == ss0{"x"});
        CHECK(math::partial(s0{"y"} * ss0{"x"}, "x") == s0{"y"});
        CHECK(math::partial(s0{"y"} * piranha::pow(ss0{"x"}, 5) , "x") ==
                          5 * s0{"y"} * piranha::pow(ss0{"x"}, 4));
    }
}

#if defined(PIRANHA_WITH_BOOST_S11N)

static const int ntries = 1000;

TEST_CASE("series_serialization_test")
{
    // Serialization test done with a randomly-generated series.
    typedef g_series_type<rational, int> p_type1;
    auto x = p_type1{"x"}, y = p_type1{"y"}, z = p_type1{"z"};
    std::uniform_int_distribution<int> int_dist(0, 5);
    std::uniform_int_distribution<unsigned> size_dist(0u, 10u);
    p_type1 tmp;
    for (int i = 0; i < ntries; ++i) {
        p_type1 p;
        const unsigned size = size_dist(rng);
        for (unsigned j = 0u; j < size; ++j) {
            p += piranha::pow(x, int_dist(rng)) * piranha::pow(y, int_dist(rng)) * piranha::pow(z, int_dist(rng));
        }
        p *= int_dist(rng);
        const auto div = int_dist(rng);
        if (div) {
            p /= div;
        }
        std::stringstream ss;
        {
            boost::archive::text_oarchive oa(ss);
            oa << p;
        }
        {
            boost::archive::text_iarchive ia(ss);
            ia >> tmp;
        }
        CHECK(tmp == p);
    }
}

#endif

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

namespace std
{

template <>
struct hash<mock_key> {
    std::size_t operator()(const mock_key &) const;
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
}

TEST_CASE("series_evaluate_test")
{
    typedef g_series_type<rational, int> p_type1;
    typedef symbol_fmap<rational> dict_type;
    typedef symbol_fmap<int> dict_type_int;
    typedef symbol_fmap<long> dict_type_long;
    CHECK((is_evaluable<p_type1, rational>::value));
    CHECK((is_evaluable<p_type1, integer>::value));
    CHECK((is_evaluable<p_type1, int>::value));
    CHECK((is_evaluable<p_type1, long>::value));
    CHECK((std::is_same<rational, decltype(math::evaluate(p_type1{}, dict_type_int{}))>::value));
    CHECK((std::is_same<rational, decltype(math::evaluate(p_type1{}, dict_type_long{}))>::value));
    CHECK(math::evaluate(p_type1{}, dict_type{}) == 0);
    p_type1 x{"x"}, y{"y"};
    CHECK_THROWS_AS(math::evaluate(x, dict_type{}), std::invalid_argument);
    CHECK(math::evaluate(x, dict_type{{"x", rational(1)}}) == 1);
    CHECK_THROWS_AS(math::evaluate(x + (2 * y).pow(3), dict_type{{"x", rational(1)}}), std::invalid_argument);
    CHECK(math::evaluate(x + (2 * y).pow(3), dict_type{{"x", rational(1)}, {"y", rational(2, 3)}}) ==
                      rational(1) + piranha::pow(2 * rational(2, 3), 3));
    CHECK(math::evaluate(x + (2 * y).pow(3), dict_type{{"x", rational(1)}, {"y", rational(2, 3)}}) ==
                      math::evaluate(x + (2 * y).pow(3), dict_type{{"x", rational(1)}, {"y", rational(2, 3)}}));
    CHECK((std::is_same<decltype(math::evaluate(p_type1{}, dict_type{})), rational>::value));
#if defined(MPPP_WITH_MPFR)
    typedef symbol_fmap<real> dict_type2;
    CHECK((is_evaluable<p_type1, real>::value));
    CHECK(                                                       //TODO:: failes because of different precision 
        math::evaluate(x + (2 * y).pow(3), dict_type2{{"x", real(1.234, 113)}, {"y", real(-5.678, 113)}, {"z", real()}}) ==
        real(1.234, 113) + piranha::pow(2 * real(-5.678, 113), 3));
    CHECK(
        math::evaluate(x + (2 * y).pow(3), dict_type2{{"x", real(1.234)}, {"y", real(-5.678)}, {"z", real()}}) ==
        math::evaluate(x + piranha::pow(2 * y, 3), dict_type2{{"x", real(1.234)}, {"y", real(-5.678)}, {"z", real()}}));
    CHECK((std::is_same<decltype(math::evaluate(p_type1{}, dict_type2{})), real>::value));
#endif
    typedef symbol_fmap<double> dict_type3;
    CHECK((is_evaluable<p_type1, double>::value));
    CHECK(math::evaluate(x + (2 * y).pow(3), dict_type3{{"x", 1.234}, {"y", -5.678}, {"z", 0.0001}}) ==
                      1.234 + piranha::pow(2 * -5.678, 3));
    CHECK(
        math::evaluate(x + (2 * y).pow(3), dict_type3{{"x", 1.234}, {"y", -5.678}, {"z", 0.0001}}) ==
        math::evaluate(x + piranha::pow(2 * y, 3), dict_type3{{"x", 1.234}, {"y", -5.678}, {"z", 0.0001}}));
    CHECK((std::is_same<decltype(math::evaluate(p_type1{}, dict_type3{})), double>::value));
    CHECK((std::is_same<decltype(math::evaluate<double>(g_series_type3<double, mock_key>{}, {})),
                              g_series_type3<double, mock_key>>::value));
    CHECK((std::is_same<decltype(math::evaluate<double>(g_series_type3<mock_cf, monomial<int>>{}, {})),
                              g_series_type3<mock_cf, monomial<int>>>::value));
    CHECK((std::is_same<decltype(math::evaluate<double>(g_series_type3<mock_cf, mock_key>{}, {})),
                              g_series_type3<mock_cf, mock_key>>::value));
    CHECK((is_evaluable<g_series_type3<double, monomial<int>>, double>::value));
    // Check the syntax from initializer list with explicit template parameter.
    CHECK(math::evaluate<int>(p_type1{}, {{"foo", 4.}}) == 0);
    CHECK(math::evaluate<double>(p_type1{}, {{"foo", 4.}, {"bar", 7}}) == 0);
}

template <typename Expo>
class g_series_type_nr : public series<float, monomial<Expo>, g_series_type_nr<Expo>>
{
    typedef series<float, monomial<Expo>, g_series_type_nr<Expo>> base;

public:
    g_series_type_nr() = default;
    g_series_type_nr(const g_series_type_nr &) = default;
    g_series_type_nr(g_series_type_nr &&) = default;
    g_series_type_nr &operator=(const g_series_type_nr &) = default;
    g_series_type_nr &operator=(g_series_type_nr &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type_nr, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type_nr, base)
};

template <typename Expo>
class g_series_type_nr2 : public series<short, monomial<Expo>, g_series_type_nr2<Expo>>
{
    typedef series<short, monomial<Expo>, g_series_type_nr2<Expo>> base;

public:
    template <typename Expo2>
    using rebind = g_series_type_nr2<Expo2>;
    g_series_type_nr2() = default;
    g_series_type_nr2(const g_series_type_nr2 &) = default;
    g_series_type_nr2(g_series_type_nr2 &&) = default;
    g_series_type_nr2 &operator=(const g_series_type_nr2 &) = default;
    g_series_type_nr2 &operator=(g_series_type_nr2 &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type_nr2, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type_nr2, base)
};

template <typename Expo>
class g_series_type_nr3 : public series<float, monomial<Expo>, g_series_type_nr3<Expo>>
{
    typedef series<float, monomial<Expo>, g_series_type_nr3<Expo>> base;

public:
    template <typename Expo2>
    using rebind = void;
    g_series_type_nr3() = default;
    g_series_type_nr3(const g_series_type_nr3 &) = default;
    g_series_type_nr3(g_series_type_nr3 &&) = default;
    g_series_type_nr3 &operator=(const g_series_type_nr3 &) = default;
    g_series_type_nr3 &operator=(g_series_type_nr3 &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type_nr3, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type_nr3, base)
};

TEST_CASE("series_series_is_rebindable_test")
{
    typedef g_series_type<rational, int> p_type1;
    CHECK((series_is_rebindable<p_type1, int>::value));
    CHECK((std::is_same<series_rebind<p_type1, int>, g_series_type<int, int>>::value));
    CHECK((series_is_rebindable<p_type1, rational>::value));
    CHECK((std::is_same<series_rebind<p_type1, rational>, p_type1>::value));
    CHECK((std::is_same<series_rebind<p_type1 &, rational const>, p_type1>::value));
    CHECK((series_is_rebindable<p_type1, p_type1>::value));
    CHECK((series_is_rebindable<p_type1 &, p_type1>::value));
    CHECK((series_is_rebindable<p_type1 &, const p_type1>::value));
    CHECK((std::is_same<series_rebind<p_type1, p_type1>, g_series_type<p_type1, int>>::value));
    typedef g_series_type_nr<int> p_type_nr;
    CHECK((!series_is_rebindable<p_type_nr, unsigned>::value));
    CHECK((!series_is_rebindable<p_type_nr, integer>::value));
    CHECK((!series_is_rebindable<p_type_nr &, unsigned const>::value));
    CHECK((!series_is_rebindable<p_type_nr &&, const integer &>::value));
    typedef g_series_type_nr2<int> p_type_nr2;
    CHECK((!series_is_rebindable<p_type_nr2, unsigned>::value));
    CHECK((!series_is_rebindable<p_type_nr2, integer>::value));
    typedef g_series_type_nr3<int> p_type_nr3;
    CHECK((!series_is_rebindable<p_type_nr3, unsigned>::value));
    CHECK((!series_is_rebindable<p_type_nr3, integer>::value));
    // Check when the requirements on the input types are not satisfied.
    CHECK((!series_is_rebindable<p_type1, std::string>::value));
    CHECK((!series_is_rebindable<p_type1, std::vector<std::string>>::value));
    CHECK((!series_is_rebindable<p_type1, std::vector<std::string> &>::value));
    CHECK((!series_is_rebindable<p_type1, const std::vector<std::string> &>::value));
    CHECK((!series_is_rebindable<p_type1, std::vector<std::string> &&>::value));
    CHECK((!series_is_rebindable<std::string, std::vector<std::string>>::value));
    CHECK((!series_is_rebindable<const std::string &, std::vector<std::string>>::value));
    CHECK((!series_is_rebindable<const std::string &, std::vector<std::string> &&>::value));
}

TEST_CASE("series_series_recursion_index_test")
{
    CHECK(series_recursion_index<int>::value == 0u);
    CHECK(series_recursion_index<double>::value == 0u);
    CHECK(series_recursion_index<float>::value == 0u);
    CHECK((series_recursion_index<g_series_type<rational, int>>::value) == 1u);
    CHECK((series_recursion_index<g_series_type<float, int>>::value) == 1u);
    CHECK((series_recursion_index<g_series_type<double, int>>::value) == 1u);
    CHECK((series_recursion_index<g_series_type<g_series_type<double, int>, int>>::value) == 2u);
    CHECK((series_recursion_index<g_series_type<g_series_type<double, int>, long>>::value) == 2u);
    CHECK(
        (series_recursion_index<g_series_type<g_series_type<g_series_type<double, int>, int>, long>>::value) == 3u);
    CHECK(
        (series_recursion_index<g_series_type<g_series_type<g_series_type<rational, int>, int>, long>>::value) == 3u);
    CHECK(
        (series_recursion_index<g_series_type<g_series_type<g_series_type<rational, int>, int>, long> &>::value) == 3u);
    CHECK(
        (series_recursion_index<g_series_type<g_series_type<g_series_type<rational, int>, int>, long> const>::value) ==
        3u);
    CHECK(
        (series_recursion_index<g_series_type<g_series_type<g_series_type<rational, int>, int>, long> const &>::value) ==
        3u);
}

template <typename T>
using typedef_type_t = typename T::type;

template <typename T>
using has_typedef_type = is_detected<typedef_type_t, T>;

template <typename T, typename U>
using binary_series_op_return_type = detail::binary_series_op_return_type<T, U, 0>;

TEST_CASE("series_binary_series_op_return_type_test")
{
    // Check missing type in case both operands are not series.
    CHECK((!has_typedef_type<binary_series_op_return_type<int, int>>::value));
    CHECK((!has_typedef_type<binary_series_op_return_type<int, double>>::value));
    CHECK((!has_typedef_type<binary_series_op_return_type<float, double>>::value));
    // Case 0.
    // NOTE: this cannot fail in any way as we require coefficients to be addable in is_cf.
    typedef g_series_type<rational, int> p_type1;
    CHECK((std::is_same<p_type1, binary_series_op_return_type<p_type1, p_type1>::type>::value));
    // Case 1 and 2.
    typedef g_series_type<double, int> p_type2;
    CHECK((std::is_same<p_type2, binary_series_op_return_type<p_type2, p_type1>::type>::value));
    CHECK((std::is_same<p_type2, binary_series_op_return_type<p_type1, p_type2>::type>::value));
    // mock_cf supports only multiplication vs mock_cf.
    CHECK((!has_typedef_type<
                 binary_series_op_return_type<g_series_type<double, int>, g_series_type<mock_cf, int>>>::value));
    CHECK((!has_typedef_type<
                 binary_series_op_return_type<g_series_type<mock_cf, int>, g_series_type<double, int>>>::value));
    // Case 3.
    typedef g_series_type<short, int> p_type3;
    CHECK((std::is_same<g_series_type<int, int>, binary_series_op_return_type<p_type3, p_type3>::type>::value));
    typedef g_series_type<char, int> p_type4;
    CHECK((std::is_same<g_series_type<int, int>, binary_series_op_return_type<p_type3, p_type4>::type>::value));
    CHECK((std::is_same<g_series_type<int, int>, binary_series_op_return_type<p_type4, p_type3>::type>::value));
    // Wrong rebind implementations.
    CHECK(
        (!has_typedef_type<binary_series_op_return_type<g_series_type_nr2<int>, g_series_type<char, int>>>::value));
    CHECK(
        (!has_typedef_type<binary_series_op_return_type<g_series_type<char, int>, g_series_type_nr2<int>>>::value));
    // Case 4 and 6.
    CHECK((std::is_same<p_type2, binary_series_op_return_type<p_type2, int>::type>::value));
    CHECK((std::is_same<p_type2, binary_series_op_return_type<int, p_type2>::type>::value));
    // mock_cf does not support multiplication with int.
    CHECK((!has_typedef_type<binary_series_op_return_type<g_series_type<mock_cf, int>, int>>::value));
    CHECK((!has_typedef_type<binary_series_op_return_type<int, g_series_type<mock_cf, int>>>::value));
    // Case 5 and 7.
    CHECK((std::is_same<p_type2, binary_series_op_return_type<p_type3, double>::type>::value));
    CHECK((std::is_same<p_type2, binary_series_op_return_type<double, p_type3>::type>::value));
    CHECK((std::is_same<g_series_type<int, int>, binary_series_op_return_type<p_type4, short>::type>::value));
    CHECK((std::is_same<g_series_type<int, int>, binary_series_op_return_type<short, p_type4>::type>::value));
    // These need rebinding, but rebind is not supported.
    CHECK((!has_typedef_type<binary_series_op_return_type<g_series_type_nr<int>, double>>::value));
    CHECK((!has_typedef_type<binary_series_op_return_type<double, g_series_type_nr<int>>>::value));
    // Wrong implementation of rebind.
    CHECK(
        (!has_typedef_type<binary_series_op_return_type<g_series_type_nr2<char>, g_series_type<char, char>>>::value));
    CHECK(
        (!has_typedef_type<binary_series_op_return_type<g_series_type<char, char>, g_series_type_nr2<char>>>::value));
    // Same coefficients, amibguity in series type.
    CHECK(
        (!has_typedef_type<binary_series_op_return_type<g_series_type_nr<int>, g_series_type<float, int>>>::value));
}

struct arithmetics_add_tag {
};

namespace piranha
{
template <>
class debug_access<arithmetics_add_tag>
{
public:
    template <typename Cf>
    struct runner {
        template <typename Expo>
        void operator()(const Expo &) const
        {
            typedef g_series_type<Cf, Expo> p_type1;
            typedef g_series_type2<Cf, Expo> p_type2;
            typedef g_series_type<int, Expo> p_type3;
            // Binary add first.
            // Some type checks - these are not addable as they result in an ambiguity
            // between two series with same coefficient but different series types.
            CHECK((!is_addable<p_type1, p_type2>::value));
            CHECK((!is_addable<p_type2, p_type1>::value));
            CHECK((!is_addable_in_place<p_type1, p_type2>::value));
            CHECK((!is_addable_in_place<p_type2, p_type1>::value));
            // Various subcases of case 0.
            p_type1 x{"x"}, y{"y"};
            // No need to merge args.
            auto tmp = x + x;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Try with moves on both sides.
            tmp = p_type1{x} + x;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            tmp = x + p_type1{x};
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            tmp = p_type1{x} + p_type1{x};
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Check that move erases.
            auto x_copy(x);
            tmp = std::move(x) + x_copy;
            CHECK(x.size() == 0u);
            x = x_copy;
            tmp = x_copy + std::move(x);
            CHECK(x.size() == 0u);
            x = x_copy;
            // A few self move tests.
            tmp = std::move(x) + std::move(x);
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            x = p_type1{"x"};
            tmp = x + std::move(x);
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            x = p_type1{"x"};
            tmp = std::move(x) + x;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            x = p_type1{"x"};
            // Now with merging.
            tmp = x + y;
            CHECK(tmp.size() == 2u);
            auto it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // With moves.
            tmp = p_type1{x} + y;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x + p_type1{y};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Test the swapping of operands when one series is larger than the other.
            tmp = (x + y) + x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(2)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(2)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x + (y + x);
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(2)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(2)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Some tests for case 1/4.
            tmp = x + p_type3{"y"};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x + (p_type3{"y"} + p_type3{"x"});
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x + 1;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 1u);
            ++it;
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Symmetric of the previous case.
            tmp = p_type3{"y"} + x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = (p_type3{"y"} + p_type3{"x"}) + x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = 1 + x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 1u);
            ++it;
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Case 3/5 and symmetric.
            using p_type4 = g_series_type<g_series_type<int, Expo>, Expo>;
            using p_type5 = g_series_type<double, Expo>;
            auto tmp2 = p_type4{"x"} + p_type5{"y"};
            CHECK(tmp2.size() == 2u);
            CHECK((std::is_same<decltype(tmp2), g_series_type<g_series_type<double, Expo>, Expo>>::value));
            auto it2 = tmp2.m_container.begin();
            CHECK((it2->m_cf == g_series_type<double, Expo>{"y"} || it2->m_cf == 1));
            CHECK(it2->m_key.size() == 1u);
            ++it2;
            CHECK((it2->m_cf == g_series_type<double, Expo>{"y"} || it2->m_cf == 1));
            CHECK(it2->m_key.size() == 1u);
            CHECK((tmp2.m_symbol_set == symbol_fset{"x"}));
            tmp2 = p_type5{"y"} + p_type4{"x"};
            CHECK(tmp2.size() == 2u);
            CHECK((std::is_same<decltype(tmp2), g_series_type<g_series_type<double, Expo>, Expo>>::value));
            it2 = tmp2.m_container.begin();
            CHECK((it2->m_cf == g_series_type<double, Expo>{"y"} || it2->m_cf == 1));
            CHECK(it2->m_key.size() == 1u);
            ++it2;
            CHECK((it2->m_cf == g_series_type<double, Expo>{"y"} || it2->m_cf == 1));
            CHECK(it2->m_key.size() == 1u);
            CHECK((tmp2.m_symbol_set == symbol_fset{"x"}));
            // Now in-place.
            // Case 0.
            tmp = x;
            tmp += x;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Move.
            tmp = x;
            tmp += p_type1{x};
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Check that a move really happens.
            tmp = x;
            tmp += std::move(x);
            // NOTE: here the symbol set has still size 1 as it does not get moved
            // (it gets moved only when operands are swapped because of difference
            // in sizes or because it's a sub operation).
            CHECK(x.size() == 0u);
            x = p_type1{"x"};
            // Move self.
            tmp += std::move(tmp);
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1) + Cf(1) + Cf(1) + Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Now with merging.
            tmp = x;
            tmp += y;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // With moves.
            tmp = x;
            tmp += p_type1{y};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Test the swapping of operands when one series is larger than the other.
            tmp = x + y;
            tmp += x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x;
            tmp += y + x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Some tests for case 1/4.
            tmp = x;
            tmp += p_type3{"y"};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x;
            tmp += p_type3{"y"} + p_type3{"x"};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == 2));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x;
            tmp += 1;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 1u);
            ++it;
            CHECK(it->m_cf == 1);
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Symmetric of the previous case.
            p_type3 tmp3{"y"};
            tmp3 += x;
            CHECK(tmp3.size() == 2u);
            auto it3 = tmp3.m_container.begin();
            CHECK(it3->m_cf == 1);
            CHECK(it3->m_key.size() == 2u);
            ++it3;
            CHECK(it3->m_cf == 1);
            CHECK(it3->m_key.size() == 2u);
            CHECK((tmp3.m_symbol_set == symbol_fset{"x", "y"}));
            tmp3 += p_type3{"y"} + p_type3{"x"};
            tmp3 += x;
            CHECK(tmp3.size() == 2u);
            it3 = tmp3.m_container.begin();
            CHECK((it3->m_cf == 2 || it3->m_cf == 3));
            CHECK(it3->m_key.size() == 2u);
            ++it3;
            CHECK((it3->m_cf == 2 || it3->m_cf == 3));
            CHECK(it3->m_key.size() == 2u);
            CHECK((tmp3.m_symbol_set == symbol_fset{"x", "y"}));
            // Case 3/5.
            auto tmp4 = p_type4{"x"};
            tmp4 += p_type5{"y"};
            CHECK(tmp4.size() == 2u);
            auto it4 = tmp4.m_container.begin();
            CHECK((std::is_same<decltype(it4->m_cf), g_series_type<int, Expo>>::value));
            CHECK((it4->m_cf == g_series_type<int, Expo>{"y"} || it4->m_cf == 1));
            CHECK(it4->m_key.size() == 1u);
            ++it4;
            CHECK((it4->m_cf == g_series_type<int, Expo>{"y"} || it4->m_cf == 1));
            CHECK(it4->m_key.size() == 1u);
            CHECK((tmp4.m_symbol_set == symbol_fset{"x"}));
            // Check with scalar on the left.
            CHECK((!is_addable_in_place<int, p_type1>::value));
            CHECK((!is_addable_in_place<int, p_type2>::value));
            CHECK((!is_addable_in_place<int, p_type3>::value));
        }
    };
    template <typename Cf>
    void operator()(const Cf &) const
    {
        tuple_for_each(expo_types{}, runner<Cf>());
    }
};
}

typedef debug_access<arithmetics_add_tag> arithmetics_add_tester;

TEST_CASE("series_arithmetics_add_test")
{
    // Functional testing.
    tuple_for_each(cf_types{}, arithmetics_add_tester());
    // Type testing for binary addition.
    typedef g_series_type<rational, int> p_type1;
    typedef g_series_type<int, rational> p_type2;
    typedef g_series_type<short, rational> p_type3;
    typedef g_series_type<char, rational> p_type4;
    // First let's check the output type.
    // Case 0.
    CHECK((std::is_same<p_type1, decltype(p_type1{} + p_type1{})>::value));
    // Case 1.
    CHECK((std::is_same<p_type1, decltype(p_type1{} + p_type2{})>::value));
    // Case 2.
    CHECK((std::is_same<p_type1, decltype(p_type2{} + p_type1{})>::value));
    // Case 3, symmetric.
    CHECK((std::is_same<p_type2, decltype(p_type3{} + p_type4{})>::value));
    CHECK((std::is_same<p_type2, decltype(p_type4{} + p_type3{})>::value));
    // Case 4.
    CHECK((std::is_same<p_type1, decltype(p_type1{} + 0)>::value));
    // Case 5.
    CHECK((std::is_same<p_type2, decltype(p_type3{} + 0)>::value));
    // Case 6.
    CHECK((std::is_same<p_type1, decltype(0 + p_type1{})>::value));
    // Case 7.
    CHECK((std::is_same<p_type2, decltype(0 + p_type3{})>::value));
    // Check non-addable series.
    typedef g_series_type2<rational, int> p_type5;
    CHECK((!is_addable<p_type1, p_type5>::value));
    CHECK((!is_addable<p_type5, p_type1>::value));
    // Check coefficient series.
    typedef g_series_type<p_type1, int> p_type11;
    typedef g_series_type<p_type2, rational> p_type22;
    typedef g_series_type<p_type1, rational> p_type21;
    CHECK((std::is_same<p_type11, decltype(p_type1{} + p_type11{})>::value));
    CHECK((std::is_same<p_type11, decltype(p_type11{} + p_type1{})>::value));
    CHECK((std::is_same<p_type21, decltype(p_type1{} + p_type22{})>::value));
    CHECK((std::is_same<p_type21, decltype(p_type22{} + p_type1{})>::value));
    CHECK((std::is_same<p_type11, decltype(p_type11{} + p_type22{})>::value));
    CHECK((std::is_same<p_type11, decltype(p_type22{} + p_type11{})>::value));
    // Type testing for in-place addition.
    // Case 0.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() += p_type1{})>::value));
    // Case 1.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() += p_type2{})>::value));
    // Case 2.
    CHECK((std::is_same<p_type2 &, decltype(std::declval<p_type2 &>() += p_type1{})>::value));
    // Case 3, symmetric.
    CHECK((std::is_same<p_type3 &, decltype(std::declval<p_type3 &>() += p_type4{})>::value));
    CHECK((std::is_same<p_type4 &, decltype(std::declval<p_type4 &>() += p_type3{})>::value));
    // Case 4.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() += 0)>::value));
    // Case 5.
    CHECK((std::is_same<p_type3 &, decltype(std::declval<p_type3 &>() += 0)>::value));
    // Cases 6 and 7 do not make sense at the moment.
    CHECK((!is_addable_in_place<int, p_type3>::value));
    CHECK((!is_addable_in_place<p_type1, p_type11>::value));
    // Checks for coefficient series.
    p_type11 tmp;
    CHECK((std::is_same<p_type11 &, decltype(tmp += p_type1{})>::value));
    p_type22 tmp2;
    CHECK((std::is_same<p_type22 &, decltype(tmp2 += p_type1{})>::value));
}

struct arithmetics_sub_tag {
};

namespace piranha
{
template <>
class debug_access<arithmetics_sub_tag>
{
public:
    template <typename Cf>
    struct runner {
        template <typename Expo>
        void operator()(const Expo &) const
        {
            typedef g_series_type<Cf, Expo> p_type1;
            typedef g_series_type2<Cf, Expo> p_type2;
            typedef g_series_type<int, Expo> p_type3;
            // Binary sub first.
            // Some type checks - these are not subtractable as they result in an ambiguity
            // between two series with same coefficient but different series types.
            CHECK((!is_subtractable<p_type1, p_type2>::value));
            CHECK((!is_subtractable<p_type2, p_type1>::value));
            CHECK((!is_subtractable_in_place<p_type1, p_type2>::value));
            CHECK((!is_subtractable_in_place<p_type2, p_type1>::value));
            // Various subcases of case 0.
            p_type1 x{"x"}, y{"y"}, x2 = x + x;
            // No need to merge args.
            auto tmp = x2 - x;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Check going to zero.
            tmp = x - x;
            CHECK(tmp.size() == 0u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Try with moves on both sides.
            tmp = p_type1{x} - x2;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(-1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            tmp = x2 - p_type1{x};
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            tmp = p_type1{x2} - p_type1{x};
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Check that move erases.
            auto x_copy(x);
            tmp = std::move(x) - x_copy;
            CHECK(x.size() == 0u);
            x = x_copy;
            tmp = x_copy - std::move(x);
            CHECK(x.size() == 0u);
            x = x_copy;
            // Self move tests.
            tmp = std::move(x) - std::move(x);
            CHECK(tmp.size() == 0u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            x = p_type1{"x"};
            tmp = x - std::move(x);
            CHECK(tmp.size() == 0u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            x = p_type1{"x"};
            tmp = std::move(x) - x;
            CHECK(tmp.size() == 0u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            x = p_type1{"x"};
            // Now with merging.
            tmp = x - y;
            CHECK(tmp.size() == 2u);
            auto it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // With moves.
            tmp = p_type1{x} - y;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x - p_type1{y};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Test the swapping of operands when one series is larger than the other.
            tmp = (x2 - y) - x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x2 - (y - x);
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 3 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 3 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Some tests for case 1/4.
            tmp = x - p_type3{"y"};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x2 - (p_type3{"y"} - p_type3{"x"});
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 3 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 3 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x - 1;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 1u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Symmetric of the previous case.
            tmp = p_type3{"y"} - x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = (p_type3{"y"} - p_type3{"x"}) - x2;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == -3));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == -3));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = 1 - x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 1u);
            ++it;
            CHECK((it->m_cf == 1 || it->m_cf == -1));
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Case 3/5 and symmetric.
            using p_type4 = g_series_type<g_series_type<int, Expo>, Expo>;
            using p_type5 = g_series_type<double, Expo>;
            auto tmp2 = p_type4{"x"} - p_type5{"y"};
            CHECK(tmp2.size() == 2u);
            CHECK((std::is_same<decltype(tmp2), g_series_type<g_series_type<double, Expo>, Expo>>::value));
            auto it2 = tmp2.m_container.begin();
            CHECK((it2->m_cf == -g_series_type<double, Expo>{"y"} || it2->m_cf == 1));
            CHECK(it2->m_key.size() == 1u);
            ++it2;
            CHECK((it2->m_cf == -g_series_type<double, Expo>{"y"} || it2->m_cf == 1));
            CHECK(it2->m_key.size() == 1u);
            CHECK((tmp2.m_symbol_set == symbol_fset{"x"}));
            tmp2 = p_type5{"y"} - p_type4{"x"};
            CHECK(tmp2.size() == 2u);
            CHECK((std::is_same<decltype(tmp2), g_series_type<g_series_type<double, Expo>, Expo>>::value));
            it2 = tmp2.m_container.begin();
            CHECK((it2->m_cf == g_series_type<double, Expo>{"y"} || it2->m_cf == -1));
            CHECK(it2->m_key.size() == 1u);
            ++it2;
            CHECK((it2->m_cf == g_series_type<double, Expo>{"y"} || it2->m_cf == -1));
            CHECK(it2->m_key.size() == 1u);
            CHECK((tmp2.m_symbol_set == symbol_fset{"x"}));
            // Now in-place.
            // Case 0.
            tmp = x2;
            tmp -= x;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Check that a move really happens.
            tmp = x;
            tmp -= std::move(x);
            CHECK(x.size() == 0u);
            x = p_type1{"x"};
            // Move.
            tmp = x2;
            tmp -= p_type1{x};
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Now with merging.
            tmp = x;
            tmp -= y;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // With moves.
            tmp = x;
            tmp -= p_type1{y};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Move self.
            tmp -= std::move(tmp);
            CHECK(tmp.size() == 0u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Test the swapping of operands when one series is larger than the other.
            tmp = x2 - y;
            tmp -= x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x;
            tmp -= y - x2;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(3) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(3) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Some tests for case 1/4.
            tmp = x;
            tmp -= p_type3{"y"};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x2;
            tmp -= p_type3{"y"} - p_type3{"x"};
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(3) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK((it->m_cf == Cf(3) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x;
            tmp -= 1;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 1u);
            ++it;
            CHECK((it->m_cf == Cf(1) || it->m_cf == Cf(-1)));
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Symmetric of the previous case.
            p_type3 tmp3{"y"};
            tmp3 -= x;
            CHECK(tmp3.size() == 2u);
            auto it3 = tmp3.m_container.begin();
            CHECK((it3->m_cf == Cf(1) || it3->m_cf == Cf(-1)));
            CHECK(it3->m_key.size() == 2u);
            ++it3;
            CHECK((it3->m_cf == Cf(1) || it3->m_cf == Cf(-1)));
            CHECK(it3->m_key.size() == 2u);
            CHECK((tmp3.m_symbol_set == symbol_fset{"x", "y"}));
            tmp3 = p_type3{"x"};
            tmp3 -= p_type3{"y"} - p_type3{"x"};
            tmp3 -= x;
            CHECK(tmp3.size() == 2u);
            it3 = tmp3.m_container.begin();
            CHECK((it3->m_cf == Cf(1) || it3->m_cf == Cf(-1)));
            CHECK(it3->m_key.size() == 2u);
            ++it3;
            CHECK((it3->m_cf == Cf(1) || it3->m_cf == Cf(-1)));
            CHECK(it3->m_key.size() == 2u);
            CHECK((tmp3.m_symbol_set == symbol_fset{"x", "y"}));
            // Case 3/5.
            auto tmp4 = p_type4{"x"};
            tmp4 -= p_type5{"y"};
            CHECK(tmp4.size() == 2u);
            auto it4 = tmp4.m_container.begin();
            CHECK((std::is_same<decltype(it4->m_cf), g_series_type<int, Expo>>::value));
            CHECK((it4->m_cf == -g_series_type<int, Expo>{"y"} || it4->m_cf == 1));
            CHECK(it4->m_key.size() == 1u);
            ++it4;
            CHECK((it4->m_cf == -g_series_type<int, Expo>{"y"} || it4->m_cf == 1));
            CHECK(it4->m_key.size() == 1u);
            CHECK((tmp4.m_symbol_set == symbol_fset{"x"}));
            // Check with scalar on the left.
            CHECK((!is_subtractable_in_place<int, p_type1>::value));
            CHECK((!is_subtractable_in_place<int, p_type2>::value));
            CHECK((!is_subtractable_in_place<int, p_type3>::value));
        }
    };
    template <typename Cf>
    void operator()(const Cf &) const
    {
        tuple_for_each(expo_types{}, runner<Cf>());
    }
};
}

typedef debug_access<arithmetics_sub_tag> arithmetics_sub_tester;

TEST_CASE("series_arithmetics_sub_test")
{
    // Functional testing.
    tuple_for_each(cf_types{}, arithmetics_sub_tester());
    // Type testing for binary subtraction.
    typedef g_series_type<rational, int> p_type1;
    typedef g_series_type<int, rational> p_type2;
    typedef g_series_type<short, rational> p_type3;
    typedef g_series_type<char, rational> p_type4;
    // First let's check the output type.
    // Case 0.
    CHECK((std::is_same<p_type1, decltype(p_type1{} - p_type1{})>::value));
    // Case 1.
    CHECK((std::is_same<p_type1, decltype(p_type1{} - p_type2{})>::value));
    // Case 2.
    CHECK((std::is_same<p_type1, decltype(p_type2{} - p_type1{})>::value));
    // Case 3, symmetric.
    CHECK((std::is_same<p_type2, decltype(p_type3{} - p_type4{})>::value));
    CHECK((std::is_same<p_type2, decltype(p_type4{} - p_type3{})>::value));
    // Case 4.
    CHECK((std::is_same<p_type1, decltype(p_type1{} - 0)>::value));
    // Case 5.
    CHECK((std::is_same<p_type2, decltype(p_type3{} - 0)>::value));
    // Case 6.
    CHECK((std::is_same<p_type1, decltype(0 - p_type1{})>::value));
    // Case 7.
    CHECK((std::is_same<p_type2, decltype(0 - p_type3{})>::value));
    // Check non-subtractable series.
    typedef g_series_type2<rational, int> p_type5;
    CHECK((!is_subtractable<p_type1, p_type5>::value));
    CHECK((!is_subtractable<p_type5, p_type1>::value));
    // Check coefficient series.
    typedef g_series_type<p_type1, int> p_type11;
    typedef g_series_type<p_type2, rational> p_type22;
    typedef g_series_type<p_type1, rational> p_type21;
    CHECK((std::is_same<p_type11, decltype(p_type1{} - p_type11{})>::value));
    CHECK((std::is_same<p_type11, decltype(p_type11{} - p_type1{})>::value));
    CHECK((std::is_same<p_type21, decltype(p_type1{} - p_type22{})>::value));
    CHECK((std::is_same<p_type21, decltype(p_type22{} - p_type1{})>::value));
    CHECK((std::is_same<p_type11, decltype(p_type11{} - p_type22{})>::value));
    CHECK((std::is_same<p_type11, decltype(p_type22{} - p_type11{})>::value));
    // Type testing for in-place subtraction.
    // Case 0.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() -= p_type1{})>::value));
    // Case 1.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() -= p_type2{})>::value));
    // Case 2.
    CHECK((std::is_same<p_type2 &, decltype(std::declval<p_type2 &>() -= p_type1{})>::value));
    // Case 3, symmetric.
    CHECK((std::is_same<p_type3 &, decltype(std::declval<p_type3 &>() -= p_type4{})>::value));
    CHECK((std::is_same<p_type4 &, decltype(std::declval<p_type4 &>() -= p_type3{})>::value));
    // Case 4.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() -= 0)>::value));
    // Case 5.
    CHECK((std::is_same<p_type3 &, decltype(std::declval<p_type3 &>() -= 0)>::value));
    // Cases 6 and 7 do not make sense at the moment.
    CHECK((!is_subtractable_in_place<int, p_type3>::value));
    CHECK((!is_subtractable_in_place<p_type1, p_type11>::value));
    // Checks for coefficient series.
    p_type11 tmp;
    CHECK((std::is_same<p_type11 &, decltype(tmp -= p_type1{})>::value));
    p_type22 tmp2;
    CHECK((std::is_same<p_type22 &, decltype(tmp2 -= p_type1{})>::value));
}

struct arithmetics_mul_tag {
};

namespace piranha
{
template <>
class debug_access<arithmetics_mul_tag>
{
public:
    template <typename Cf>
    struct runner {
        template <typename Expo>
        void operator()(const Expo &) const
        {
            typedef g_series_type<Cf, Expo> p_type1;
            typedef g_series_type2<Cf, Expo> p_type2;
            typedef g_series_type<int, Expo> p_type3;
            // Binary mul first.
            // Some type checks - these are not multipliable as they result in an ambiguity
            // between two series with same coefficient but different series types.
            CHECK((!is_multipliable<p_type1, p_type2>::value));
            CHECK((!is_multipliable<p_type2, p_type1>::value));
            CHECK((!is_multipliable_in_place<p_type1, p_type2>::value));
            CHECK((!is_multipliable_in_place<p_type2, p_type1>::value));
            // Various subcases of case 0.
            p_type1 x{"x"}, y{"y"};
            // No need to merge args.
            auto tmp = 2 * x * x;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(2) * Cf(1));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Try with moves on both sides.
            tmp = 3 * p_type1{x} * 2 * x;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(3) * Cf(2));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            tmp = 2 * x * 3 * p_type1{x};
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(2) * Cf(3));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Now with merging.
            tmp = x * y;
            CHECK(tmp.size() == 1u);
            auto it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(1) * Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // With moves.
            tmp = p_type1{x} * y;
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(1) * Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x * p_type1{y};
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(1) * Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Test the swapping of operands when one series is larger than the other.
            tmp = (x + y) * 2 * x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(2) * Cf(1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == Cf(2) * Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x * (2 * y + 2 * x);
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(2) * Cf(1));
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == Cf(2) * Cf(1));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Some tests for case 1/4.
            tmp = 3 * x * p_type3{"y"};
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 3);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = 3 * x * (p_type3{"y"} + p_type3{"x"});
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 3);
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == 3);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x * 2;
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 2);
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Symmetric of the previous case.
            tmp = p_type3{"y"} * x * 3;
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 3);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = (p_type3{"y"} + p_type3{"x"}) * 4 * x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 4);
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == 4);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = -2 * x;
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == -2);
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Case 3/5 and symmetric.
            using p_type4 = g_series_type<g_series_type<int, Expo>, Expo>;
            using p_type5 = g_series_type<double, Expo>;
            auto tmp2 = p_type4{"x"} * p_type5{"y"} * -1;
            CHECK(tmp2.size() == 1u);
            CHECK((std::is_same<decltype(tmp2), g_series_type<g_series_type<double, Expo>, Expo>>::value));
            auto it2 = tmp2.m_container.begin();
            CHECK((it2->m_cf == -g_series_type<double, Expo>{"y"}));
            CHECK(it2->m_key.size() == 1u);
            CHECK((tmp2.m_symbol_set == symbol_fset{"x"}));
            tmp2 = p_type5{"y"} * p_type4{"x"} * 2;
            CHECK(tmp2.size() == 1u);
            CHECK((std::is_same<decltype(tmp2), g_series_type<g_series_type<double, Expo>, Expo>>::value));
            it2 = tmp2.m_container.begin();
            CHECK((it2->m_cf == 2 * g_series_type<double, Expo>{"y"}));
            CHECK(it2->m_key.size() == 1u);
            CHECK((tmp2.m_symbol_set == symbol_fset{"x"}));
            // Now in-place.
            // Case 0.
            tmp = 2 * x;
            tmp *= x;
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(2));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Move.
            tmp = 2 * x;
            tmp *= p_type1{x};
            CHECK(tmp.size() == 1u);
            CHECK(tmp.m_container.begin()->m_cf == Cf(2));
            CHECK(tmp.m_container.begin()->m_key.size() == 1u);
            CHECK(tmp.m_symbol_set == symbol_fset{"x"});
            // Now with merging.
            tmp = -3 * x;
            tmp *= y;
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(-3));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // With moves.
            tmp = 4 * x;
            tmp *= p_type1{y};
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(4));
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Test the swapping of operands when one series is larger than the other.
            tmp = 4 * (x + y);
            tmp *= x;
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 4);
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == 4);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x;
            tmp *= 3 * (y + x);
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 3);
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == 3);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            // Some tests for case 1/4.
            tmp = 4 * x;
            tmp *= p_type3{"y"};
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 4);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x;
            tmp *= -4 * (p_type3{"y"} + p_type3{"x"});
            CHECK(tmp.size() == 2u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == -4);
            CHECK(it->m_key.size() == 2u);
            ++it;
            CHECK(it->m_cf == -4);
            CHECK(it->m_key.size() == 2u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x", "y"}));
            tmp = x;
            tmp *= 3;
            CHECK(tmp.size() == 1u);
            it = tmp.m_container.begin();
            CHECK(it->m_cf == 3);
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Symmetric of the previous case.
            p_type3 tmp3{"y"};
            tmp3 *= -4 * x;
            CHECK(tmp3.size() == 1u);
            auto it3 = tmp3.m_container.begin();
            CHECK(it3->m_cf == -4);
            CHECK(it3->m_key.size() == 2u);
            CHECK((tmp3.m_symbol_set == symbol_fset{"x", "y"}));
            tmp3 *= p_type3{"y"} + p_type3{"x"};
            tmp3 *= -x;
            CHECK(tmp3.size() == 2u);
            it3 = tmp3.m_container.begin();
            CHECK(it3->m_cf == 4);
            CHECK(it3->m_key.size() == 2u);
            CHECK((tmp3.m_symbol_set == symbol_fset{"x", "y"}));
            // Case 3/5.
            auto tmp4 = p_type4{"x"};
            tmp4 *= p_type5{"y"} * 3;
            CHECK(tmp4.size() == 1u);
            auto it4 = tmp4.m_container.begin();
            CHECK((std::is_same<decltype(it4->m_cf), g_series_type<int, Expo>>::value));
            CHECK((it4->m_cf == 3 * g_series_type<int, Expo>{"y"}));
            CHECK(it4->m_key.size() == 1u);
            CHECK((tmp4.m_symbol_set == symbol_fset{"x"}));
            // Check with scalar on the left.
            CHECK((!is_multipliable_in_place<int, p_type1>::value));
            CHECK((!is_multipliable_in_place<int, p_type2>::value));
            CHECK((!is_multipliable_in_place<int, p_type3>::value));
        }
    };
    template <typename Cf>
    void operator()(const Cf &) const
    {
        tuple_for_each(expo_types{}, runner<Cf>());
    }
};
}

typedef debug_access<arithmetics_mul_tag> arithmetics_mul_tester;

TEST_CASE("series_arithmetics_mul_test")
{
    // Functional testing.
    tuple_for_each(cf_types{}, arithmetics_mul_tester());
    // Type testing for binary multiplication.
    typedef g_series_type<rational, int> p_type1;
    typedef g_series_type<int, rational> p_type2;
    typedef g_series_type<short, rational> p_type3;
    typedef g_series_type<char, rational> p_type4;
    // First let's check the output type.
    // Case 0.
    CHECK((std::is_same<p_type1, decltype(p_type1{} * p_type1{})>::value));
    // Case 1.
    CHECK((std::is_same<p_type1, decltype(p_type1{} * p_type2{})>::value));
    // Case 2.
    CHECK((std::is_same<p_type1, decltype(p_type2{} * p_type1{})>::value));
    // Case 3, symmetric.
    CHECK((std::is_same<p_type2, decltype(p_type3{} * p_type4{})>::value));
    CHECK((std::is_same<p_type2, decltype(p_type4{} * p_type3{})>::value));
    // Case 4.
    CHECK((std::is_same<p_type1, decltype(p_type1{} * 0)>::value));
    // Case 5.
    CHECK((std::is_same<p_type2, decltype(p_type3{} * 0)>::value));
    // Case 6.
    CHECK((std::is_same<p_type1, decltype(0 * p_type1{})>::value));
    // Case 7.
    CHECK((std::is_same<p_type2, decltype(0 * p_type3{})>::value));
    // Check non-multipliable series.
    typedef g_series_type2<rational, int> p_type5;
    CHECK((!is_multipliable<p_type1, p_type5>::value));
    CHECK((!is_multipliable<p_type5, p_type1>::value));
    // Check coefficient series.
    typedef g_series_type<p_type1, int> p_type11;
    typedef g_series_type<p_type2, rational> p_type22;
    typedef g_series_type<p_type1, rational> p_type21;
    CHECK((std::is_same<p_type11, decltype(p_type1{} * p_type11{})>::value));
    CHECK((std::is_same<p_type11, decltype(p_type11{} * p_type1{})>::value));
    CHECK((std::is_same<p_type21, decltype(p_type1{} * p_type22{})>::value));
    CHECK((std::is_same<p_type21, decltype(p_type22{} * p_type1{})>::value));
    CHECK((std::is_same<p_type11, decltype(p_type11{} * p_type22{})>::value));
    CHECK((std::is_same<p_type11, decltype(p_type22{} * p_type11{})>::value));
    // Type testing for in-place multiplication.
    // Case 0.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() *= p_type1{})>::value));
    // Case 1.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() *= p_type2{})>::value));
    // Case 2.
    CHECK((std::is_same<p_type2 &, decltype(std::declval<p_type2 &>() *= p_type1{})>::value));
    // Case 3, symmetric.
    CHECK((std::is_same<p_type3 &, decltype(std::declval<p_type3 &>() *= p_type4{})>::value));
    CHECK((std::is_same<p_type4 &, decltype(std::declval<p_type4 &>() *= p_type3{})>::value));
    // Case 4.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() *= 0)>::value));
    // Case 5.
    CHECK((std::is_same<p_type3 &, decltype(std::declval<p_type3 &>() *= 0)>::value));
    // Cases 6 and 7 do not make sense at the moment.
    CHECK((!is_multipliable_in_place<int, p_type3>::value));
    CHECK((!is_multipliable_in_place<p_type1, p_type11>::value));
    // Checks for coefficient series.
    p_type11 tmp;
    CHECK((std::is_same<p_type11 &, decltype(tmp *= p_type1{})>::value));
    p_type22 tmp2;
    CHECK((std::is_same<p_type22 &, decltype(tmp2 *= p_type1{})>::value));
}

struct arithmetics_div_tag {
};

namespace piranha
{
template <>
class debug_access<arithmetics_div_tag>
{
public:
    template <typename Cf>
    struct runner {
        template <typename Expo>
        void operator()(const Expo &) const
        {
            typedef g_series_type<Cf, Expo> p_type1;
            p_type1 x{"x"};
            // Some tests for case 4.
            auto tmp = 3 * x / 2;
            CHECK(tmp.size() == 1u);
            auto it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(3) / 2);
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Case 5.
            auto tmp2 = 3 * x / 2.;
            auto it2 = tmp2.m_container.begin();
            CHECK(it2->m_cf == Cf(3) / 2.);
            CHECK(it2->m_key.size() == 1u);
            CHECK((tmp2.m_symbol_set == symbol_fset{"x"}));
            // In-place.
            // Case 4.
            tmp = 3 * x;
            tmp /= 2;
            it = tmp.m_container.begin();
            CHECK(it->m_cf == Cf(3) / 2);
            CHECK(it->m_key.size() == 1u);
            CHECK((tmp.m_symbol_set == symbol_fset{"x"}));
            // Case 5.
            tmp2 = 3 * x;
            tmp2 /= 2.;
            it2 = tmp2.m_container.begin();
            CHECK(it2->m_cf == Cf(3) / 2.);
            CHECK(it2->m_key.size() == 1u);
            CHECK((tmp2.m_symbol_set == symbol_fset{"x"}));
            // Test division by zero of empty series.
            if (std::is_same<integer, Cf>::value) {
                CHECK_THROWS_AS(p_type1{} / 0, mppp::zero_division_error);
                p_type1 zero;
                CHECK_THROWS_AS(zero /= 0, mppp::zero_division_error);
            }
            if (std::is_same<rational, Cf>::value) {
                CHECK_THROWS_AS(p_type1{} / 0, mppp::zero_division_error);
                p_type1 zero;
                CHECK_THROWS_AS(zero /= 0, mppp::zero_division_error);
            }
            // Check with scalar on the left.
            CHECK((!is_divisible_in_place<int, p_type1>::value));
        }
    };
    template <typename Cf>
    void operator()(const Cf &) const
    {
        tuple_for_each(expo_types{}, runner<Cf>());
    }
};
}

typedef debug_access<arithmetics_div_tag> arithmetics_div_tester;

TEST_CASE("series_arithmetics_div_test")
{
    // Functional testing.
    tuple_for_each(cf_types{}, arithmetics_div_tester());
    // Type testing for binary division.
    typedef g_series_type<rational, int> p_type1;
    typedef g_series_type<p_type1, int> p_type11;
    typedef g_series_type<double, int> p_type1d;
    typedef g_series_type<float, int> p_type1f;
    // First let's check the output type.
    // Case 4.
    CHECK((std::is_same<p_type1, decltype(p_type1{} / 0)>::value));
    CHECK((std::is_same<p_type1, decltype(p_type1{} / integer{})>::value));
    CHECK((std::is_same<p_type1, decltype(p_type1{} / rational{})>::value));
    // Case 5.
    CHECK((std::is_same<p_type1d, decltype(p_type1{} / 0.)>::value));
    CHECK((std::is_same<p_type1f, decltype(p_type1{} / 0.f)>::value));
    // Some scalars on the first argument.
    CHECK((is_divisible<double, p_type1>::value));
    CHECK((std::is_same<decltype(3. / p_type1{}), g_series_type<double, int>>::value));
    CHECK((is_divisible<int, p_type1>::value));
    CHECK((std::is_same<decltype(3 / p_type1{}), p_type1>::value));
    CHECK((is_divisible<integer, p_type1>::value));
    CHECK((std::is_same<decltype(3_z / p_type1{}), p_type1>::value));
    // Type testing for in-place division.
    // Case 4.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() /= 0)>::value));
    // Case 5.
    CHECK((std::is_same<p_type1 &, decltype(std::declval<p_type1 &>() /= 0.)>::value));
    // Not divisible in-place.
    CHECK((!is_divisible_in_place<int, p_type1>::value));
    // Divisible in-place after recent changes.
    CHECK((is_divisible_in_place<p_type11, p_type1>::value));
    // Special cases to test the erasing of terms.
    using pint = g_series_type<integer, int>;
    pint x{"x"}, y{"y"};
    auto tmp = 2 * x + y;
    tmp /= 2;
    CHECK(tmp == x);
    tmp = 2 * x + 2 * y;
    tmp /= 3;
    CHECK(tmp.empty());
    // Check zero division error.
    tmp = 2 * x + y;
    CHECK_THROWS_AS(tmp /= 0, mppp::zero_division_error);
    CHECK(tmp.empty());
}

struct eq_tag {
};

namespace piranha
{
template <>
class debug_access<eq_tag>
{
public:
    template <typename Cf>
    struct runner {
        template <typename Expo>
        void operator()(const Expo &) const
        {
            typedef g_series_type<Cf, Expo> p_type1;
            typedef g_series_type2<Cf, Expo> p_type2;
            typedef g_series_type<int, Expo> p_type3;
            // Some type checks - these are not comparable as they result in an ambiguity
            // between two series with same coefficient but different series types.
            CHECK((!is_equality_comparable<const p_type1 &, const p_type2 &>::value));
            CHECK((!is_equality_comparable<const p_type2 &, const p_type1 &>::value));
            CHECK((!is_equality_comparable<const p_type1 &, const p_type2 &>::value));
            CHECK((!is_equality_comparable<const p_type2 &, const p_type1 &>::value));
            // Various subcases of case 0.
            p_type1 x{"x"}, y{"y"};
            CHECK(x == x);
            CHECK(y == y);
            CHECK(x == x + y - y);
            CHECK(y == y + x - x);
            // Arguments merging on both sides.
            CHECK(x != y);
            // Check with series of different size.
            CHECK(x != y + x);
            // Arguments merging on the other side.
            CHECK(y + x != y);
            // Some tests for case 1/4.
            CHECK(x != p_type3{"y"});
            CHECK(y != p_type3{"x"});
            CHECK(x != p_type3{"y"} + p_type3{"x"});
            CHECK(y != p_type3{"x"} + p_type3{"y"});
            CHECK(x == p_type3{"x"});
            CHECK(x == p_type3{"x"} + p_type3{"y"} - p_type3{"y"});
            CHECK(x != 0);
            CHECK(y != 0);
            CHECK(x - x == 0);
            CHECK(p_type1{1} == 1);
            CHECK(p_type1{-1} == -1);
            // Symmetric of above.
            CHECK(p_type3{"y"} != x);
            CHECK(p_type3{"x"} != y);
            CHECK(p_type3{"y"} + p_type3{"x"} != x);
            CHECK(p_type3{"x"} + p_type3{"y"} != y);
            CHECK(p_type3{"x"} == x);
            CHECK(p_type3{"x"} + p_type3{"y"} - p_type3{"y"} == x);
            CHECK(0 != x);
            CHECK(0 != y);
            CHECK(0 == x - x);
            CHECK(1 == p_type1{1});
            CHECK(-1 == p_type1{-1});
            // Case 3/5 and symmetric.
            using p_type4 = g_series_type<g_series_type<int, Expo>, Expo>;
            using p_type5 = g_series_type<double, Expo>;
            CHECK((p_type4{g_series_type<int, Expo>{"x"}}) == p_type5{"x"});
            CHECK(p_type5{"x"} == (p_type4{g_series_type<int, Expo>{"x"}}));
            CHECK((p_type4{g_series_type<int, Expo>{"x"}} != p_type5{"y"}));
            CHECK((p_type5{"y"} != p_type4{g_series_type<int, Expo>{"x"}}));
        }
    };
    template <typename Cf>
    void operator()(const Cf &) const
    {
        tuple_for_each(expo_types{}, runner<Cf>());
    }
};
}

typedef debug_access<eq_tag> eq_tester;

TEST_CASE("series_eq_test")
{
    // Functional testing.
    tuple_for_each(cf_types{}, eq_tester());
    // Type testing for binary addition.
    typedef g_series_type<rational, int> p_type1;
    typedef g_series_type<int, rational> p_type2;
    typedef g_series_type<short, rational> p_type3;
    typedef g_series_type<char, rational> p_type4;
    // First let's check the output type.
    // Case 0.
    CHECK((std::is_same<bool, decltype(p_type1{} == p_type1{})>::value));
    CHECK((std::is_same<bool, decltype(p_type1{} != p_type1{})>::value));
    // Case 1.
    CHECK((std::is_same<bool, decltype(p_type1{} == p_type2{})>::value));
    CHECK((std::is_same<bool, decltype(p_type1{} != p_type2{})>::value));
    // Case 2.
    CHECK((std::is_same<bool, decltype(p_type2{} == p_type1{})>::value));
    CHECK((std::is_same<bool, decltype(p_type2{} != p_type1{})>::value));
    // Case 3, symmetric.
    CHECK((std::is_same<bool, decltype(p_type3{} == p_type4{})>::value));
    CHECK((std::is_same<bool, decltype(p_type3{} != p_type4{})>::value));
    CHECK((std::is_same<bool, decltype(p_type4{} == p_type3{})>::value));
    CHECK((std::is_same<bool, decltype(p_type4{} != p_type3{})>::value));
    // Case 4.
    CHECK((std::is_same<bool, decltype(p_type1{} == 0)>::value));
    CHECK((std::is_same<bool, decltype(p_type1{} != 0)>::value));
    // Case 5.
    CHECK((std::is_same<bool, decltype(p_type3{} == 0)>::value));
    CHECK((std::is_same<bool, decltype(p_type3{} != 0)>::value));
    // Case 6.
    CHECK((std::is_same<bool, decltype(0 == p_type1{})>::value));
    CHECK((std::is_same<bool, decltype(0 != p_type1{})>::value));
    // Case 7.
    CHECK((std::is_same<bool, decltype(0 == p_type3{})>::value));
    CHECK((std::is_same<bool, decltype(0 != p_type3{})>::value));
    // Check non-addable series.
    typedef g_series_type2<rational, int> p_type5;
    CHECK((!is_equality_comparable<p_type1, p_type5>::value));
    CHECK((!is_equality_comparable<p_type5, p_type1>::value));
    // Check coefficient series.
    typedef g_series_type<p_type1, int> p_type11;
    typedef g_series_type<p_type2, rational> p_type22;
    CHECK((std::is_same<bool, decltype(p_type1{} == p_type11{})>::value));
    CHECK((std::is_same<bool, decltype(p_type1{} != p_type11{})>::value));
    CHECK((std::is_same<bool, decltype(p_type11{} == p_type1{})>::value));
    CHECK((std::is_same<bool, decltype(p_type11{} != p_type1{})>::value));
    CHECK((std::is_same<bool, decltype(p_type1{} == p_type22{})>::value));
    CHECK((std::is_same<bool, decltype(p_type1{} != p_type22{})>::value));
    CHECK((std::is_same<bool, decltype(p_type22{} == p_type1{})>::value));
    CHECK((std::is_same<bool, decltype(p_type22{} != p_type1{})>::value));
    CHECK((std::is_same<bool, decltype(p_type11{} == p_type22{})>::value));
    CHECK((std::is_same<bool, decltype(p_type11{} != p_type22{})>::value));
    CHECK((std::is_same<bool, decltype(p_type22{} == p_type11{})>::value));
    CHECK((std::is_same<bool, decltype(p_type22{} != p_type11{})>::value));
}

TEST_CASE("series_hash_test")
{
    typedef g_series_type<rational, int> p_type1;
    typedef g_series_type<integer, int> p_type2;
    CHECK(p_type1{}.hash() == 0u);
    CHECK(p_type2{}.hash() == 0u);
    // Check that only the key is used to compute the hash.
    CHECK(p_type1{"x"}.hash() == p_type2{"x"}.hash());
    auto x = p_type1{"x"}, y = p_type1{"y"}, x2 = (x + y) - y;
    // NOTE: this is not 100% sure as the hash mixing in the monomial could actually lead to identical hashes.
    // But the probability should be rather low.
    CHECK(x.hash() != x2.hash());
    // This shows we cannot use standard equality operator in hash tables.
    CHECK(x == x2);
    // A bit more testing.
    CHECK((x + 2 * y).hash() == (x + y + y).hash());
    CHECK((x + 2 * y - y).hash() == (x + y).hash());
}

TEST_CASE("series_is_identical_test")
{
    typedef g_series_type<rational, int> p_type1;
    CHECK(p_type1{}.is_identical(p_type1{}));
    auto x = p_type1{"x"}, y = p_type1{"y"}, x2 = (x + y) - y;
    CHECK(x.is_identical(x));
    CHECK(x.is_identical(p_type1{"x"}));
    CHECK(!x.is_identical(y));
    CHECK(!y.is_identical(x));
    CHECK(x2 == x);
    CHECK(!x2.is_identical(x));
    CHECK(!x.is_identical(x2));
    CHECK(x.is_identical(x2.trim()));
    CHECK(x2.trim().is_identical(x));
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

TEST_CASE("series_has_series_multiplier_test")
{
    typedef g_series_type<rational, int> p_type1;
    CHECK(series_has_multiplier<p_type1>::value);
    CHECK(series_has_multiplier<p_type1 &>::value);
    CHECK(series_has_multiplier<const p_type1 &>::value);
    typedef g_series_type<mock_cf3, int> p_type2;
    CHECK(!series_has_multiplier<p_type2>::value);
    CHECK(!series_has_multiplier<p_type2 const>::value);
    CHECK(!series_has_multiplier<p_type2 const &>::value);
    typedef g_series_type3<double, mock_key> p_type3;
    CHECK(!series_has_multiplier<p_type3>::value);
    CHECK(!series_has_multiplier<p_type3 &>::value);
    CHECK(!series_has_multiplier<p_type3 &&>::value);
}

// A non-multipliable series, missing a suitable series_multiplier specialisation.
template <typename Cf, typename Expo>
class g_series_type_nm : public series<Cf, monomial<Expo>, g_series_type_nm<Cf, Expo>>
{
    typedef series<Cf, monomial<Expo>, g_series_type_nm<Cf, Expo>> base;

public:
    template <typename Cf2>
    using rebind = g_series_type_nm<Cf2, Expo>;
    g_series_type_nm() = default;
    g_series_type_nm(const g_series_type_nm &) = default;
    g_series_type_nm(g_series_type_nm &&) = default;
    explicit g_series_type_nm(const char *name) : base()
    {
        typedef typename base::term_type term_type;
        // Insert the symbol.
        this->m_symbol_set.add(name);
        // Construct and insert the term.
        this->insert(term_type(Cf(1), typename term_type::key_type{Expo(1)}));
    }
    g_series_type_nm &operator=(const g_series_type_nm &) = default;
    g_series_type_nm &operator=(g_series_type_nm &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type_nm, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type_nm, base)
};

namespace piranha
{

template <typename Cf, typename Expo>
class series_multiplier<g_series_type_nm<Cf, Expo>, void>
{
};
}

TEST_CASE("series_no_series_multiplier_test")
{
    typedef g_series_type_nm<rational, int> p_type1;
    CHECK(!is_multipliable<p_type1>::value);
}

// Mock coefficient, with weird semantics for operator+(integer): the output is not a coefficient type.
struct mock_cf2 {
    mock_cf2();
    explicit mock_cf2(const int &);
    mock_cf2(const mock_cf2 &);
    mock_cf2(mock_cf2 &&) noexcept;
    mock_cf2 &operator=(const mock_cf2 &);
    mock_cf2 &operator=(mock_cf2 &&) noexcept;
    friend std::ostream &operator<<(std::ostream &, const mock_cf2 &);
    mock_cf2 operator-() const;
    bool operator==(const mock_cf2 &) const;
    bool operator!=(const mock_cf2 &) const;
    mock_cf2 &operator+=(const mock_cf2 &);
    mock_cf2 &operator-=(const mock_cf2 &);
    mock_cf2 operator+(const mock_cf2 &) const;
    mock_cf2 operator-(const mock_cf2 &) const;
    mock_cf2 &operator*=(const mock_cf2 &);
    mock_cf2 operator*(const mock_cf2 &)const;
    std::string operator+(const integer &) const;
    std::vector<std::string> operator*(const integer &)const;
    std::vector<std::string> operator-(const integer &) const;
};

// Check that attempting to rebind to an invalid coefficient disables the operator, rather
// than resulting in a static assertion firing (as it was the case in the past).
TEST_CASE("series_rebind_failure_test")
{
    CHECK(is_cf<mock_cf2>::value);
    CHECK((!is_addable<g_series_type<integer, int>, g_series_type<mock_cf2, int>>::value));
    CHECK((!is_addable<g_series_type<mock_cf2, int>, g_series_type<integer, int>>::value));
    CHECK((is_addable<g_series_type<mock_cf2, int>, g_series_type<mock_cf2, int>>::value));
    CHECK((!is_subtractable<g_series_type<integer, int>, g_series_type<mock_cf2, int>>::value));
    CHECK((!is_subtractable<g_series_type<mock_cf2, int>, g_series_type<integer, int>>::value));
    CHECK((is_subtractable<g_series_type<mock_cf2, int>, g_series_type<mock_cf2, int>>::value));
    CHECK((!is_multipliable<g_series_type<integer, int>, g_series_type<mock_cf2, int>>::value));
    CHECK((!is_multipliable<g_series_type<mock_cf2, int>, g_series_type<integer, int>>::value));
    CHECK((is_multipliable<g_series_type<mock_cf2, int>, g_series_type<mock_cf2, int>>::value));
}
