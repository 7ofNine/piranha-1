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

#include <piranha/poisson_series.hpp>

#include <boost/lexical_cast.hpp>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include <mp++/config.hpp>
#if defined(MPPP_WITH_MPFR)
#include <mp++/real.hpp>
#endif

#include <piranha/divisor.hpp>
#include <piranha/divisor_series.hpp>
#include <piranha/integer.hpp>
#include <piranha/invert.hpp>
#include <piranha/math.hpp>
#include <piranha/math/cos.hpp>
#include <piranha/math/degree.hpp>
#include <piranha/math/ldegree.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/math/sin.hpp>
#include <piranha/monomial.hpp>
#include <piranha/polynomial.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/series.hpp>
#include <piranha/symbol_utils.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"

struct foo {
};

using namespace piranha;

using cf_types = std::tuple<double, rational, polynomial<rational, monomial<int>>>;

struct constructor_tester {
    template <typename Cf, enable_if_t<std::is_base_of<detail::polynomial_tag, Cf>::value, int> = 0>
    void poly_ctor_test() const
    {
        typedef poisson_series<Cf> p_type;
        // Construction from symbol name.
        p_type p2{"x"};
        CHECK(p2.size() == 1u);
        CHECK(p2 == p_type{"x"});
        CHECK(p2 != p_type{std::string("y")});
        CHECK(p2 == p_type{"x"} + p_type{"y"} - p_type{"y"});
        CHECK((std::is_constructible<p_type, std::string>::value));
        CHECK((std::is_constructible<p_type, const char *>::value));
        CHECK((!std::is_constructible<p_type, foo>::value));
        CHECK((std::is_assignable<p_type, std::string>::value));
        CHECK((!std::is_assignable<p_type, foo>::value));
    }
    template <typename Cf, enable_if_t<!std::is_base_of<detail::polynomial_tag, Cf>::value, int> = 0>
    void poly_ctor_test() const
    {
        typedef poisson_series<Cf> p_type;
        if (!std::is_constructible<Cf, std::string>::value) {
            CHECK((!std::is_constructible<p_type, std::string>::value));
            CHECK((!std::is_constructible<p_type, const char *>::value));
        }
        CHECK((!std::is_constructible<p_type, foo>::value));
        CHECK((!std::is_assignable<p_type, foo>::value));
        CHECK((std::is_assignable<p_type, int>::value));
    }
    template <typename Cf>
    void operator()(const Cf &) const
    {
        typedef poisson_series<Cf> p_type;
        CHECK(is_series<p_type>::value);
        // Default construction.
        p_type p1;
        CHECK(p1 == 0);
        CHECK(p1.empty());
        poly_ctor_test<Cf>();
        // Construction from number-like entities.
        p_type p3{3};
        CHECK(p3.size() == 1u);
        CHECK(p3 == 3);
        CHECK(3 == p3);
        p_type p3a{integer(3)};
        CHECK(p3a == p3);
        CHECK(p3 == p3a);
        // Construction from poisson series of different type.
        typedef poisson_series<polynomial<rational, monomial<short>>> p_type1;
        typedef poisson_series<polynomial<integer, monomial<short>>> p_type2;
        p_type1 p4(1);
        p_type2 p5(p4);
        CHECK(p4 == p5);
        CHECK(p5 == p4);
        p_type1 p6("x");
        p_type2 p7(std::string("x"));
        p_type2 p8("y");
        CHECK(p6 == p7);
        CHECK(p7 == p6);
        CHECK(p6 != p8);
        CHECK(p8 != p6);
    }
};

TEST_CASE("poisson_series_constructors_test")
{
#if defined(MPPP_WITH_MPFR)
    //mppp::real_set_default_prec(100); Do we really need this. is there a mppp::real in cf_types??
#endif
    tuple_for_each(cf_types{}, constructor_tester());
}

struct assignment_tester {
    template <typename Cf, enable_if_t<std::is_base_of<detail::polynomial_tag, Cf>::value, int> = 0>
    void poly_assignment_test() const
    {
        typedef poisson_series<Cf> p_type;
        p_type p1;
        p1 = "x";
        CHECK(p1 == p_type("x"));
    }
    template <typename Cf, enable_if_t<!std::is_base_of<detail::polynomial_tag, Cf>::value, int> = 0>
    void poly_assignment_test() const
    {
    }
    template <typename Cf>
    void operator()(const Cf &) const
    {
        typedef poisson_series<Cf> p_type;
        p_type p1;
        p1 = 1;
        CHECK(p1 == 1);
        p1 = integer(10);
        CHECK(p1 == integer(10));
        poly_assignment_test<Cf>();
    }
};

