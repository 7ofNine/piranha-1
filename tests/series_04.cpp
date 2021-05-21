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

#include <algorithm>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <piranha/base_series_multiplier.hpp>
#include <piranha/forwarding.hpp>
#include <piranha/integer.hpp>
#include <piranha/key_is_multipliable.hpp>
#include <piranha/math.hpp>
#include <piranha/math/cos.hpp>
#include <piranha/math/is_zero.hpp>
#include <piranha/math/sin.hpp>
#include <piranha/monomial.hpp>
#include <piranha/polynomial.hpp>
#include <piranha/print_coefficient.hpp>
#include <piranha/print_tex_coefficient.hpp>
#include <piranha/rational.hpp>
#include <piranha/series_multiplier.hpp>
#include <piranha/settings.hpp>
#include <piranha/symbol_utils.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"

using namespace piranha;

typedef boost::mpl::vector<double, integer, rational> cf_types;
typedef boost::mpl::vector<unsigned, integer> expo_types;

template <typename Cf, typename Expo>
class g_series_type : public series<Cf, monomial<Expo>, g_series_type<Cf, Expo>>
{
public:
    template <typename Cf2>
    using rebind = g_series_type<Cf2, Expo>;
    typedef series<Cf, monomial<Expo>, g_series_type<Cf, Expo>> base;
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
    // Provide fake sin/cos methods with wrong sigs.
    g_series_type sin()
    {
        return g_series_type(42);
    }
    int cos() const
    {
        return -42;
    }
};

