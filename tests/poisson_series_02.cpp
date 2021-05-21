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
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <mp++/config.hpp>
#include <mp++/exceptions.hpp>

#include <piranha/config.hpp>
#include <piranha/detail/polynomial_fwd.hpp>
#include <piranha/divisor.hpp>
#include <piranha/divisor_series.hpp>
#include <piranha/exceptions.hpp>
#include <piranha/integer.hpp>
#include <piranha/invert.hpp>
#include <piranha/math.hpp>
#include <piranha/math/cos.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/math/sin.hpp>
#include <piranha/monomial.hpp>
#include <piranha/polynomial.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/s11n.hpp>
#include <piranha/series.hpp>

#include "catch.hpp"

using namespace piranha;

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
    mock_cf &operator/=(int);
    mock_cf operator/(const mock_cf &) const;
};

TEST_CASE("poisson_series_ipow_subs_test")
{
    typedef poisson_series<polynomial<rational, monomial<short>>> p_type1;
    {
        CHECK((has_ipow_subs<p_type1, p_type1>::value));
        CHECK((has_ipow_subs<p_type1, integer>::value));
        CHECK((has_ipow_subs<p_type1, typename p_type1::term_type::cf_type>::value));
        {
            CHECK(p_type1{"x"}.ipow_subs("x", integer(4), integer(1)) == p_type1{"x"});
            CHECK(p_type1{"x"}.ipow_subs("x", integer(1), p_type1{"x"}) == p_type1{"x"});
            p_type1 x{"x"}, y{"y"}, z{"z"};
            CHECK((x.pow(2) + x * y + z).ipow_subs("x", integer(2), integer(3)) == 3 + x * y + z);
            CHECK((x.pow(2) + x * y + z).ipow_subs("y", integer(1),rational(3, 2)) ==
                              x * x + x * rational(3, 2) + z);
            CHECK((x.pow(7) + x.pow(2) * y + z).ipow_subs("x", integer(3), x) == x.pow(3) + x.pow(2) * y + z);
            CHECK((x.pow(6) + x.pow(2) * y + z).ipow_subs("x", integer(3), p_type1{}) == x.pow(2) * y + z);
        }
#if defined(MPPP_WITH_MPFR)
        {
            typedef poisson_series<polynomial<real, monomial<short>>> p_type2;
            CHECK((has_ipow_subs<p_type2, p_type2>::value));
            CHECK((has_ipow_subs<p_type2, integer>::value));
            CHECK((has_ipow_subs<p_type2, typename p_type2::term_type::cf_type>::value));
            p_type2 x{"x"}, y{"y"};
            CHECK((x * x * x + y * y).ipow_subs("x", integer(1), real(1.234)) ==
                              y * y + piranha::pow(real(1.234), integer(3)));
            CHECK((x * x * x + y * y).ipow_subs("x", integer(3), real(1.234)) == y * y + real(1.234));
            CHECK(
                (x * x * x + y * y).ipow_subs("x", integer(2), real(1.234)).ipow_subs("y", integer(2), real(-5.678)) ==
                real(-5.678) + real(1.234) * x);
            CHECK(math::ipow_subs(x * x * x + y * y, "x", integer(1), real(1.234))
                                  .ipow_subs("y", integer(1), real(-5.678)) ==
                              piranha::pow(real(-5.678), integer(2)) + piranha::pow(real(1.234), integer(3)));
        }
#endif
        p_type1 x{"x"}, y{"y"}, z{"z"};
        CHECK(math::ipow_subs(x.pow(-7) + y + z, "x", integer(2), y) == x.pow(-7) + y + z);
        CHECK(math::ipow_subs(x.pow(-7) + y + z, "x", integer(-2), y) == x.pow(-1) * y.pow(3) + y + z);
        CHECK(math::ipow_subs(x.pow(-7) + y + z, "x", integer(-7), z) == y + 2 * z);
        CHECK(math::ipow_subs(x.pow(-7) * piranha::cos(x) + y + z, "x", integer(-4), z) ==
                          (z * x.pow(-3)) * piranha::cos(x) + y + z);
        CHECK(math::ipow_subs(x.pow(-7) * piranha::cos(x) + y + z, "x", integer(4), z) ==
                          x.pow(-7) * piranha::cos(x) + y + z);
    }
    // Try also with eps.
    {
        using eps = poisson_series<divisor_series<polynomial<rational, monomial<short>>, divisor<short>>>;
        using math::invert;
        using math::ipow_subs;
        eps x{"x"}, y{"y"}, z{"z"};
        CHECK((has_ipow_subs<eps, eps>::value));
        CHECK(ipow_subs(x, "x", 1, y) == y);
        CHECK(ipow_subs(x * x, "x", 1, y) == y * y);
        CHECK(ipow_subs(x * x * x, "x", 2, y) == x * y);
        CHECK(ipow_subs(x * x * x * invert(x), "x", 2, y) == x * y * invert(x));
        CHECK(ipow_subs(x * x * x * invert(x) * cos(z), "x", 3, y) == y * cos(z) * invert(x));
        CHECK(ipow_subs(x * x * x * invert(x) * cos(x), "x", 3, y) == y * cos(x) * invert(x));
    }
}

