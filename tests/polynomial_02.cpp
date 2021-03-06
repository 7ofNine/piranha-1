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

#include <piranha/polynomial.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <limits>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <mp++/config.hpp>
#include <mp++/exceptions.hpp>
#if defined(MPPP_WITH_MPFR)
#include <mp++/real.hpp>
#endif

#include <piranha/base_series_multiplier.hpp>
#include <piranha/config.hpp>
#include <piranha/exceptions.hpp>
#include <piranha/forwarding.hpp>
#include <piranha/integer.hpp>
#include <piranha/invert.hpp>
#include <piranha/key_is_multipliable.hpp>
#include <piranha/math.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/monomial.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/s11n.hpp>
#include <piranha/series.hpp>
#include <piranha/series_multiplier.hpp>
#include <piranha/settings.hpp>
#include <piranha/symbol_utils.hpp>

#include "catch.hpp"

using namespace piranha;

typedef boost::mpl::vector<double, integer, rational> cf_types;

template <typename Cf, typename Expo>
class polynomial_alt : public series<Cf, monomial<Expo>, polynomial_alt<Cf, Expo>>
{
    typedef series<Cf, monomial<Expo>, polynomial_alt<Cf, Expo>> base;

public:
    polynomial_alt() = default;
    polynomial_alt(const polynomial_alt &) = default;
    polynomial_alt(polynomial_alt &&) = default;
    explicit polynomial_alt(const char *name) : base()
    {
        typedef typename base::term_type term_type;
        // Insert the symbol.
        this->m_symbol_set = symbol_fset{name};
        // Construct and insert the term.
        this->insert(term_type(Cf(1), typename term_type::key_type{Expo(1)}));
    }
    PIRANHA_FORWARDING_CTOR(polynomial_alt, base)
    ~polynomial_alt() = default;
    polynomial_alt &operator=(const polynomial_alt &) = default;
    polynomial_alt &operator=(polynomial_alt &&) = default;
    PIRANHA_FORWARDING_ASSIGNMENT(polynomial_alt, base)
};

namespace piranha
{

template <typename Cf, typename Expo>
class series_multiplier<polynomial_alt<Cf, Expo>, void> : public base_series_multiplier<polynomial_alt<Cf, Expo>>
{
    using base = base_series_multiplier<polynomial_alt<Cf, Expo>>;
    template <typename T>
    using call_enabler = typename std::enable_if<
        key_is_multipliable<typename T::term_type::cf_type, typename T::term_type::key_type>::value, int>::type;

public:
    using base::base;
    template <typename T = polynomial_alt<Cf, Expo>, call_enabler<T> = 0>
    polynomial_alt<Cf, Expo> operator()() const
    {
        return this->plain_multiplication();
    }
};
}