template <typename Cf, typename Expo>
class g_series_type2 : public series<Cf, monomial<Expo>, g_series_type2<Cf, Expo>>
{
public:
    template <typename Cf2>
    using rebind = g_series_type2<Cf2, Expo>;
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

template <typename Cf, typename Expo>
class g_series_type3 : public series<Cf, monomial<Expo>, g_series_type3<Cf, Expo>>
{
public:
    template <typename Cf2>
    using rebind = g_series_type3<Cf2, Expo>;
    typedef series<Cf, monomial<Expo>, g_series_type3<Cf, Expo>> base;
    g_series_type3() = default;
    g_series_type3(const g_series_type3 &) = default;
    g_series_type3(g_series_type3 &&) = default;
    explicit g_series_type3(const char *name) : base()
    {
        typedef typename base::term_type term_type;
        // Insert the symbol.
        this->m_symbol_set = symbol_fset{name};
        // Construct and insert the term.
        this->insert(term_type(Cf(1), typename term_type::key_type{Expo(1)}));
    }
    g_series_type3 &operator=(const g_series_type3 &) = default;
    g_series_type3 &operator=(g_series_type3 &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type3, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type3, base)
};

template <typename Cf, typename Expo>
class g_series_type4 : public series<Cf, monomial<Expo>, g_series_type4<Cf, Expo>>
{
public:
    template <typename Cf2>
    using rebind = g_series_type4<Cf2, Expo>;
    typedef series<Cf, monomial<Expo>, g_series_type4<Cf, Expo>> base;
    g_series_type4() = default;
    g_series_type4(const g_series_type4 &) = default;
    g_series_type4(g_series_type4 &&) = default;
    explicit g_series_type4(const char *name) : base()
    {
        typedef typename base::term_type term_type;
        // Insert the symbol.
        this->m_symbol_set = symbol_fset{name};
        // Construct and insert the term.
        this->insert(term_type(Cf(1), typename term_type::key_type{Expo(1)}));
    }
    g_series_type4 &operator=(const g_series_type4 &) = default;
    g_series_type4 &operator=(g_series_type4 &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type4, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type4, base)
    g_series_type4 sin() const;
    g_series_type4 cos() const;
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

template <typename Cf, typename Key>
class series_multiplier<g_series_type4<Cf, Key>, void> : public base_series_multiplier<g_series_type4<Cf, Key>>
{
    using base = base_series_multiplier<g_series_type4<Cf, Key>>;
    template <typename T>
    using call_enabler = typename std::enable_if<
        key_is_multipliable<typename T::term_type::cf_type, typename T::term_type::key_type>::value, int>::type;

public:
    using base::base;
    template <typename T = g_series_type4<Cf, Key>, call_enabler<T> = 0>
    g_series_type4<Cf, Key> operator()() const
    {
        return this->plain_multiplication();
    }
};
}

TEST_CASE("series_is_single_coefficient_test")
{
    typedef g_series_type<integer, int> p_type;
    CHECK(p_type{}.is_single_coefficient());
    CHECK(p_type{1}.is_single_coefficient());
    CHECK(!p_type{"x"}.is_single_coefficient());
    CHECK(!(3 * p_type{"x"}).is_single_coefficient());
    CHECK(!(1 + p_type{"x"}).is_single_coefficient());
}

// Mock coefficient.
struct mock_cf {
    mock_cf();
    mock_cf(const int &);
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

// Another mock cf, with valid sin/cos specialisations which change the type.
struct mock_cf2 {
    mock_cf2();
    mock_cf2(const int &);
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
};

namespace piranha
{

// Provide mock sine/cosine implementation returning unusable return type.
template <typename T>
class sin_impl<T, typename std::enable_if<std::is_same<T, mock_cf>::value>::type>
{
public:
    std::string operator()(const T &) const;
};

template <typename T>
class cos_impl<T, typename std::enable_if<std::is_same<T, mock_cf>::value>::type>
{
public:
    std::string operator()(const T &) const;
};

// Sin/cos of mock_cf2 will return mock_cf.
template <typename T>
class sin_impl<T, typename std::enable_if<std::is_same<T, mock_cf2>::value>::type>
{
public:
    mock_cf operator()(const T &) const;
};

template <typename T>
class cos_impl<T, typename std::enable_if<std::is_same<T, mock_cf2>::value>::type>
{
public:
    mock_cf operator()(const T &) const;
};
}

// NOTE: here:
// - g_series_type has a wrong sin() overload but a good cos() one, and g_series_type2 have suitable sin/cos members,
// - g_series_type3 has no members,
// - g_series_type4 has good members.
TEST_CASE("series_sin_cos_test")
{
    typedef g_series_type<double, int> p_type1;
    // What happens here:
    // - p_type1 has piranha::sin() via its coefficient type,
    // - g_series_type<mock_cf,int> has no sine because piranha::sin() on mock_cf is wrong,
    // - piranha::cos() on p_type1 returns the -42 value from the method.
    CHECK(is_sine_type<p_type1>::value);
    CHECK(is_cosine_type<p_type1>::value);
    CHECK((!is_sine_type<g_series_type<mock_cf, int>>::value));
    CHECK((is_cosine_type<g_series_type<mock_cf, int>>::value));
    CHECK(piranha::sin(p_type1{.5}) == piranha::sin(.5));
    CHECK(piranha::cos(p_type1{.5}) == -42);
    CHECK_THROWS_AS(piranha::sin(p_type1{"x"}), std::invalid_argument);
    CHECK_THROWS_AS(piranha::sin(p_type1{"x"} + 1), std::invalid_argument);
    CHECK(piranha::cos(p_type1{"x"}) == -42);
    CHECK(piranha::cos(p_type1{"x"} - 1) == -42);
    typedef g_series_type2<double, int> p_type2;
    CHECK(is_sine_type<p_type2>::value);
    CHECK(is_cosine_type<p_type2>::value);
    CHECK(piranha::sin(p_type2{.5}) == double(42));
    CHECK(piranha::cos(p_type2{.5}) == double(-42));
    typedef g_series_type2<p_type2, int> p_type3;
    CHECK(is_sine_type<p_type3>::value);
    CHECK(is_cosine_type<p_type3>::value);
    CHECK(piranha::sin(p_type3{.5}) == double(42));
    CHECK(piranha::cos(p_type3{.5}) == double(-42));
    typedef g_series_type<mock_cf2, int> p_type4;
    CHECK(is_sine_type<p_type4>::value);
    CHECK(is_cosine_type<p_type4>::value);
    CHECK((std::is_same<decltype(piranha::sin(p_type4{})), g_series_type<mock_cf, int>>::value));
    CHECK((std::is_same<decltype(piranha::cos(p_type4{})), int>::value));
    typedef g_series_type<mock_cf2, int> p_type4;
    CHECK(is_sine_type<p_type4>::value);
    CHECK(is_cosine_type<p_type4>::value);
    CHECK((std::is_same<decltype(piranha::sin(p_type4{})), g_series_type<mock_cf, int>>::value));
    CHECK((std::is_same<decltype(piranha::cos(p_type4{})), int>::value));
    typedef g_series_type3<mock_cf2, int> p_type5;
    CHECK(is_sine_type<p_type5>::value);
    CHECK(is_cosine_type<p_type5>::value);
    CHECK((std::is_same<decltype(piranha::sin(p_type5{})), g_series_type3<mock_cf, int>>::value));
    CHECK((std::is_same<decltype(piranha::cos(p_type5{})), g_series_type3<mock_cf, int>>::value));
    // Check that casting a series type to its base type and then calling sin/cos still gets out the original
    // type. Test with series with and without members.
    typedef g_series_type3<double, int> p_type6;
    CHECK(is_sine_type<p_type6>::value);
    CHECK(is_cosine_type<p_type6>::value);
    CHECK((std::is_same<p_type6, decltype(piranha::sin(std::declval<const p_type6::base &>()))>::value));
    CHECK((std::is_same<p_type6, decltype(piranha::cos(std::declval<const p_type6::base &>()))>::value));
    typedef g_series_type4<double, int> p_type7;
    CHECK(is_sine_type<p_type7>::value);
    CHECK(is_cosine_type<p_type7>::value);
    CHECK((std::is_same<p_type7, decltype(piranha::sin(std::declval<const p_type7::base &>()))>::value));
    CHECK((std::is_same<p_type7, decltype(piranha::cos(std::declval<const p_type7::base &>()))>::value));
    // Test also with bad members.
    CHECK((std::is_same<p_type1, decltype(piranha::sin(std::declval<const p_type1::base &>()))>::value));
    CHECK((std::is_same<p_type1, decltype(piranha::cos(std::declval<const p_type1::base &>()))>::value));
}

TEST_CASE("series_iterator_test")
{
    typedef g_series_type<rational, int> p_type1;
    p_type1 empty;
    CHECK(empty.begin() == empty.end());
    p_type1 x{"x"};
    typedef std::decay<decltype(*(x.begin()))>::type pair_type;
    CHECK((std::is_same<pair_type, std::pair<rational, p_type1>>::value));
    x *= 2;
    auto it = x.begin();
    CHECK(it->first == 2);
    CHECK((std::is_same<typename p_type1::term_type::cf_type, decltype(it->first)>::value));
    CHECK(it->second == p_type1{"x"});
    CHECK((std::is_same<p_type1, decltype(it->second)>::value));
    ++it;
    CHECK(it == x.end());
    x /= 2;
    p_type1 p1 = x + p_type1{"y"} + p_type1{"z"};
    p1 *= 3;
    it = p1.begin();
    CHECK(it != p1.end());
    CHECK(it->first == 3);
    ++it;
    CHECK(it != p1.end());
    CHECK(it->first == 3);
    ++it;
    CHECK(it != p1.end());
    CHECK(it->first == 3);
    ++it;
    CHECK(it == p1.end());
}

TEST_CASE("series_filter_test")
{
    typedef g_series_type<rational, int> p_type1;
    p_type1 x{"x"}, y{"y"}, z{"z"};
    typedef std::decay<decltype(*(x.begin()))>::type pair_type;
    CHECK(x == x.filter([](const pair_type &) { return true; }));
    CHECK(x.filter([](const pair_type &) { return false; }).empty());
    CHECK(x == (x + 2 * y).filter([](const pair_type &p) { return p.first < 2; }));
    CHECK(x + 2 * y == (x + 2 * y).filter([](const pair_type &p) { return p.second.size(); }));
    CHECK(0 == (x + 2 * y).filter([](const pair_type &p) { return p.second.size() == 0u; }));
    CHECK(-y == (x - y + 3).filter([](const pair_type &p) { return p.first < 0; }));
    CHECK(-y - 3 == (x - y - 3).filter([](const pair_type &p) { return p.first < 0; }));
    CHECK(x == (x - y - 3).filter([](const pair_type &p) { return p.first > 0; }));
}

TEST_CASE("series_transform_test")
{
    typedef g_series_type<rational, int> p_type1;
    p_type1 x{"x"}, y{"y"};
    typedef std::decay<decltype(*(x.begin()))>::type pair_type;
    CHECK(x == x.transform([](const pair_type &p) { return p; }));
    CHECK(0 == x.transform([](const pair_type &) { return pair_type(); }));
    CHECK(rational(1, 2) ==
                      x.transform([](const pair_type &) { return pair_type(rational(1, 2), p_type1(1)); }));
    CHECK(2 * (x + y) ==
                      (x + y).transform([](const pair_type &p) { return pair_type(p.first * 2, p.second); }));
    typedef g_series_type<p_type1, int> p_type2;
    p_type2 y2{"y"};
    y2 *= (x + 2);
    y2 += p_type2{"x"};
    typedef std::decay<decltype(*(y2.begin()))>::type pair_type2;
    CHECK(y2.transform([](const pair_type2 &p) {
        return std::make_pair(p.first.filter([](const pair_type &q) { return q.first < 2; }),  p.second);
    }) ==
                      p_type2{"y"} * x + p_type2{"x"});
}

struct print_tex_tester {
    template <typename Cf>
    struct runner {
        template <typename Expo>
        void operator()(const Expo &)
        {
            // Avoid the stream tests with floating-point, because of messy output.
            if (std::is_same<Cf, double>::value) {
                return;
            }
            typedef g_series_type<Cf, Expo> p_type1;
            typedef g_series_type<p_type1, Expo> p_type11;
            std::ostringstream oss;
            p_type1{}.print_tex(oss);
            CHECK(oss.str() == "0");
            oss.str("");
            p_type1{1}.print_tex(oss);
            CHECK(oss.str() == "1");
            oss.str("");
            p_type1{-1}.print_tex(oss);
            CHECK(oss.str() == "-1");
            oss.str("");
            p_type1{"x"}.print_tex(oss);
            CHECK(oss.str() == "{x}");
            oss.str("");
            (-p_type1{"x"}).print_tex(oss);
            CHECK(oss.str() == "-{x}");
            oss.str("");
            (-p_type1{"x"} * p_type1{"y"}.pow(2)).print_tex(oss);
            CHECK(oss.str() == "-{x}{y}^{2}");
            oss.str("");
            (-p_type1{"x"} + 1).print_tex(oss);
            CHECK((oss.str() == "1-{x}" || oss.str() == "-{x}+1"));
            oss.str("");
            p_type11{}.print_tex(oss);
            CHECK(oss.str() == "0");
            oss.str("");
            p_type11{"x"}.print_tex(oss);
            CHECK(oss.str() == "{x}");
            oss.str("");
            (-3 * p_type11{"x"}.pow(2)).print_tex(oss);
            CHECK(oss.str() == "-3{x}^{2}");
            oss.str("");
            (p_type11{1}).print_tex(oss);
            CHECK(oss.str() == "1");
            oss.str("");
            (p_type11{-1}).print_tex(oss);
            CHECK(oss.str() == "-1");
            oss.str("");
            (p_type11{"x"} * p_type11{"y"}).print_tex(oss);
            CHECK(oss.str() == "{x}{y}");
            oss.str("");
            (-p_type11{"x"} * p_type11{"y"}).print_tex(oss);
            CHECK(oss.str() == "-{x}{y}");
            oss.str("");
            (-p_type11{"x"} + 1).print_tex(oss);
            CHECK((oss.str() == "1-{x}" || oss.str() == "-{x}+1"));
            oss.str("");
            (p_type11{"x"} - 1).print_tex(oss);
            CHECK((oss.str() == "{x}-1" || oss.str() == "-1+{x}"));
            // Test wih less term output.
            settings::set_max_term_output(3u);
            oss.str("");
            p_type11{}.print_tex(oss);
            CHECK(oss.str() == "0");
            oss.str("");
            p_type11{"x"}.print_tex(oss);
            CHECK(oss.str() == "{x}");
            oss.str("");
            (-p_type11{"x"}).print_tex(oss);
            CHECK(oss.str() == "-{x}");
            oss.str("");
            (p_type11{1}).print_tex(oss);
            CHECK(oss.str() == "1");
            oss.str("");
            (p_type11{-1}).print_tex(oss);
            CHECK(oss.str() == "-1");
            oss.str("");
            (p_type11{"x"} * p_type11{"y"}).print_tex(oss);
            CHECK(oss.str() == "{x}{y}");
            oss.str("");
            (-p_type11{"x"} * p_type11{"y"}).print_tex(oss);
            CHECK(oss.str() == "-{x}{y}");
            // Test wih little term output.
            typedef polynomial<Cf, monomial<Expo>> poly_type;
            settings::set_max_term_output(1u);
            oss.str("");
            (-3 * poly_type{"x"} + 1 + poly_type{"x"} * poly_type{"x"}
             + poly_type{"x"} * poly_type{"x"} * poly_type{"x"})
                .print_tex(oss);
            const std::string tmp_out = oss.str(), tmp_cmp = "\\ldots";
            CHECK(std::equal(tmp_cmp.rbegin(), tmp_cmp.rend(), tmp_out.rbegin()));
            oss.str("");
            poly_type{}.print_tex(oss);
            CHECK(oss.str() == "0");
            settings::reset_max_term_output();
        }
    };
    template <typename Cf>
    void operator()(const Cf &)
    {
        boost::mpl::for_each<expo_types>(runner<Cf>());
    }
};

TEST_CASE("series_print_tex_test")
{
    boost::mpl::for_each<cf_types>(print_tex_tester());
}

struct trim_tester {
    template <typename Cf>
    struct runner {
        template <typename Expo>
        void operator()(const Expo &)
        {
            if (std::is_same<Cf, double>::value) {
                return;
            }
            typedef g_series_type<Cf, Expo> p_type1;
            typedef g_series_type<p_type1, Expo> p_type11;
            p_type1 x{"x"}, y{"y"};
            CHECK((1 + x - x).trim().get_symbol_set().size() == 0u);
            CHECK((1 + x * y - y * x + x).trim().get_symbol_set().size() == 1u);
            CHECK((1 + x * y - y * x + x + y).trim().get_symbol_set().size() == 2u);
            p_type11 xx(x), yy(y);
            CHECK(((1 + xx) - xx).begin()->first.get_symbol_set().size() == 1u);
            CHECK(((1 + xx) - xx).trim().begin()->first.get_symbol_set().size() == 0u);
            CHECK(((1 + xx * yy) - xx * yy + xx).trim().begin()->first.get_symbol_set().size() == 1u);
            CHECK(((1 + xx * yy) - xx * yy + xx + yy).trim().begin()->first.get_symbol_set().size() == 2u);
            CHECK((1 + x * xx + y * yy - x * xx).trim().begin()->first.get_symbol_set().size() == 1u);
            CHECK(
                (1 + x * p_type11{"x"} + y * p_type11{"y"} - x * p_type11{"x"}).trim().get_symbol_set().size() == 1u);
            CHECK((((1 + x).pow(5) + y) - y).trim() == (1 + x).pow(5));
        }
    };
    template <typename Cf>
    void operator()(const Cf &)
    {
        boost::mpl::for_each<expo_types>(runner<Cf>());
    }
};

TEST_CASE("series_trim_test")
{
    boost::mpl::for_each<cf_types>(trim_tester());
}

struct is_zero_tester {
    template <typename Cf>
    struct runner {
        template <typename Expo>
        void operator()(const Expo &)
        {
            typedef g_series_type<Cf, Expo> p_type1;
            typedef g_series_type<p_type1, Expo> p_type11;
            CHECK(is_is_zero_type<p_type1>::value);
            CHECK(is_is_zero_type<p_type11>::value);
            CHECK(piranha::is_zero(p_type1{}));
            CHECK(piranha::is_zero(p_type11{}));
            CHECK(piranha::is_zero(p_type1{0}));
            CHECK(piranha::is_zero(p_type11{0}));
            CHECK(!piranha::is_zero(p_type1{1}));
            CHECK(!piranha::is_zero(p_type11{1}));
        }
    };
    template <typename Cf>
    void operator()(const Cf &)
    {
        boost::mpl::for_each<expo_types>(runner<Cf>());
    }
};

TEST_CASE("series_is_zero_test")
{
    boost::mpl::for_each<cf_types>(is_zero_tester());
}

struct type_traits_tester {
    template <typename Cf>
    struct runner {
        template <typename Expo>
        void operator()(const Expo &)
        {
            typedef g_series_type<Cf, Expo> p_type1;
            typedef g_series_type<p_type1, Expo> p_type11;
            CHECK(is_series<p_type1>::value);
            CHECK(is_series<p_type11>::value);
            CHECK(!is_series<p_type1 &>::value);
            CHECK(!is_series<p_type11 const &>::value);
            CHECK(!is_series<p_type11 const>::value);
            CHECK(is_equality_comparable<const p_type1 &>::value);
            CHECK((is_equality_comparable<const p_type1 &, const Cf &>::value));
            CHECK((is_equality_comparable<const Cf &, const p_type1 &>::value));
            CHECK((!is_equality_comparable<const p_type1 &, const std::string &>::value));
            CHECK(is_equality_comparable<const p_type11 &>::value);
            CHECK((is_equality_comparable<const p_type11 &, const p_type1 &>::value));
            CHECK((is_equality_comparable<const p_type1 &, const p_type11 &>::value));
            CHECK(is_ostreamable<p_type1>::value);
            CHECK(is_ostreamable<p_type11>::value);
            CHECK(is_container_element<p_type1>::value);
            CHECK(is_container_element<p_type11>::value);
            CHECK(!is_less_than_comparable<p_type1>::value);
            CHECK((!is_less_than_comparable<p_type1, int>::value));
            CHECK(!is_less_than_comparable<p_type11>::value);
            CHECK((!is_less_than_comparable<p_type11, int>::value));
            CHECK((!is_less_than_comparable<p_type11, p_type1>::value));
            // Addition.
            CHECK(is_addable<p_type1>::value);
            CHECK((is_addable<p_type1, int>::value));
            CHECK((is_addable<int, p_type1>::value));
            CHECK(is_addable<p_type11>::value);
            CHECK((is_addable<p_type11, int>::value));
            CHECK((is_addable<int, p_type11>::value));
            CHECK((is_addable<p_type11, p_type1>::value));
            CHECK(is_addable_in_place<p_type1>::value);
            CHECK(!is_addable_in_place<p_type1 const>::value);
            CHECK((is_addable_in_place<p_type1, int>::value));
            CHECK((!is_addable_in_place<p_type1 const, int>::value));
            CHECK(is_addable_in_place<p_type11>::value);
            CHECK((is_addable_in_place<p_type11, int>::value));
            CHECK((is_addable_in_place<p_type11, p_type1>::value));
            // Subtraction.
            CHECK(is_subtractable<p_type1>::value);
            CHECK((is_subtractable<p_type1, int>::value));
            CHECK((is_subtractable<int, p_type1>::value));
            CHECK(is_subtractable<p_type11>::value);
            CHECK((is_subtractable<p_type11, int>::value));
            CHECK((is_subtractable<int, p_type11>::value));
            CHECK((is_subtractable<p_type11, p_type1>::value));
            CHECK(is_subtractable_in_place<p_type1>::value);
            CHECK(!is_subtractable_in_place<p_type1 const>::value);
            CHECK((is_subtractable_in_place<p_type1, int>::value));
            CHECK((!is_subtractable_in_place<p_type1 const, int>::value));
            CHECK(is_subtractable_in_place<p_type11>::value);
            CHECK((is_subtractable_in_place<p_type11, int>::value));
            CHECK((is_subtractable_in_place<p_type11, p_type1>::value));
            // Multiplication.
            CHECK(is_multipliable<p_type1>::value);
            CHECK((is_multipliable<p_type1, int>::value));
            CHECK((is_multipliable<int, p_type1>::value));
            CHECK(is_multipliable<p_type11>::value);
            CHECK((is_multipliable<p_type11, int>::value));
            CHECK((is_multipliable<int, p_type11>::value));
            CHECK((is_multipliable<p_type11, p_type1>::value));
            CHECK(is_multipliable_in_place<p_type1>::value);
            CHECK(!is_multipliable_in_place<p_type1 const>::value);
            CHECK((is_multipliable_in_place<p_type1, int>::value));
            CHECK((!is_multipliable_in_place<p_type1 const, int>::value));
            CHECK(is_multipliable_in_place<p_type11>::value);
            CHECK((is_multipliable_in_place<p_type11, int>::value));
            CHECK((is_multipliable_in_place<p_type11, p_type1>::value));
            // Division.
            CHECK(is_divisible<p_type1>::value);
            CHECK((is_divisible<p_type1, int>::value));
            CHECK((is_divisible<int, p_type1>::value));
            CHECK(is_divisible<p_type11>::value);
            CHECK((is_divisible<p_type11, int>::value));
            CHECK((is_divisible<int, p_type11>::value));
            CHECK((is_divisible<p_type11, p_type1>::value));
            CHECK(is_divisible_in_place<p_type1>::value);
            CHECK(!is_divisible_in_place<p_type1 const>::value);
            CHECK((is_divisible_in_place<p_type1, int>::value));
            CHECK((!is_divisible_in_place<p_type1 const, int>::value));
            CHECK(is_divisible_in_place<p_type11>::value);
            CHECK((is_divisible_in_place<p_type11, int>::value));
            CHECK((is_divisible_in_place<p_type11, p_type1>::value));
            CHECK(has_print_coefficient<p_type1>::value);
            CHECK(has_print_coefficient<p_type11>::value);
            CHECK(has_print_tex_coefficient<p_type1>::value);
            CHECK(has_print_tex_coefficient<p_type11>::value);
            CHECK((std::is_same<void, decltype(print_coefficient(*static_cast<std::ostream *>(nullptr),
                                                                       std::declval<p_type1>()))>::value));
            CHECK((std::is_same<void, decltype(print_coefficient(*static_cast<std::ostream *>(nullptr),
                                                                       std::declval<p_type11>()))>::value));
            CHECK((std::is_same<void, decltype(print_tex_coefficient(*static_cast<std::ostream *>(nullptr),
                                                                           std::declval<p_type1>()))>::value));
            CHECK((std::is_same<void, decltype(print_tex_coefficient(*static_cast<std::ostream *>(nullptr),
                                                                           std::declval<p_type11>()))>::value));
            CHECK(has_negate<p_type1>::value);
            CHECK(has_negate<p_type1 &>::value);
            CHECK(!has_negate<const p_type1 &>::value);
            CHECK(!has_negate<const p_type1>::value);
            CHECK((std::is_same<decltype(math::negate(*static_cast<p_type1 *>(nullptr))), void>::value));
            CHECK(has_negate<p_type11>::value);
            CHECK(has_negate<p_type11 &>::value);
            CHECK(!has_negate<const p_type11 &>::value);
            CHECK(!has_negate<const p_type11>::value);
            CHECK((std::is_same<decltype(math::negate(*static_cast<p_type11 *>(nullptr))), void>::value));
        }
    };
    template <typename Cf>
    void operator()(const Cf &)
    {
        boost::mpl::for_each<expo_types>(runner<Cf>());
    }
};

TEST_CASE("series_type_traits_test")
{
    boost::mpl::for_each<cf_types>(type_traits_tester());
    CHECK(!is_series<int>::value);
    CHECK(!is_series<double>::value);
    CHECK(!is_series<void>::value);
}
