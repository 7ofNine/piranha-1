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

#include <piranha/divisor_series.hpp>

#include <boost/lexical_cast.hpp>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include <mp++/config.hpp>
#include <mp++/exceptions.hpp>

#include <piranha/divisor.hpp>
#include <piranha/exceptions.hpp>
#include <piranha/integer.hpp>
#include <piranha/invert.hpp>
#include <piranha/kronecker_monomial.hpp>
#include <piranha/math.hpp>
#include <piranha/math/cos.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/math/sin.hpp>
#include <piranha/monomial.hpp>
#include <piranha/poisson_series.hpp>
#include <piranha/polynomial.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/symbol_utils.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

using cf_types = std::tuple<double, integer,
#if defined(MPPP_WITH_MPFR)
                            real,
#endif
                            rational, polynomial<rational, monomial<int>>>;
using expo_types = std::tuple<short, int, long, integer>;

struct test_00_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using s_type = divisor_series<T, divisor<short>>;
        s_type s0{3};
        // Just test some math operations and common functionalities.
        CHECK(s0 + s0 == 6);
        CHECK(s0 * s0 == 9);
        CHECK(s0 * 4 == 12);
        CHECK(4 * s0 == 12);
        CHECK(piranha::pow(s0, 3) == 27);
        CHECK(piranha::cos(s_type{0}) == 1);
        CHECK(piranha::sin(s_type{0}) == 0);
        CHECK(math::evaluate<int>(piranha::pow(s0, 3), {{"x", 4}}) == 27);
        CHECK(is_differentiable<s_type>::value);
        CHECK(s_type{1}.partial("x") == 0);
        if (std::is_base_of<detail::polynomial_tag, T>::value) {
            CHECK((has_subs<s_type, s_type>::value));
            CHECK((has_subs<s_type, int>::value));
            CHECK((has_subs<s_type, integer>::value));
        }
        CHECK((!has_subs<s_type, std::string>::value));
        if (std::is_base_of<detail::polynomial_tag, T>::value) {
            CHECK((has_ipow_subs<s_type, s_type>::value));
            CHECK((has_ipow_subs<s_type, int>::value));
            CHECK((has_ipow_subs<s_type, integer>::value));
        }
        CHECK((!has_ipow_subs<s_type, std::string>::value));
    }
};