TEST_CASE("polynomial_integrate_test")
{
#if defined(MPPP_WITH_MPFR)
    //mppp::real_set_default_prec(100); // is that really needed?? is there mppp::real used anywhere below???
#endif
    // Simple echelon-1 polynomial.
    typedef polynomial<rational, monomial<short>> p_type1;
    CHECK(is_integrable<p_type1>::value);
    CHECK(is_integrable<p_type1 &>::value);
    CHECK(is_integrable<const p_type1>::value);
    CHECK(is_integrable<p_type1 const &>::value);
    p_type1 x("x"), y("y"), z("z");
    CHECK(p_type1{}.integrate("x") == p_type1{});
    CHECK(x.integrate("x") == x * x / 2);
    CHECK(y.integrate("x") == x * y);
    CHECK((x + 3 * y * x * x + z * y * x / 4).integrate("x") ==
                      x * x / 2 + y * x * x * x + z * y * x * x / 8);
    CHECK_THROWS_AS(x.pow(-1).integrate("x"), std::invalid_argument);
    CHECK((x + 3 * y * x * x + z * y * x / 4).integrate("x").partial("x") ==
                      x + 3 * y * x * x + z * y * x / 4);
    CHECK((x + 3 * y * x * x + z * y * x / 4).integrate("y").partial("y") ==
                      x + 3 * y * x * x + z * y * x / 4);
    CHECK((x + 3 * y * x * x + z * y * x / 4).integrate("z").partial("z") ==
                      x + 3 * y * x * x + z * y * x / 4);
    CHECK(p_type1{4}.integrate("z") == 4 * z);
    CHECK((x * y * z).pow(-5).integrate("x") == (y * z).pow(-5) * x.pow(-4) * rational(1, -4));
    // Polynomial with polynomial coefficient, no variable mixing.
    typedef polynomial<p_type1, monomial<short>> p_type11;
    CHECK(is_integrable<p_type11>::value);
    CHECK(is_integrable<p_type11 &>::value);
    CHECK(is_integrable<const p_type11>::value);
    CHECK(is_integrable<p_type11 const &>::value);
    p_type11 a("a"), b("b"), c("c");
    CHECK((a * x).integrate("x") == a * x * x / 2);
    CHECK((a * x).integrate("a") == a * a * x / 2);
    CHECK((a * x * x + b * x / 15 - c * x * y).integrate("x") ==
                      a * x * x * x / 3 + b * x * x / 30 - c * x * x * y / 2);
    CHECK((a * ((x * x).pow(-1)) + b * x / 15 - a * y).integrate("x") ==
                      -a * (x).pow(-1) + b * x * x / 30 - a * x * y);
    CHECK_THROWS_AS((a * (x).pow(-1) + b * x / 15 - a * y).integrate("x"), std::invalid_argument);
    CHECK((a * x * x + b * x / 15 - a * y).integrate("a") ==
                      a * a * x * x / 2 + a * b * x / 15 - a * a * y / 2);
    CHECK(math::integrate(a * x * x + b * x / 15 - a * y, "a") ==
                      a * a * x * x / 2 + a * b * x / 15 - a * a * y / 2);
    CHECK((7 * x * a.pow(-2) + b * x / 15 - a * y).integrate("a") ==
                      -7 * x * a.pow(-1) + a * b * x / 15 - a * a * y / 2);
    CHECK((7 * x * a.pow(-2) - a * y + b * x / 15).integrate("a") ==
                      -7 * x * a.pow(-1) + a * b * x / 15 - a * a * y / 2);
    CHECK(math::integrate(x.pow(4) * y * a.pow(4) + x * y * b, "x") ==
                      x.pow(5) * y * a.pow(4) / 5 + x * x / 2 * y * b);
    // Variable mixing (integration by parts).
    p_type11 xx("x"), yy("y"), zz("z");
    CHECK((x * xx).integrate("x") == x * x * xx / 2 - math::integrate(x * x / 2, "x"));
    CHECK(((3 * x + y) * xx).integrate("x") ==
                      (3 * x * x + 2 * x * y) * xx / 2 - math::integrate((3 * x * x + 2 * x * y) / 2 , "x"));
    CHECK((x * xx * xx).integrate("x") ==
                      x * x * xx * xx / 2 - 2 * xx * x * x * x / 6 + 2 * x * x * x * x / 24);
    CHECK(math::partial((x * xx * xx).integrate("x"), "x") == x * xx * xx);
    CHECK_THROWS_AS((x.pow(-1) * xx * xx).integrate("x"), std::invalid_argument);
    CHECK_THROWS_AS((x.pow(-2) * xx * xx).integrate("x"), std::invalid_argument);
    CHECK_THROWS_AS((x.pow(-3) * xx * xx).integrate("x"), std::invalid_argument);
    CHECK((x.pow(-4) * xx * xx).integrate("x") ==
                      -x.pow(-3) / 3 * xx * xx - x.pow(-2) * 2 * xx / 6 - 2 * x.pow(-1) / 6);
    CHECK((x.pow(-4) * xx).integrate("x") == -x.pow(-3) / 3 * xx - x.pow(-2) / 6);
    CHECK((y * x.pow(-4) * xx * xx).integrate("x") ==
                      y * (-x.pow(-3) / 3 * xx * xx - x.pow(-2) * 2 * xx / 6 - 2 * x.pow(-1) / 6));
    CHECK(((y + z.pow(2) * y) * x.pow(-4) * xx * xx).integrate("x") ==
                      (y + z.pow(2) * y) * (-x.pow(-3) / 3 * xx * xx - x.pow(-2) * 2 * xx / 6 - 2 * x.pow(-1) / 6));
    CHECK(((y + z.pow(2) * y) * x.pow(-4) * xx * xx - x.pow(-4) * xx).integrate("x") ==
                      (y + z.pow(2) * y) * (-x.pow(-3) / 3 * xx * xx - x.pow(-2) * 2 * xx / 6 - 2 * x.pow(-1) / 6)
                          - (-x.pow(-3) / 3 * xx - x.pow(-2) / 6));
    // Misc tests.
    CHECK(math::partial((x + y + z).pow(10).integrate("x"), "x") == (x + y + z).pow(10));
    CHECK(math::partial((x + y + z).pow(10).integrate("y"), "y") == (x + y + z).pow(10));
    CHECK(math::partial((x + y + z).pow(10).integrate("z"), "z") == (x + y + z).pow(10));
    CHECK_THROWS_AS((x * xx.pow(-1)).integrate("x"), std::invalid_argument);
    CHECK((x * xx.pow(-1)).integrate("y") == x * xx.pow(-1) * yy);
    CHECK_THROWS_AS((x * yy.pow(-1)).integrate("y"), std::invalid_argument);
    CHECK((x * yy.pow(-2)).integrate("y") == -x * yy.pow(-1));
    // Non-integrable coefficient.
    typedef polynomial<polynomial_alt<rational, int>, monomial<int>> p_type_alt;
    p_type_alt n("n"), m("m");
    CHECK(math::integrate(n * m + m, "n") == n * n * m / 2 + m * n);
    CHECK(math::integrate(n * m + m, "m") == m * n * m / 2 + m * m / 2);
    CHECK_THROWS_AS(math::integrate(p_type_alt{polynomial_alt<rational, int>{"m"}}, "m"), std::invalid_argument);
    CHECK(math::integrate(p_type_alt{polynomial_alt<rational, int>{"n"}}, "m") ==
                      (polynomial_alt<rational, int>{"n"} * m));
    CHECK(math::integrate(p_type_alt{polynomial_alt<rational, int>{"m"}}, "n") ==
                      (polynomial_alt<rational, int>{"m"} * n));
    // Check with rational exponents and the new type-deduction logic.
    using p_type2 = polynomial<integer, monomial<rational>>;
    using p_type3 = polynomial<int, monomial<rational>>;
    CHECK(is_integrable<p_type2>::value);
    CHECK(is_integrable<p_type3>::value);
    CHECK((std::is_same<decltype(p_type2{}.integrate("x")), polynomial<rational, monomial<rational>>>::value));
    CHECK((std::is_same<decltype(p_type3{}.integrate("x")), polynomial<rational, monomial<rational>>>::value));
    CHECK(
        (std::is_same<decltype(math::integrate(p_type2{}, "x")), polynomial<rational, monomial<rational>>>::value));
    CHECK(
        (std::is_same<decltype(math::integrate(p_type3{}, "x")), polynomial<rational, monomial<rational>>>::value));
    CHECK(math::integrate(p_type2{"x"}.pow(3 / 4_q), "x") == 4 / 7_q * p_type2{"x"}.pow(7 / 4_q));
    CHECK(math::integrate(3 * p_type3{"x"}.pow(3 / 4_q), "x") == 12 / 7_q * p_type3{"x"}.pow(7 / 4_q));
}