TEST_CASE("poisson_series_assignment_test")
{
    tuple_for_each(cf_types{}, assignment_tester());
}

TEST_CASE("poisson_series_stream_test")
{
    typedef poisson_series<integer> p_type1;
    CHECK(boost::lexical_cast<std::string>(p_type1{}) == "0");
    CHECK(boost::lexical_cast<std::string>(p_type1{1}) == "1");
    CHECK(boost::lexical_cast<std::string>(p_type1{1} - 3) == "-2");
    typedef poisson_series<rational> p_type2;
    CHECK(boost::lexical_cast<std::string>(p_type2{}) == "0");
    CHECK(boost::lexical_cast<std::string>(p_type2{rational(1, 2)}) == "1/2");
#if defined(MPPP_WITH_MPFR)
    CHECK(boost::lexical_cast<std::string>(p_type2{real("-0.5", 32)}) == "-1/2");
#endif
    typedef poisson_series<polynomial<rational, monomial<short>>> p_type3;
    CHECK(boost::lexical_cast<std::string>(p_type3{}) == "0");
    CHECK(boost::lexical_cast<std::string>(p_type3{"x"}) == "x");
    CHECK(boost::lexical_cast<std::string>(rational(3, -2) * p_type3{"x"}) == "-3/2*x");
    CHECK(boost::lexical_cast<std::string>(rational(3, -2) * p_type3{"x"}.pow(2)) == "-3/2*x**2");
}