TEST_CASE("divisor_series_test_00")
{
    tuple_for_each(cf_types{}, test_00_tester());
}
struct partial_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using p_type = polynomial<rational, monomial<int>>;
        using s_type = divisor_series<p_type, divisor<T>>;
        s_type x{"x"}, y{"y"}, z{"z"};
        // First with variables only in the divisors.
        auto s0 = math::invert(x + y - 2 * z);
        CHECK((std::is_same<s_type, decltype(s0.partial("x"))>::value));
        CHECK((std::is_same<s_type, decltype(math::partial(s0, "x"))>::value));
        CHECK(s0.partial("x") == -s0 * s0);
        CHECK(math::partial(s0, "x") == -s0 * s0);
        CHECK(s0.partial("z") == 2 * s0 * s0);
        auto s1 = s0 * s0;
        CHECK(s1.partial("x") == -2 * s0 * s1);
        CHECK(s1.partial("z") == 4 * s0 * s1);
        auto s2 = math::invert(x - y);
        auto s3 = s0 * s2;
        CHECK(s3.partial("x") == -s0 * s0 * s2 - s0 * s2 * s2);
        auto s4 = math::invert(x);
        auto s5 = s0 * s2 * s4;
        CHECK(s5.partial("x") == -s0 * s0 * s2 * s4 - s0 * s2 * s2 * s4 - s0 * s2 * s4 * s4);
        CHECK(s5.partial("z") == 2 * s0 * s0 * s2 * s4);
        auto s6 = s0 * s0 * s2 * s4;
        CHECK(s6.partial("x") ==
                          -2 * s0 * s0 * s0 * s2 * s4 - s0 * s0 * s2 * s2 * s4 - s0 * s0 * s2 * s4 * s4);
        // Variables only in the coefficients.
        auto s7 = s2 * s4 * (x * x / 5 + y - 3 * z);
        CHECK(s7.partial("z") == s2 * s4 * -3);
        auto s8 = s2 * s4 * (x * x / 5 + y - 3 * z) + z * s2 * s4 * y;
        CHECK(s8.partial("z") == s2 * s4 * -3 + s2 * s4 * y);
        CHECK((x * x * math::invert(z)).partial("x") == 2 * x * math::invert(z));
        // This excercises the presense of an additional divisor variable with a zero multiplier.
        CHECK((x * x * math::invert(z) + s4 - s4).partial("x") == 2 * x * math::invert(z));
        // Variables both in the coefficients and in the divisors.
        auto s9 = x * s2;
        CHECK(s9.partial("x") == s2 - x * s2 * s2);
        CHECK(math::partial(s9, "x") == s2 - x * s2 * s2);
        auto s10 = x * s2 * s4;
        CHECK(s10.partial("x") == s2 * s4 + x * (-s2 * s2 * s4 - s2 * s4 * s4));
        auto s11 = math::invert(-3 * x - y);
        auto s12 = math::invert(z);
        auto s13 = x * s11 * s4 + x * y * z * s2 * s2 * s2 * s12;
        CHECK(s13.partial("x") == s11 * s4 + x * (3 * s11 * s11 * s4 - s11 * s4 * s4)
                                                + y * z * s2 * s2 * s2 * s12
                                                + x * y * z * (-3 * s2 * s2 * s2 * s2 * s12));
        CHECK(math::partial(s13, "x") == s11 * s4 + x * (3 * s11 * s11 * s4 - s11 * s4 * s4)
                                                       + y * z * s2 * s2 * s2 * s12
                                                       + x * y * z * (-3 * s2 * s2 * s2 * s2 * s12));
        auto s15 = x * s11 * s4 + x * y * z * s2 * s2 * s2 * s12 + s4 * s12;
        CHECK(s15.partial("x") == s11 * s4 + x * (3 * s11 * s11 * s4 - s11 * s4 * s4)
                                                + y * z * s2 * s2 * s2 * s12
                                                + x * y * z * (-3 * s2 * s2 * s2 * s2 * s12) - s4 * s4 * s12);
        // Overflow in an exponent.
        overflow_check<T>();
        auto s16 = math::invert(x - 4 * y);
        auto s17 = s2 * s2 * s2 * s2 * s2 * s16 * s16 * s16 * s12;
        CHECK(s17.partial("x") == -5 * s2 * s2 * s2 * s2 * s2 * s2 * s16 * s16 * s16 * s12
                                                - 3 * s2 * s2 * s2 * s2 * s2 * s16 * s16 * s16 * s16 * s12);
        // Excercise the chain rule.
        auto s18 = x * x * 3 / 4_q * y * z * z;
        auto s19 = -y * y * x * z * z;
        auto s20 = y * x * x * 4;
        auto s21 = s18 * s17 + s19 * s2 * s11 * s12 + s20 * s16 * s2 * s3;
        CHECK(s21.partial("x") == s18.partial("x") * s17 + s18 * s17.partial("x")
                                                + s19.partial("x") * s2 * s11 * s12
                                                + s19 * (s2 * s11 * s12).partial("x") + s20.partial("x") * s16 * s2 * s3
                                                + s20 * (s16 * s2 * s3).partial("x"));
        CHECK(s21.partial("y") == s18.partial("y") * s17 + s18 * s17.partial("y")
                                                + s19.partial("y") * s2 * s11 * s12
                                                + s19 * (s2 * s11 * s12).partial("y") + s20.partial("y") * s16 * s2 * s3
                                                + s20 * (s16 * s2 * s3).partial("y"));
        CHECK(s21.partial("z") == s18.partial("z") * s17 + s18 * s17.partial("z")
                                                + s19.partial("z") * s2 * s11 * s12
                                                + s19 * (s2 * s11 * s12).partial("z") + s20.partial("z") * s16 * s2 * s3
                                                + s20 * (s16 * s2 * s3).partial("z"));
        CHECK(s21.partial("v") == 0);
        CHECK(s_type{1}.partial("x") == 0);
    }
    template <typename T, typename U = T, typename std::enable_if<std::is_integral<U>::value, int>::type = 0>
    static void overflow_check()
    {
        using p_type = polynomial<rational, monomial<int>>;
        using s_type = divisor_series<p_type, divisor<T>>;
        s_type s14;
        s14.set_symbol_set(symbol_fset{"x"});
        typename s_type::term_type::key_type k0;
        std::vector<T> vs;
        vs.push_back(T(1));
        T expo = std::numeric_limits<T>::max();
        k0.insert(vs.begin(), vs.end(), expo);
        s14.insert(typename s_type::term_type(typename s_type::term_type::cf_type(1), k0));
        CHECK_THROWS_AS(s14.partial("x"), std::overflow_error);
        // Skip this overflow test if T is short, as short * short will become int * int and it will
        // not overflow.
        if (std::is_same<T, short>::value) {
            return;
        }
        s_type s15;
        s15.set_symbol_set(symbol_fset{"x", "y"});
        vs[0] = static_cast<T>(std::numeric_limits<T>::max() / T(4));
        vs.push_back(T(1));
        expo = static_cast<T>(std::numeric_limits<T>::max() - 1);
        typename s_type::term_type::key_type k1;
        k1.insert(vs.begin(), vs.end(), expo);
        s15.insert(typename s_type::term_type(typename s_type::term_type::cf_type(1), k1));
        CHECK_THROWS_AS(s15.partial("x"), std::overflow_error);
    }
    template <typename T, typename U = T, typename std::enable_if<!std::is_integral<U>::value, int>::type = 0>
    static void overflow_check()
    {
    }
};