TEST_CASE("polynomial_ipow_subs_test")
{
    typedef polynomial<rational, monomial<int>> p_type1;
    CHECK((has_ipow_subs<p_type1, p_type1>::value));
    CHECK((has_ipow_subs<p_type1, integer>::value));
    {
        CHECK(p_type1{"x"}.ipow_subs("x", integer(4), integer(1)) == p_type1{"x"});
        CHECK(p_type1{"x"}.ipow_subs("x", integer(1), p_type1{"x"}) == p_type1{"x"});
        p_type1 x{"x"}, y{"y"}, z{"z"};
        CHECK((x.pow(2) + x * y + z).ipow_subs("x", integer(2), integer(3)) == 3 + x * y + z);
        CHECK((x.pow(2) + x * y + z).ipow_subs("y", integer(1), rational(3, 2)) ==
                          x * x + x * rational(3, 2) + z);
        CHECK((x.pow(7) + x.pow(2) * y + z).ipow_subs("x", integer(3), x) == x.pow(3) + x.pow(2) * y + z);
        CHECK((x.pow(6) + x.pow(2) * y + z).ipow_subs("x", integer(3), p_type1{}) == x.pow(2) * y + z);
        CHECK((1 + 3 * x.pow(2) - 5 * y.pow(5))
                              .pow(10)
                              .ipow_subs("x", integer(2), p_type1{"x2"})
                              .template subs<p_type1>({{"x2", x.pow(2)}}) ==
                          (1 + 3 * x.pow(2) - 5 * y.pow(5)).pow(10));
        // Check with negative powers.
        CHECK(x.pow(-5).ipow_subs("x", -2, 5) == x.pow(-1) * 25);
        CHECK(x.pow(-5).ipow_subs("y", -2, 5) == x.pow(-5));
        CHECK((x.pow(-5) * y * z).ipow_subs("x", -4, 5) == x.pow(-1) * 5 * z * y);
    }
#if defined(MPPP_WITH_MPFR)
    {
        typedef polynomial<real, monomial<int>> p_type2;                                     //TODO:: all these tests are probably sensitive to comaprison failures 
        CHECK((has_ipow_subs<p_type2, p_type2>::value));                                     // mppp had the handling of precision changed after version 0.13
        CHECK((has_ipow_subs<p_type2, integer>::value));
        p_type2 x{"x"}, y{"y"};
        CHECK((x * x * x + y * y).ipow_subs("x", integer(1), real(1.234, 113)) ==                 //TODO:: fails for float different precision causes differetn output.
                          y * y + piranha::pow(real(1.234, 113), 3));
        CHECK((x * x * x + y * y).ipow_subs("x", integer(3), real(1.234)) == y * y + real(1.234, 113));
        CHECK(
            (x * x * x + y * y).ipow_subs("x", integer(2), real(1.234)).ipow_subs("y", integer(2), real(-5.678, 113)) ==
            real(-5.678, 113) + real(1.234, 113) * x);
        CHECK(                                                                                //TODO:: fails for float different precision causes different output.
            math::ipow_subs(x * x * x + y * y, "x", integer(1), real(1.234, 113)).ipow_subs("y", integer(1), real(-5.678, 113)) ==
            piranha::pow(real(-5.678, 113), 2) + piranha::pow(real(1.234, 113), 3));
    }
#endif
    {
        typedef polynomial<integer, monomial<long>> p_type3;
        CHECK((has_ipow_subs<p_type3, p_type3>::value));
        CHECK((has_ipow_subs<p_type3, integer>::value));
        p_type3 x{"x"}, y{"y"}, z{"z"};
        CHECK(math::ipow_subs(x.pow(-7) + y + z, "x", integer(2), y) == x.pow(-7) + y + z);
        CHECK(math::ipow_subs(x.pow(-7) + y + z, "x", integer(-2), y) == x.pow(-1) * y.pow(3) + y + z);
        CHECK(math::ipow_subs(x.pow(-7) + y + z, "x", integer(-7), z) == y + 2 * z);
    }
    {
        // Some tests with rational exponents.
        typedef polynomial<rational, monomial<rational>> p_type4;
        CHECK((has_ipow_subs<p_type4, p_type4>::value));
        CHECK((has_ipow_subs<p_type4, integer>::value));
        p_type4 x{"x"}, y{"y"}, z{"z"};
        // CHECK(x * y * 2 * z.pow(7 / 3_q).ipow_subs("z", 2, 4) == 4 * z.pow(1 / 3_q) * y * 2 * x);
        // CHECK(x * y * 2 * z.pow(-7 / 3_q).ipow_subs("z", -1, 4) == 16 * z.pow(-1 / 3_q) * y * 2 * x);
    }
}