TEST_CASE("poisson_series_sin_cos_test")
{
    typedef poisson_series<polynomial<rational, monomial<short>>> p_type1;
    p_type1 p1{"x"};
    CHECK((std::is_same<p_type1, decltype(piranha::sin(p_type1{}))>::value));
    CHECK((std::is_same<p_type1, decltype(piranha::cos(p_type1{}))>::value));
    CHECK(boost::lexical_cast<std::string>(piranha::sin(-p1)) == "-sin(x)");
    CHECK(boost::lexical_cast<std::string>(piranha::cos(p1)) == "cos(x)");
    CHECK(boost::lexical_cast<std::string>(p1.sin()) == "sin(x)");
    CHECK(boost::lexical_cast<std::string>((-p1).cos()) == "cos(x)");
    p1 = 0;
    CHECK(piranha::sin(-p1) == 0);
    CHECK(piranha::cos(p1) == 1);
    p1 = p_type1{"x"} - 2 * p_type1{"y"};
    CHECK(boost::lexical_cast<std::string>(piranha::sin(-p1)) == "-sin(x-2*y)");
    CHECK(boost::lexical_cast<std::string>(piranha::cos(-p1)) == "cos(x-2*y)");
    CHECK(boost::lexical_cast<std::string>(3 * p1.sin()) == "3*sin(x-2*y)");
    CHECK(boost::lexical_cast<std::string>(p1.cos()) == "cos(x-2*y)");
    p1 = p_type1{"x"} * p_type1{"y"};
    CHECK_THROWS_AS(piranha::sin(p1), std::invalid_argument);
    CHECK_THROWS_AS(piranha::cos(p1), std::invalid_argument);
    CHECK_THROWS_AS(piranha::sin(p_type1{"x"} + 1), std::invalid_argument);
    CHECK_THROWS_AS(piranha::cos(p_type1{"x"} - 1), std::invalid_argument);
    CHECK_THROWS_AS(piranha::sin(p_type1{"x"} * rational(1, 2)), std::invalid_argument);
    CHECK_THROWS_AS(piranha::cos(p_type1{"x"} * rational(1, 2)), std::invalid_argument);
    CHECK(boost::lexical_cast<std::string>(piranha::sin(p_type1{"x"} * rational(4, -2))) == "-sin(2*x)");
    CHECK(boost::lexical_cast<std::string>(-piranha::cos(p_type1{"x"} * rational(4, 2))) == "-cos(2*x)");
#if defined(MPPP_WITH_MPFR)
    typedef poisson_series<polynomial<real, monomial<short>>> p_type2;
    CHECK((std::is_same<p_type2, decltype(piranha::sin(p_type2{}))>::value));
    CHECK((std::is_same<p_type2, decltype(piranha::cos(p_type2{}))>::value));
    CHECK(piranha::sin(p_type2{3}) == piranha::sin(real(3)));
    CHECK(piranha::cos(p_type2{3}) == piranha::cos(real(3)));
    p_type2 p2 = p_type2{"x"} - 2 * p_type2{"y"};
    CHECK(boost::lexical_cast<std::string>(piranha::sin(-p2)) ==                                  
                      "-1.0000000000*sin(x-2*y)");  // mppp real precision is now 32bits (53 bits = 15 digits ? are specified??
    CHECK(boost::lexical_cast<std::string>(piranha::cos(-p2)) ==
                       "1.0000000000*cos(x-2*y)");                            
    CHECK_THROWS_AS(piranha::sin(p_type2{"x"} * real(rational(1, 2))), std::invalid_argument);
    CHECK_THROWS_AS(piranha::cos(p_type2{"x"} * real(rational(1, 2))), std::invalid_argument);
    typedef poisson_series<real> p_type3;
    CHECK(piranha::sin(p_type3{3}) == piranha::sin(real(3)));
    CHECK(piranha::cos(p_type3{3}) == piranha::cos(real(3)));
#endif
    typedef poisson_series<double> p_type4;
    CHECK((std::is_same<p_type4, decltype(piranha::sin(p_type4{}))>::value));
    CHECK((std::is_same<p_type4, decltype(piranha::cos(p_type4{}))>::value));
    CHECK(piranha::sin(p_type4{0}) == 0);
    CHECK(piranha::cos(p_type4{0}) == std::cos(0));
    CHECK(piranha::cos(p_type4{1}) == std::cos(1));
    CHECK(piranha::sin(p_type4{1}) == std::sin(1));
    // Type traits checks.
    CHECK(is_sine_type<p_type4>::value);
    CHECK(is_cosine_type<p_type4>::value);
#if defined(MPPP_WITH_MPFR)
    CHECK(is_sine_type<p_type3>::value);
    CHECK(is_cosine_type<p_type3>::value);
#endif
    CHECK(is_sine_type<p_type1>::value);
    CHECK(is_cosine_type<p_type1>::value);
    CHECK(is_sine_type<poisson_series<rational>>::value);
    CHECK(is_cosine_type<poisson_series<rational>>::value);
    // Check with eps.
    using p_type5 = poisson_series<divisor_series<polynomial<rational, monomial<short>>, divisor<short>>>;
    CHECK((std::is_same<p_type5, decltype(piranha::sin(p_type5{}))>::value));
    CHECK((std::is_same<p_type5, decltype(piranha::cos(p_type5{}))>::value));
    CHECK(is_sine_type<p_type5>::value);
    CHECK(is_cosine_type<p_type5>::value);
    CHECK(boost::lexical_cast<std::string>(piranha::cos(p_type5{"x"})) == "cos(x)");
    CHECK(boost::lexical_cast<std::string>(piranha::cos(p_type5{"x"} + p_type5{"y"})) == "cos(x+y)");
    CHECK(boost::lexical_cast<std::string>(piranha::cos(-p_type5{"x"} + p_type5{"y"})) == "cos(x-y)");
    CHECK(boost::lexical_cast<std::string>(piranha::sin(p_type5{"x"})) == "sin(x)");
    CHECK(boost::lexical_cast<std::string>(piranha::sin(p_type5{"x"} + p_type5{"y"})) == "sin(x+y)");
    CHECK(boost::lexical_cast<std::string>(piranha::sin(-p_type5{"x"} + p_type5{"y"})) == "-sin(x-y)");
    CHECK(piranha::cos(p_type5{0}) == 1);
    CHECK(piranha::sin(p_type5{0}) == 0);
    using p_type6 = poisson_series<divisor_series<polynomial<double, monomial<short>>, divisor<short>>>;
    CHECK(piranha::cos(p_type6{1.23}) == std::cos(1.23));
    CHECK(piranha::sin(p_type6{-4.56}) == std::sin(-4.56));
    // Double divisor.
    using p_type7 = poisson_series<
        divisor_series<divisor_series<polynomial<rational, monomial<short>>, divisor<short>>, divisor<short>>>;
    CHECK(is_sine_type<p_type7>::value);
    CHECK(is_cosine_type<p_type7>::value);
    CHECK((std::is_same<p_type7, decltype(piranha::sin(p_type7{}))>::value));
    CHECK((std::is_same<p_type7, decltype(piranha::cos(p_type7{}))>::value));
    CHECK(boost::lexical_cast<std::string>(piranha::cos(p_type7{"x"})) == "cos(x)");
    CHECK(boost::lexical_cast<std::string>(piranha::cos(p_type7{"x"} + p_type7{"y"})) == "cos(x+y)");
    CHECK(boost::lexical_cast<std::string>(piranha::cos(-p_type7{"x"} + p_type7{"y"})) == "cos(x-y)");
    CHECK(boost::lexical_cast<std::string>(piranha::sin(p_type7{"x"})) == "sin(x)");
    CHECK(boost::lexical_cast<std::string>(piranha::sin(p_type7{"x"} + p_type7{"y"})) == "sin(x+y)");
    CHECK(boost::lexical_cast<std::string>(piranha::sin(-p_type7{"x"} + p_type7{"y"})) == "-sin(x-y)");
    CHECK(piranha::cos(p_type7{0}) == 1);
    CHECK(piranha::sin(p_type7{0}) == 0);
}