TEST_CASE("divisor_series_partial_test")
{
    // A couple of general tests to start.
    using p_type = polynomial<rational, monomial<int>>;
    using s_type = divisor_series<p_type, divisor<short>>;
    {
        CHECK(s_type{}.partial("x") == 0);
        s_type s0{3};
        CHECK(s0.partial("x") == 0);
        s_type x{"x"};
        CHECK((x * 3).partial("x") == 3);
        CHECK((x * 3).partial("y") == 0);
        // Define an EPS.
        using ps_type = poisson_series<s_type>;
        ps_type a{"a"}, b{"b"}, c{"c"};
        auto p1 = 3 * a * b * piranha::cos(3 * c);
        CHECK(boost::lexical_cast<std::string>(p1.t_integrate()) == "a*b*1/[(\\nu_{c})]*sin(3*c)");
        CHECK(boost::lexical_cast<std::string>(p1.t_integrate().partial("a")) == "b*1/[(\\nu_{c})]*sin(3*c)");
        CHECK(boost::lexical_cast<std::string>(p1.t_integrate().partial("b")) == "a*1/[(\\nu_{c})]*sin(3*c)");
        CHECK(boost::lexical_cast<std::string>(p1.t_integrate().partial("c")) ==
                          "3*a*b*1/[(\\nu_{c})]*cos(3*c)");
        CHECK(boost::lexical_cast<std::string>(p1.t_integrate().partial("\\nu_{c}")) ==
                          "-a*b*1/[(\\nu_{c})**2]*sin(3*c)");
    }
    // Test with various exponent types.
    tuple_for_each(expo_types{}, partial_tester{});
    // Test custom derivatives.
    s_type x{"x"}, y{"y"};
    s_type::register_custom_derivative(
        "x", [x](const s_type &s) -> s_type { return s.partial("x") + math::partial(s, "y") * 2 * x; });
    CHECK(math::partial(math::invert(x + y), "x") == (-1 - 2 * x) * math::invert(x + y).pow(2));
    s_type::register_custom_derivative(
        "x", [y](const s_type &s) -> s_type { return s.partial("x") + math::partial(s, "y") * math::invert(y) / 2; });
    CHECK(math::partial(math::invert(x + 2 * y),"x") ==
                      (-1 - 1 * y.invert()) * math::invert(x + 2 * y).pow(2));
    s_type::register_custom_derivative(
        "x", [y](const s_type &s) -> s_type { return s.partial("x") + math::partial(s, "y") * math::invert(y) / 2; });
    CHECK(math::partial(math::invert(x + y),"x") ==
                      -math::invert(x + y).pow(2) - 1 / 2_q * math::invert(x + y).pow(2) * math::invert(y));
    // Implicit variable dependency both in the poly and in the divisor.
    s_type::register_custom_derivative(
        "x", [x](const s_type &s) -> s_type { return s.partial("x") + math::partial(s, "y") * 2 * x; });
    CHECK(math::partial(y * math::invert(x + y), "x") ==
                      2 * x * math::invert(x + y) - y * (2 * x + 1) * math::invert(x + y).pow(2));
}

TEST_CASE("divisor_series_integrate_test")
{
    using s_type = divisor_series<polynomial<rational, monomial<short>>, divisor<short>>;
    s_type x{"x"}, y{"y"}, z{"z"};
    CHECK((is_integrable<s_type>::value));
    // A few cases with the variables only in the polynomial part.
    CHECK(x.integrate("x") == x * x / 2);
    CHECK(math::integrate(x, "x") == x * x / 2);
    CHECK((std::is_same<s_type, decltype(math::integrate(x, "x"))>::value));
    CHECK(math::integrate(x, "y") == x * y);
    CHECK(math::integrate(x + y, "x") == x * y + x * x / 2);
    CHECK(math::integrate(x + y, "y") == x * y + y * y / 2);
    CHECK(math::integrate(s_type{1}, "y") == y);
    CHECK(math::integrate(s_type{1}, "x") == x);
    CHECK(math::integrate(s_type{0}, "x") == 0);
    // Put variables in the divisors as well.
    CHECK(math::integrate(x + y.invert(), "x") == x * x / 2 + x * y.invert());
    CHECK_THROWS_AS(math::integrate(x + y.invert() + x.invert(), "x") , std::invalid_argument);
    CHECK(math::integrate(x + y.invert() + x.invert() - x.invert(), "x") == x * x / 2 + x * y.invert());
}