#if defined(PIRANHA_WITH_BOOST_S11N)

TEST_CASE("polynomial_serialization_test")
{
    typedef polynomial<integer, monomial<long>> stype;
    stype x("x"), y("y"), z = x + y, tmp;
    std::stringstream ss;
    {
        boost::archive::text_oarchive oa(ss);
        oa << z;
    }
    {
        boost::archive::text_iarchive ia(ss);
        ia >> tmp;
    }
    CHECK(z == tmp);
}

#endif

TEST_CASE("polynomial_rebind_test")
{
    typedef polynomial<integer, monomial<long>> stype;
    CHECK((series_is_rebindable<stype, double>::value));
    CHECK((series_is_rebindable<stype, rational>::value));
    CHECK((series_is_rebindable<stype, float>::value));
    CHECK((std::is_same<series_rebind<stype, float>, polynomial<float, monomial<long>>>::value));
    CHECK((std::is_same<series_rebind<stype, rational>, polynomial<rational, monomial<long>>>::value));
    CHECK((std::is_same<series_rebind<stype, long double>, polynomial<long double, monomial<long>>>::value));
}

TEST_CASE("polynomial_invert_test")
{
    using pt0 = polynomial<integer, monomial<long>>;
    CHECK(is_invertible<pt0>::value);
    CHECK((std::is_same<pt0, decltype(math::invert(pt0{}))>::value));
    CHECK(math::invert(pt0{1}) == 1);
    CHECK(math::invert(pt0{2}) == 0);
    CHECK_THROWS_AS(math::invert(pt0{0}), mppp::zero_division_error);
    CHECK(math::invert(pt0{"x"}) == piranha::pow(pt0{"x"}, -1));
    using pt1 = polynomial<rational, monomial<long>>;
    CHECK(is_invertible<pt1>::value);
    CHECK((std::is_same<pt1, decltype(math::invert(pt1{}))>::value));
    CHECK(math::invert(pt1{1}) == 1);
    CHECK(math::invert(pt1{2}) == 1 / 2_q);
    CHECK(math::invert(2 * pt1{"y"}) == 1 / 2_q * pt1{"y"}.pow(-1));
    CHECK_THROWS_AS(math::invert(pt1{0}), mppp::zero_division_error);
    CHECK_THROWS_AS(math::invert(pt1{"x"} + pt1{"y"}), std::invalid_argument);
}