TEST_CASE("poisson_series_arithmetic_test")
{
    // Just some random arithmetic tests using known trigonometric identities.
    typedef poisson_series<polynomial<rational, monomial<short>>> p_type1;
    p_type1 x{"x"}, y{"y"};
    CHECK(piranha::cos(x) * piranha::cos(y) == (piranha::cos(x - y) + piranha::cos(x + y)) / 2);
    CHECK(piranha::cos(-x) * piranha::cos(y) == (piranha::cos(x - y) + piranha::cos(x + y)) / 2);
    CHECK(piranha::cos(x) * piranha::cos(-y) == (piranha::cos(x - y) + piranha::cos(x + y)) / 2);
    CHECK(piranha::cos(-x) * piranha::cos(-y) == (piranha::cos(x - y) + piranha::cos(x + y)) / 2);
    CHECK(piranha::sin(x) * piranha::sin(y) == (piranha::cos(x - y) - piranha::cos(x + y)) / 2);
    CHECK(piranha::sin(-x) * piranha::sin(y) == -(piranha::cos(x - y) - piranha::cos(x + y)) / 2);
    CHECK(piranha::sin(x) * piranha::sin(-y) == -(piranha::cos(x - y) - piranha::cos(x + y)) / 2);
    CHECK(piranha::sin(-x) * piranha::sin(-y) == (piranha::cos(x - y) - piranha::cos(x + y)) / 2);
    CHECK(piranha::sin(x) * piranha::cos(y) == (piranha::sin(x + y) + piranha::sin(x - y)) / 2);
    CHECK(piranha::sin(-x) * piranha::cos(y) == -(piranha::sin(x + y) + piranha::sin(x - y)) / 2);
    CHECK(piranha::sin(x) * piranha::cos(-y) == (piranha::sin(x + y) + piranha::sin(x - y)) / 2);
    CHECK(piranha::sin(-x) * piranha::cos(-y) == -(piranha::sin(x + y) + piranha::sin(x - y)) / 2);
    CHECK(piranha::cos(x) * piranha::sin(y) == (piranha::sin(x + y) - piranha::sin(x - y)) / 2);
    CHECK(piranha::cos(-x) * piranha::sin(y) == (piranha::sin(x + y) - piranha::sin(x - y)) / 2);
    CHECK(piranha::cos(x) * piranha::sin(-y) == -(piranha::sin(x + y) - piranha::sin(x - y)) / 2);
    CHECK(piranha::cos(-x) * piranha::sin(-y) == -(piranha::sin(x + y) - piranha::sin(x - y)) / 2);
    CHECK(piranha::pow(piranha::sin(x), 5) ==
                      (10 * piranha::sin(x) - 5 * piranha::sin(3 * x) + piranha::sin(5 * x)) / 16);
    CHECK(piranha::pow(piranha::cos(x), 5) ==
                      (10 * piranha::cos(x) + 5 * piranha::cos(3 * x) + piranha::cos(5 * x)) / 16);
    CHECK(piranha::pow(piranha::cos(x), 5) * piranha::pow(piranha::sin(x), 5) ==
                      (10 * piranha::sin(2 * x) - 5 * piranha::sin(6 * x) + piranha::sin(10 * x)) / 512);
    CHECK(piranha::pow(p_type1{rational(1, 2)}, 5) == piranha::pow(rational(1, 2), 5));
#if defined(MPPP_WITH_MPFR)
    // NOTE: these won't work until we specialise safe_cast for real, due
    // to the new monomial pow() requirements.
    typedef poisson_series<polynomial<real, monomial<short>>> p_type2;
    CHECK(piranha::pow(p_type2(real("1.234", 100)), real("-5.678", 100)) ==
                      piranha::pow(real("1.234", 100), real("-5.678", 100)));
    CHECK(piranha::sin(p_type2(real("1.234", 100)))== piranha::sin(real("1.234", 100)));
    CHECK(piranha::cos(p_type2(real("1.234", 100))) == piranha::cos(real("1.234", 100)));
    typedef poisson_series<real> p_type3;
    CHECK(piranha::sin(p_type3(real("1.234", 100))) == piranha::sin(real("1.234", 100)));
    CHECK(piranha::cos(p_type3(real("1.234", 100))) == piranha::cos(real("1.234", 100)));
#endif
}