TEST_CASE("divisor_series_invert_test")
{
    using s_type0 = divisor_series<int, divisor<short>>;
    CHECK(math::invert(s_type0{2}) == 0);
    using s_type1 = divisor_series<rational, divisor<short>>;
    CHECK(math::invert(s_type1{2}) == 1 / 2_q);
    CHECK(math::invert(s_type1{2 / 3_q}) == 3 / 2_q);
    {
        using s_type = divisor_series<polynomial<rational, monomial<short>>, divisor<short>>;
        s_type x{"x"}, y{"y"}, z{"z"}, null;
        CHECK(is_invertible<s_type>::value);
        CHECK((std::is_same<s_type, decltype(math::invert(s_type{}))>::value));
        CHECK(boost::lexical_cast<std::string>(math::invert(x)) == "1/[(x)]");
        CHECK(math::invert(s_type{2}) == 1 / 2_q);
        CHECK(boost::lexical_cast<std::string>(piranha::pow(x, -1)) == "x**-1");
        CHECK_THROWS_AS(math::invert(null), mppp::zero_division_error);
        CHECK((std::is_same<decltype(x.invert()), s_type>::value));
        CHECK((std::is_same<decltype(math::invert(x)), s_type>::value));
        CHECK(boost::lexical_cast<std::string>(math::invert(x - y)) == "1/[(x-y)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(2 * x - 4 * y)) == "1/2*1/[(x-2*y)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(-2 * x + 4 * y)) == "-1/2*1/[(x-2*y)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(x + y + z)) == "1/[(x+y+z)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(x + y + z - z)) == "1/[(x+y)]");
        CHECK_THROWS_AS(math::invert(x - 1), std::invalid_argument);
        CHECK_THROWS_AS(math::invert(x - y / 2), std::invalid_argument);
        CHECK_THROWS_AS(math::invert(x - x), mppp::zero_division_error);
        // Out of bounds for short.
        CHECK_THROWS_AS(math::invert((std::numeric_limits<short>::max() + 1_q) * x + y), std::invalid_argument);
        // Check, if appropriate, construction from outside the bounds defined in divisor.
        if (safe_abs_sint<short>::value < std::numeric_limits<short>::max()) {
            CHECK_THROWS_AS(math::invert((safe_abs_sint<short>::value + 1_q) * x + y), std::invalid_argument);
        }
        if (-safe_abs_sint<short>::value > std::numeric_limits<short>::min()) {
            CHECK_THROWS_AS(math::invert((-safe_abs_sint<short>::value - 1_q) * x + y), std::invalid_argument);
        }
    }
    {
        using s_type = divisor_series<polynomial<rational, k_monomial>, divisor<short>>;
        s_type x{"x"}, y{"y"}, z{"z"}, null;
        CHECK(is_invertible<s_type>::value);
        CHECK((std::is_same<s_type, decltype(math::invert(s_type{}))>::value));
        CHECK(boost::lexical_cast<std::string>(math::invert(x)) == "1/[(x)]");
        CHECK(math::invert(s_type{2}) == 1 / 2_q);
        CHECK(boost::lexical_cast<std::string>(piranha::pow(x, -1)) == "x**-1");
        CHECK_THROWS_AS(math::invert(null), mppp::zero_division_error);
        CHECK((std::is_same<decltype(x.invert()), s_type>::value));
        CHECK((std::is_same<decltype(math::invert(x)), s_type>::value));
        CHECK(boost::lexical_cast<std::string>(math::invert(x - y)) == "1/[(x-y)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(2 * x - 4 * y)) == "1/2*1/[(x-2*y)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(-2 * x + 4 * y)) == "-1/2*1/[(x-2*y)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(x + y + z)) == "1/[(x+y+z)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(x + y + z - z)) == "1/[(x+y)]");
        CHECK_THROWS_AS(math::invert(x - 1), std::invalid_argument);
        CHECK_THROWS_AS(math::invert(x - y / 2), std::invalid_argument);
        CHECK_THROWS_AS(math::invert(x - x), mppp::zero_division_error);
        // Out of bounds for short.
        CHECK_THROWS_AS(math::invert((std::numeric_limits<short>::max() + 1_q) * x + y), std::invalid_argument);
        // Check, if appropriate, construction from outside the bounds defined in divisor.
        if (safe_abs_sint<short>::value < std::numeric_limits<short>::max()) {
            CHECK_THROWS_AS(math::invert((safe_abs_sint<short>::value + 1_q) * x + y), std::invalid_argument);
        }
        if (-safe_abs_sint<short>::value > std::numeric_limits<short>::min()) {
            CHECK_THROWS_AS(math::invert((-safe_abs_sint<short>::value - 1_q) * x + y), std::invalid_argument);
        }
    }
    {
        using s_type = divisor_series<polynomial<rational, monomial<rational>>, divisor<short>>;
        s_type x{"x"}, y{"y"}, z{"z"}, null;
        CHECK(is_invertible<s_type>::value);
        CHECK((std::is_same<s_type, decltype(math::invert(s_type{}))>::value));
        CHECK(boost::lexical_cast<std::string>(math::invert(x)) == "1/[(x)]");
        CHECK(math::invert(s_type{2}) == 1 / 2_q);
        CHECK(boost::lexical_cast<std::string>(piranha::pow(x, -1)) == "x**-1");
        CHECK_THROWS_AS(math::invert(null), mppp::zero_division_error);
        CHECK((std::is_same<decltype(x.invert()), s_type>::value));
        CHECK((std::is_same<decltype(math::invert(x)), s_type>::value));
        CHECK(boost::lexical_cast<std::string>(math::invert(x - y))== "1/[(x-y)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(2 * x - 4 * y)) == "1/2*1/[(x-2*y)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(-2 * x + 4 * y)) == "-1/2*1/[(x-2*y)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(x + y + z)) == "1/[(x+y+z)]");
        CHECK(boost::lexical_cast<std::string>(math::invert(x + y + z - z)) == "1/[(x+y)]");
        CHECK_THROWS_AS(math::invert(x - 1), std::invalid_argument);
        CHECK_THROWS_AS(math::invert(x - y / 2), std::invalid_argument);
        CHECK_THROWS_AS(math::invert(x - x), mppp::zero_division_error);
        // Out of bounds for short.
        CHECK_THROWS_AS(math::invert((std::numeric_limits<short>::max() + 1_q) * x + y), std::invalid_argument);
        // Check, if appropriate, construction from outside the bounds defined in divisor.
        if (safe_abs_sint<short>::value < std::numeric_limits<short>::max()) {
            CHECK_THROWS_AS(math::invert((safe_abs_sint<short>::value + 1_q) * x + y), std::invalid_argument);
        }
        if (-safe_abs_sint<short>::value > std::numeric_limits<short>::min()) {
            CHECK_THROWS_AS(math::invert((-safe_abs_sint<short>::value - 1_q) * x + y), std::invalid_argument);
        }
    }
    {
        // Try with something else between poly and divisor.
        using s_type = divisor_series<poisson_series<polynomial<rational, monomial<short>>>, divisor<short>>;
        CHECK(is_invertible<s_type>::value);
        CHECK((std::is_same<s_type, decltype(math::invert(s_type{}))>::value));
        s_type x{"x"}, y{"y"}, null;
        CHECK(boost::lexical_cast<std::string>(piranha::pow(2 * x, -1)) == "1/2*x**-1");
        CHECK(boost::lexical_cast<std::string>(math::invert(2 * x)) == "1/2*1/[(x)]");
        CHECK_THROWS_AS(math::invert(piranha::cos(2 * x)), std::invalid_argument);
        CHECK_THROWS_AS(piranha::pow(x + y, -1), std::invalid_argument);
        CHECK(boost::lexical_cast<std::string>(math::invert(-2 * x + 4 * y)) == "-1/2*1/[(x-2*y)]");
        CHECK_THROWS_AS(math::invert(null), mppp::zero_division_error);
        CHECK_THROWS_AS(piranha::pow(null, -1), mppp::zero_division_error);
    }
}

TEST_CASE("divisor_series_rational_multiplication_test")
{
    // Test that we handle correctly rational coefficients wrt the lcm computation in the multiplier.
    using s_type = divisor_series<rational, divisor<short>>;
    s_type s1{1 / 2_q}, s2{2 / 3_q};
    CHECK(s1 * s2 == 1 / 3_q);
}