TEST_CASE("poisson_series_is_evaluable_test")
{
    typedef poisson_series<polynomial<rational, monomial<short>>> p_type1;
    CHECK((is_evaluable<p_type1, double>::value));
    CHECK((is_evaluable<p_type1, float>::value));
#if defined(MPPP_WITH_MPFR)
    CHECK((is_evaluable<p_type1, real>::value));
#endif
    CHECK((is_evaluable<p_type1, rational>::value));
    CHECK((is_evaluable<p_type1, integer>::value));
    CHECK((is_evaluable<p_type1, int>::value));
    CHECK((is_evaluable<p_type1, long>::value));
    CHECK((is_evaluable<p_type1, long long>::value));
    CHECK((is_evaluable<poisson_series<polynomial<mock_cf, monomial<short>>>, double>::value));
    CHECK((is_evaluable<poisson_series<mock_cf>, double>::value));
}

#if defined(PIRANHA_WITH_BOOST_S11N)

TEST_CASE("poisson_series_serialization_test")
{
    typedef poisson_series<polynomial<rational, monomial<short>>> stype;
    stype x("x"), y("y"), z = x + piranha::cos(x + y), tmp;
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

TEST_CASE("poisson_series_rebind_test")
{
    typedef poisson_series<polynomial<integer, monomial<long>>> stype;
    CHECK((series_is_rebindable<stype, double>::value));
    CHECK((series_is_rebindable<stype, rational>::value));
    CHECK((series_is_rebindable<stype, float>::value));
    CHECK((std::is_same<series_rebind<stype, polynomial<float, monomial<long>>>,
                              poisson_series<polynomial<float, monomial<long>>>>::value));
    CHECK((std::is_same<series_rebind<stype, polynomial<rational, monomial<long>>>,
                              poisson_series<polynomial<rational, monomial<long>>>>::value));
    CHECK((std::is_same<series_rebind<stype, polynomial<long double, monomial<long>>>,
                              poisson_series<polynomial<long double, monomial<long>>>>::value));
}

TEST_CASE("poisson_series_t_integrate_test")
{
    using math::invert;
    using div_type0 = divisor<short>;
    using ptype0 = polynomial<rational, monomial<short>>;
    using dtype0 = divisor_series<ptype0, div_type0>;
    using ts0 = poisson_series<dtype0>;
    ts0 x{"x"}, y{"y"}, z{"z"};
    ts0 nu_x{"\\nu_{x}"}, nu_y{"\\nu_{y}"}, nu_z{"\\nu_{z}"};
    ts0 a{"a"}, b{"b"};
    auto tmp0 = (1 / 5_q * z * piranha::sin(x + y)).t_integrate();
    CHECK((std::is_same<decltype(tmp0), ts0>::value));
    CHECK(tmp0 == -1 / 5_q * z * piranha::cos(x + y) * invert(nu_x + nu_y));
    CHECK_THROWS_AS((1 / 5_q * z * piranha::sin(x + y)).t_integrate({}), std::invalid_argument);
    tmp0 = (1 / 5_q * z * piranha::sin(x + y)).t_integrate({"a", "b"});
    CHECK(tmp0 == -1 / 5_q * z * piranha::cos(x + y) * invert(a + b));
    tmp0 = (1 / 5_q * z * piranha::sin(x + y)).t_integrate({"a", "a", "b"});
    CHECK(tmp0 == -1 / 5_q * z * piranha::cos(x + y) * invert(a + b));
    tmp0 = (1 / 5_q * z * piranha::sin(x + y)).t_integrate({"a", "b", "b"});
    CHECK(tmp0 == -1 / 5_q * z * piranha::cos(x + y) * invert(a + b));
    tmp0 = (1 / 5_q * z * piranha::sin(x + y)).t_integrate({"a", "a", "b", "b"});
    CHECK(tmp0 == -1 / 5_q * z * piranha::cos(x + y) * invert(a + b));
    CHECK_THROWS_AS((1 / 5_q * z * piranha::sin(x + y)).t_integrate({"a", "b", "c"}), std::invalid_argument);
    CHECK_THROWS_AS((1 / 5_q * z * piranha::sin(x + y)).t_integrate({"a", "b", "b", "c"}), std::invalid_argument);
    CHECK_THROWS_AS((1 / 5_q * z * piranha::sin(x + y)).t_integrate({"a", "b", "b", "c", "c"}),
                      std::invalid_argument);
    CHECK_THROWS_AS((1 / 5_q * z * piranha::sin(x + y)).t_integrate({"b", "a"}), std::invalid_argument);
    tmp0 = (1 / 5_q * z * piranha::cos(x + y)).t_integrate();
    CHECK(tmp0 == 1 / 5_q * z * piranha::sin(x + y) * invert(nu_x + nu_y));
    tmp0 = (1 / 5_q * z * piranha::cos(x + y)).t_integrate({"a", "b"});
    CHECK(tmp0 == 1 / 5_q * z * piranha::sin(x + y) * invert(a + b));
    tmp0 = (1 / 5_q * z * piranha::cos(3 * x + y)).t_integrate();
    CHECK(tmp0 == 1 / 5_q * z * piranha::sin(3 * x + y) * invert((3 * nu_x + nu_y)));
    tmp0 = (1 / 5_q * z * piranha::cos(3 * x + y)).t_integrate({"a", "b"});
    CHECK(tmp0 == 1 / 5_q * z * piranha::sin(3 * x + y) * invert((3 * a + b)));
    // Check with a common divisor.
    tmp0 = (1 / 5_q * z * piranha::cos(3 * x + 6 * y)).t_integrate();
    CHECK(tmp0 == 1 / 15_q * z * piranha::sin(3 * x + 6 * y) * invert(nu_x + 2 * nu_y));
    tmp0 = (1 / 5_q * z * piranha::cos(3 * x + 6 * y)).t_integrate({"a", "b"});
    CHECK(tmp0 == 1 / 15_q * z * piranha::sin(3 * x + 6 * y) * invert(a + 2 * b));
    // Check with a leading zero.
    // NOTE: this complication is to produce cos(6y) while avoiding x being trimmed by the linear argument
    // deduction.
    tmp0 = (1 / 5_q * z * (piranha::cos(x + 6 * y) * piranha::cos(x) - piranha::cos(2 * x + 6 * y) / 2)).t_integrate();
    CHECK(tmp0 == 1 / 60_q * z * piranha::sin(6 * y) * invert(nu_y));
    tmp0 = (1 / 5_q * z * (piranha::cos(x + 6 * y) * piranha::cos(x) - piranha::cos(2 * x + 6 * y) / 2))
               .t_integrate({"a", "b"});
    CHECK(tmp0 == 1 / 60_q * z * piranha::sin(6 * y) * invert(b));
    // Test throwing.
    CHECK_THROWS_AS(z.t_integrate(), std::invalid_argument);
    CHECK_THROWS_AS(z.t_integrate({}), std::invalid_argument);
    // An example with more terms.
    tmp0 = (1 / 5_q * z * piranha::cos(3 * x + 6 * y) - 2 * z * piranha::sin(12 * x - 9 * y)).t_integrate();
    CHECK(tmp0 == 1 / 15_q * z * piranha::sin(3 * x + 6 * y) * invert(nu_x + 2 * nu_y)
                                + 2 / 3_q * z * piranha::cos(12 * x - 9 * y) * invert(4 * nu_x - 3 * nu_y));
    tmp0 = (1 / 5_q * z * piranha::cos(3 * x + 6 * y) - 2 * z * piranha::sin(12 * x - 9 * y)).t_integrate({"a", "b"});
    CHECK(tmp0 == 1 / 15_q * z * piranha::sin(3 * x + 6 * y) * invert(a + 2 * b)
                                + 2 / 3_q * z * piranha::cos(12 * x - 9 * y) * invert(4 * a - 3 * b));
    // Test with existing divisors.
    tmp0 = 1 / 5_q * z * cos(3 * x + 6 * y) * invert(nu_x + 2 * nu_y);
    CHECK(tmp0.t_integrate() == 1 / 15_q * z * sin(3 * x + 6 * y) * piranha::pow(invert(nu_x + 2 * nu_y), 2));
    tmp0 = 1 / 5_q * z * cos(3 * x + 6 * y) * invert(nu_x + 2 * nu_y);
    CHECK(tmp0.t_integrate({"a", "b"}) ==
                      1 / 15_q * z * sin(3 * x + 6 * y) * invert(nu_x + 2 * nu_y) * invert(a + 2 * b));
    tmp0 = 1 / 5_q * z * cos(3 * x + 6 * y) * invert(nu_x + nu_y);
    CHECK(tmp0.t_integrate() ==
                      1 / 15_q * z * sin(3 * x + 6 * y) * invert(nu_x + nu_y) * invert(nu_x + 2 * nu_y));
    tmp0 = 1 / 5_q * z * cos(3 * x + 6 * y) * invert(nu_x + nu_y);
    CHECK(tmp0.t_integrate({"a", "b"}) ==
                      1 / 15_q * z * sin(3 * x + 6 * y) * invert(nu_x + nu_y) * invert(a + 2 * b));
    tmp0 = 1 / 5_q * z * cos(3 * x + 6 * y) * invert(nu_x + 2 * nu_y)
           + 1 / 3_q * z * z * sin(2 * x + 6 * y) * invert(nu_y);
    CHECK(tmp0.t_integrate() ==
                      1 / 15_q * z * sin(3 * x + 6 * y) * piranha::pow(invert(nu_x + 2 * nu_y), 2)
                          + -1 / 6_q * z * z * cos(2 * x + 6 * y) * invert(nu_y) * invert(nu_x + 3 * nu_y));
    tmp0 = 1 / 5_q * z * cos(3 * x + 6 * y) * invert(nu_x + 2 * nu_y)
           + 1 / 3_q * z * z * sin(2 * x + 6 * y) * invert(nu_y);
    CHECK(tmp0.t_integrate({"a", "b"}) ==
                      1 / 15_q * z * sin(3 * x + 6 * y) * invert(nu_x + 2 * nu_y) * invert(a + 2 * b)
                          + -1 / 6_q * z * z * cos(2 * x + 6 * y) * invert(nu_y) * invert(a + 3 * b));
    // Test derivative.
    tmp0 = (1 / 5_q * z * piranha::cos(3 * x + 6 * y) - 2 * z * piranha::sin(12 * x - 9 * y)).t_integrate();
    CHECK(tmp0.partial("z") == tmp0 * invert(ptype0{"z"}));
    CHECK(tmp0.partial("\\nu_{x}") ==
                      -1 / 15_q * z * invert(nu_x + 2 * nu_y).pow(2) * piranha::sin(3 * x + 6 * y)
                          - 8 / 3_q * z * invert(4 * nu_x - 3 * nu_y).pow(2) * piranha::cos(12 * x - 9 * y));
    CHECK(tmp0.partial("\\nu_{y}") ==
                      -2 / 15_q * z * invert(nu_x + 2 * nu_y).pow(2) * piranha::sin(3 * x + 6 * y)
                          + 2 * z * invert(4 * nu_x - 3 * nu_y).pow(2) * piranha::cos(12 * x - 9 * y));
    // Try the custom derivative with respect to the nu_x variable.
    ts0::register_custom_derivative("\\nu_{x}",
                                    [](const ts0 &s) { return s.partial("\\nu_{x}") + s.partial("x") * ts0{"t"}; });
    CHECK(math::partial(tmp0, "\\nu_{x}") ==
                      -1 / 15_q * z * invert(nu_x + 2 * nu_y).pow(2) * piranha::sin(3 * x + 6 * y)
                          + 3 / 15_q * z * invert(nu_x + 2 * nu_y) * piranha::cos(3 * x + 6 * y) * ts0{"t"}
                          - 8 / 3_q * z * invert(4 * nu_x - 3 * nu_y).pow(2) * piranha::cos(12 * x - 9 * y)
                          - 24 / 3_q * z * piranha::sin(12 * x - 9 * y) * invert(4 * nu_x - 3 * nu_y) * ts0{"t"});
    ts0::unregister_all_custom_derivatives();
}

// Test here the poly_in_cf type trait, for convenience.
TEST_CASE("poisson_series_poly_in_cf_test")
{
    CHECK((!detail::poly_in_cf<poisson_series<double>>::value));
#if defined(MPPP_WITH_MPFR)
    CHECK((!detail::poly_in_cf<poisson_series<real>>::value));
    CHECK((detail::poly_in_cf<poisson_series<polynomial<real, monomial<short>>>>::value));
#endif
    CHECK((detail::poly_in_cf<poisson_series<polynomial<rational, monomial<short>>>>::value));
#if defined(MPPP_WITH_MPFR)
    CHECK(
        (detail::poly_in_cf<poisson_series<divisor_series<polynomial<real, monomial<short>>, divisor<short>>>>::value));
    CHECK((detail::poly_in_cf<
                 poisson_series<divisor_series<polynomial<rational, monomial<short>>, divisor<short>>>>::value));
    CHECK((!detail::poly_in_cf<
                 poisson_series<divisor_series<divisor_series<real, divisor<short>>, divisor<short>>>>::value));
    CHECK((!detail::poly_in_cf<
                 poisson_series<divisor_series<divisor_series<rational, divisor<short>>, divisor<short>>>>::value));
#endif
}

TEST_CASE("poisson_series_invert_test")
{
    using pt0 = poisson_series<polynomial<integer, monomial<long>>>;
    CHECK(is_invertible<pt0>::value);
    CHECK((std::is_same<pt0, decltype(math::invert(pt0{}))>::value));
    CHECK(math::invert(pt0{1}) == 1);
    CHECK(math::invert(pt0{2}) == 0);
    CHECK_THROWS_AS(math::invert(pt0{0}), mppp::zero_division_error);
    CHECK(math::invert(pt0{"x"}) == piranha::pow(pt0{"x"}, -1));
    using pt1 = poisson_series<polynomial<rational, monomial<long>>>;
    CHECK(is_invertible<pt1>::value);
    CHECK((std::is_same<pt1, decltype(math::invert(pt1{}))>::value));
    CHECK(math::invert(pt1{1}) == 1);
    CHECK(math::invert(pt1{2}) == 1 / 2_q);
    CHECK(math::invert(2 * pt1{"y"}) == 1 / 2_q * pt1{"y"}.pow(-1));
    CHECK_THROWS_AS(math::invert(pt1{0}), mppp::zero_division_error);
    CHECK_THROWS_AS(math::invert(pt1{"x"} + pt1{"y"}), std::invalid_argument);
    using pt2 = poisson_series<polynomial<double, monomial<long>>>;
    CHECK(is_invertible<pt2>::value);
    CHECK((std::is_same<pt2, decltype(math::invert(pt2{}))>::value));
    CHECK(math::invert(pt2{1}) == 1);
    CHECK(math::invert(pt2{.2}) == piranha::pow(.2, -1));
    CHECK(math::invert(2 * pt2{"y"}) == piranha::pow(2., -1) * pt2{"y"}.pow(-1));
    CHECK_THROWS_AS(math::invert(pt2{"x"} + pt2{"y"}), std::invalid_argument);
    // A couple of checks with eps.
    using pt3 = poisson_series<divisor_series<polynomial<rational, monomial<short>>, divisor<short>>>;
    CHECK(is_invertible<pt3>::value);
    CHECK((std::is_same<pt3, decltype(math::invert(pt3{}))>::value));
    CHECK(math::invert(pt3{-1 / 3_q}) == -3);
    CHECK(boost::lexical_cast<std::string>(math::invert(pt3{"x"})) == "1/[(x)]");
    CHECK(boost::lexical_cast<std::string>(math::invert(-pt3{"x"} + pt3{"y"})) == "-1/[(x-y)]");
    CHECK(boost::lexical_cast<std::string>(piranha::pow(pt3{"x"}, -1)) == "x**-1");
    CHECK(boost::lexical_cast<std::string>(piranha::pow(pt3{"x"} * 3, -3)) == "1/27*x**-3");
}

TEST_CASE("poisson_series_truncation_test")
{
    using pt = polynomial<rational, monomial<short>>;
    using ps = poisson_series<pt>;
    {
        ps x{"x"}, y{"y"}, z{"z"};
        CHECK((has_truncate_degree<ps, int>::value));
        CHECK(math::truncate_degree(x, 1) == x);
        CHECK(math::truncate_degree(x, 0) == 0);
        CHECK(math::truncate_degree(y + x * x, 1) == y);
        CHECK(math::truncate_degree(y + x * x + z.pow(-3), 0) == z.pow(-3));
        CHECK(math::truncate_degree((y + x * x + z.pow(-3)) * piranha::cos(x), 0) ==
                          z.pow(-3) * piranha::cos(x));
        CHECK(math::truncate_degree((y + x * x + z.pow(-3)) * piranha::cos(x), 0, {"x"}) ==
                          (y + z.pow(-3)) * piranha::cos(x));
        CHECK(math::truncate_degree((y + x * x + z.pow(-3)) * piranha::cos(x), 0, {"x"}) ==
                          (y + z.pow(-3)) * piranha::cos(x));
        pt::set_auto_truncate_degree(2, {"x", "z"});
        CHECK((x * x * z).empty());
        CHECK(!(x * x * piranha::cos(x)).empty());
        pt::unset_auto_truncate_degree();
    }
    {
        using eps = poisson_series<divisor_series<pt, divisor<short>>>;
        eps x{"x"}, y{"y"}, z{"z"};
        CHECK((has_truncate_degree<eps, int>::value));
        CHECK(math::truncate_degree(x, 1) == x);
        CHECK(math::truncate_degree(x, 0) == 0);
        CHECK(math::truncate_degree(y + x * x, 1) == y);
        CHECK(math::truncate_degree(y + x * x * math::invert(x), 1) == y);
        CHECK(math::truncate_degree(y + x * x + z.pow(-3), 0) == z.pow(-3));
        CHECK(math::truncate_degree((y + x * x + z.pow(-3)) * piranha::cos(x), 0) ==
                          z.pow(-3) * piranha::cos(x));
        CHECK(math::truncate_degree((y + x * x + z.pow(-3)) * piranha::cos(x), 0, {"x"}) ==
                          (y + z.pow(-3)) * piranha::cos(x));
        CHECK(math::truncate_degree((y + x * x + z.pow(-3)) * piranha::cos(x), 0, {"x"}) ==
                          (y + z.pow(-3)) * piranha::cos(x));
        pt::set_auto_truncate_degree(2, {"x", "z"});
        CHECK((x * x * z).empty());
        CHECK(!(x * x * piranha::cos(x)).empty());
        CHECK(!(math::invert(x) * x * x * piranha::cos(x)).empty());
        pt::unset_auto_truncate_degree();
    }
}

TEST_CASE("poisson_series_multiplier_test")
{
    // Some checks for the erasing of terms after division by 2.
    {
        using ps = poisson_series<integer>;
        CHECK(ps(2) * ps(4) == 8);
    }
    {
        using ps = poisson_series<polynomial<integer, monomial<short>>>;
        ps x{"x"}, y{"y"}, z{"z"};
        CHECK(x * piranha::cos(y) * z * piranha::sin(y) == 0);
        CHECK(x * piranha::cos(y) * z * piranha::sin(y) + x * piranha::cos(z) == x * piranha::cos(z));
    }
    {
        using ps = poisson_series<polynomial<rational, monomial<short>>>;
        settings::set_min_work_per_thread(1u);
        ps x{"x"}, y{"y"}, z{"z"};
        for (unsigned nt = 1u; nt <= 4u; ++nt) {
            settings::set_n_threads(nt);
            auto res = (x * cos(x) + y * sin(x)) * (z * cos(x) + x * sin(y));
            auto cmp = -1 / 2_q * piranha::pow(x, 2) * sin(x - y) + 1 / 2_q * piranha::pow(x, 2) * sin(x + y)
                       + 1 / 2_q * y * z * sin(2 * x) + 1 / 2_q * x * y * cos(x - y) - 1 / 2_q * x * y * cos(x + y)
                       + x * z / 2 + 1 / 2_q * x * z * cos(2 * x);
            CHECK(res == cmp);
        }
        settings::reset_n_threads();
        settings::reset_min_work_per_thread();
    }
    {
        using ps = poisson_series<polynomial<integer, monomial<short>>>;
        settings::set_min_work_per_thread(1u);
        ps x{"x"}, y{"y"}, z{"z"};
        for (unsigned nt = 1u; nt <= 4u; ++nt) {
            settings::set_n_threads(nt);
            auto res = (x * cos(x) + y * sin(x)) * (z * cos(x) + x * sin(y));
            CHECK(res == 0);
        }
        settings::reset_n_threads();
        settings::reset_min_work_per_thread();
    }
}