TEST_CASE("poisson_series_degree_test")
{
    {
        typedef poisson_series<polynomial<rational, monomial<short>>> p_type1;
        CHECK(is_degree_type<p_type1>::value);
        CHECK(is_ldegree_type<p_type1>::value);
        CHECK(piranha::degree(p_type1{}) == 0);
        CHECK(piranha::degree(p_type1{"x"}) == 1);
        CHECK(piranha::degree(p_type1{"x"} + 1) == 1);
        CHECK(piranha::degree(p_type1{"x"}.pow(2) + 1) == 2);
        CHECK(piranha::degree(p_type1{"x"} * p_type1{"y"} + 1) == 2);
        CHECK(piranha::degree(p_type1{"x"} * p_type1{"y"} + 1, {"x"}) == 1);
        CHECK(piranha::degree(p_type1{"x"} * p_type1{"y"} + 1, {"x", "y"}) == 2);
        CHECK(piranha::degree(p_type1{"x"} * p_type1{"y"} + 1, {"z"}) == 0);
        CHECK(piranha::ldegree(p_type1{"x"} + 1) == 0);
        CHECK(piranha::ldegree(p_type1{"x"} * p_type1{"y"} + p_type1{"x"}, {"x", "y"}) == 1);
        CHECK(piranha::ldegree(p_type1{"x"} * p_type1{"y"} + p_type1{"x"}, {"x"}) == 1);
        CHECK(piranha::ldegree(p_type1{"x"} * p_type1{"y"} + p_type1{"x"}, {"y"}) == 0);
        p_type1 x{"x"}, y{"y"};
        CHECK(piranha::degree(piranha::pow(x, 2) * piranha::cos(y) + 1) == 2);
        CHECK(piranha::ldegree(piranha::pow(x, 2) * piranha::cos(y) + 1) == 0);
        CHECK(piranha::ldegree((x * y + y) * piranha::cos(y) + 1, {"x"}) == 0);
        CHECK(piranha::ldegree((x * y + y) * piranha::cos(y) + 1, {"y"}) == 0);
        CHECK(piranha::ldegree((x * y + y) * piranha::cos(y) + y, {"y"}) == 1);
        CHECK(piranha::ldegree((x * y + y) * piranha::cos(y) + y, {"x"}) == 0);
        CHECK(piranha::ldegree((x * y + y) * piranha::cos(y) + y) == 1);
        CHECK(piranha::ldegree((x * y + y) * piranha::cos(y) + y, {"x", "y"}) == 1);
        CHECK(piranha::ldegree((x * y + y) * piranha::cos(y) + 1, {"x", "y"}) == 0);
        typedef poisson_series<rational> p_type2;
        CHECK(!is_degree_type<p_type2>::value);
        CHECK(!is_ldegree_type<p_type2>::value);
    }
    // Try also with eps.
    {
        using eps = poisson_series<divisor_series<polynomial<rational, monomial<short>>, divisor<short>>>;
        using math::invert;
        using piranha::degree;
        using piranha::ldegree;
        eps x{"x"}, y{"y"}, z{"z"};
        CHECK(is_degree_type<eps>::value);
        CHECK(is_ldegree_type<eps>::value);
        CHECK(degree(x) == 1);
        CHECK(degree(x * y + z) == 2);
        CHECK(ldegree(x * y + z) == 1);
        // Divisors don't count in the computation of the degree.
        CHECK(degree(invert(x)) == 0);
        CHECK(degree(invert(x) * x + y * x * z) == 3);
        CHECK(ldegree(invert(x)) == 0);
        CHECK(ldegree(invert(x) * x + y * x * z) == 1);
        CHECK(ldegree((invert(x) * x + y * x * z) * cos(x) + cos(y)) == 0);
    }
}