TEST_CASE("polynomial_find_cf_test")
{
    using pt1 = polynomial<integer, k_monomial>;
    CHECK(pt1{}.find_cf<int>({}) == 0);
    CHECK_THROWS_AS(pt1{}.find_cf({1}), std::invalid_argument);
    CHECK(3 * pt1{"x"}.find_cf({1}) == 3);
    CHECK(3 * pt1{"x"}.find_cf({0}) == 0);
    CHECK(3 * pt1{"x"}.find_cf({2}) == 0);
    CHECK_THROWS_AS((3 * pt1{"x"} + 4 * pt1{"y"}).find_cf({2}), std::invalid_argument);
    CHECK((3 * pt1{"x"} + 4 * pt1{"y"}).find_cf({1, 0}) == 3);
    CHECK((3 * pt1{"x"} + 4 * pt1{"y"}).find_cf({0, 1}) == 4);
    CHECK((3 * pt1{"x"} + 4 * pt1{"y"}).find_cf({1_z, 1_z}) == 0);
    CHECK((3 * pt1{"x"} + 4 * pt1{"y"}).find_cf(std::vector<integer>{1_z, 1_z}) == 0);
    CHECK((3 * pt1{"x"} + 4 * pt1{"y"}).find_cf(std::list<int>{0, 1}) == 4);
    using pt2 = polynomial<integer, monomial<int>>;
    CHECK(pt2{}.find_cf<int>({}) == 0);
    CHECK_THROWS_AS(pt2{}.find_cf({1}), std::invalid_argument);
    CHECK(3 * pt2{"x"}.find_cf({1}) == 3);
    CHECK(3 * pt2{"x"}.find_cf({0}) == 0);
    CHECK(3 * pt2{"x"}.find_cf({2}) == 0);
    CHECK_THROWS_AS((3 * pt2{"x"} + 4 * pt2{"y"}).find_cf({2}), std::invalid_argument);
    CHECK((3 * pt2{"x"} + 4 * pt2{"y"}).find_cf({1, 0}) == 3);
    CHECK((3 * pt2{"x"} + 4 * pt2{"y"}).find_cf({0, 1}) == 4);
    CHECK((3 * pt2{"x"} + 4 * pt2{"y"}).find_cf({1_z, 1_z}) == 0);
    CHECK((3 * pt2{"x"} + 4 * pt2{"y"}).find_cf(std::vector<integer>{1_z, 1_z}) == 0);
    CHECK((3 * pt2{"x"} + 4 * pt2{"y"}).find_cf(std::list<int>{0, 1}) == 4);
    CHECK((3 * pt2{"x"} + 4 * pt2{"y"}).find_cf(std::list<signed char>{0, 1}) == 4);
    if (std::numeric_limits<long>::max() > std::numeric_limits<int>::max()) {
        CHECK_THROWS_AS(pt2{"x"}.find_cf(std::list<long>{std::numeric_limits<long>::max()}), std::invalid_argument);
    }
}
