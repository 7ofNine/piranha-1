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

#include <limits>

#include <mp++/config.hpp>
#if defined(MPPP_WITH_MPFR)
#include <mp++/real.hpp>
#endif

#include <piranha/integer.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/monomial.hpp>
#include <piranha/polynomial.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/type_traits.hpp>

#include "catch.hpp"

using namespace piranha;

TEST_CASE("series_zero_is_absorbing_test")
{
#if defined(MPPP_WITH_MPFR)
//    mppp::real_set_default_prec(100);// Is that used anywhere. is there a mppp::real in use????
#endif
    {
        using pt1 = polynomial<double, monomial<int>>;
        using pt2 = polynomial<pt1, monomial<int>>;
        if (std::numeric_limits<double>::has_quiet_NaN || std::numeric_limits<double>::has_signaling_NaN) {
            CHECK((!zero_is_absorbing<pt1>::value));
            CHECK((!zero_is_absorbing<pt1 &>::value));
            CHECK((!zero_is_absorbing<const pt1 &>::value));
            CHECK((!zero_is_absorbing<const pt1>::value));
            CHECK((!zero_is_absorbing<pt1 &&>::value));
            CHECK((!zero_is_absorbing<pt2>::value));
            CHECK((!zero_is_absorbing<pt2 &>::value));
            CHECK((!zero_is_absorbing<const pt2 &>::value));
            CHECK((!zero_is_absorbing<const pt2>::value));
            CHECK((!zero_is_absorbing<pt2 &&>::value));
        }
    }
#if defined(MPPP_WITH_MPFR)
    {
        using pt1 = polynomial<real, monomial<int>>;
        using pt2 = polynomial<pt1, monomial<int>>;
        CHECK((!zero_is_absorbing<pt1>::value));
        CHECK((!zero_is_absorbing<pt1 &>::value));
        CHECK((!zero_is_absorbing<const pt1 &>::value));
        CHECK((!zero_is_absorbing<const pt1>::value));
        CHECK((!zero_is_absorbing<pt1 &&>::value));
        CHECK((!zero_is_absorbing<pt2>::value));
        CHECK((!zero_is_absorbing<pt2 &>::value));
        CHECK((!zero_is_absorbing<const pt2 &>::value));
        CHECK((!zero_is_absorbing<const pt2>::value));
        CHECK((!zero_is_absorbing<pt2 &&>::value));
    }
#endif
}

TEST_CASE("series_fp_coefficient_test")
{
    {
        using pt1 = polynomial<double, monomial<int>>;
        pt1 x{"x"};
        if (std::numeric_limits<double>::is_iec559) {
            CHECK((pt1(0.) * pt1(std::numeric_limits<double>::infinity())).size() == 1u);
            CHECK((pt1(0.) * pt1(std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((0. * pt1(std::numeric_limits<double>::infinity())).size() == 1u);
            CHECK((0. * pt1(std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((pt1(std::numeric_limits<double>::infinity()) * 0.).size() == 1u);
            CHECK((pt1(std::numeric_limits<double>::quiet_NaN()) * 0.).size() == 1u);
            CHECK((pt1(0.) * std::numeric_limits<double>::infinity()).size() == 1u);
            CHECK((pt1(0.) * std::numeric_limits<double>::quiet_NaN()).size() == 1u);
            CHECK((std::numeric_limits<double>::infinity() * pt1(0.)).size() == 1u);
            CHECK((std::numeric_limits<double>::quiet_NaN() * pt1(0.)).size() == 1u);
            CHECK((pt1(0.) * pt1(-std::numeric_limits<double>::infinity())).size() == 1u);
            CHECK((pt1(0.) * pt1(-std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((0. * pt1(-std::numeric_limits<double>::infinity())).size() == 1u);
            CHECK((0. * pt1(-std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((pt1(-std::numeric_limits<double>::infinity()) * 0.).size() == 1u);
            CHECK((pt1(-std::numeric_limits<double>::quiet_NaN()) * 0.).size() == 1u);
            CHECK((pt1(0.) * -std::numeric_limits<double>::infinity()).size() == 1u);
            CHECK((pt1(0.) * -std::numeric_limits<double>::quiet_NaN()).size() == 1u);
            CHECK((-std::numeric_limits<double>::infinity() * pt1(0.)).size() == 1u);
            CHECK((-std::numeric_limits<double>::quiet_NaN() * pt1(0.)).size() == 1u);
            CHECK((pt1(0.) * pt1(0.)).size() == 0u);
            CHECK((pt1(0.) * pt1(1.)).size() == 0u);
            CHECK((pt1(1.) * pt1(0.)).size() == 0u);
            CHECK((pt1(0.) * (pt1(std::numeric_limits<double>::infinity()) + x)).size() == 1u);
            CHECK((pt1(0.) * (pt1(std::numeric_limits<double>::quiet_NaN()) + x)).size() == 1u);
            CHECK((0. * (pt1(std::numeric_limits<double>::infinity()) - x)).size() == 1u);
            CHECK((0. * (pt1(std::numeric_limits<double>::quiet_NaN()) - x)).size() == 1u);
            CHECK(((pt1(std::numeric_limits<double>::infinity()) + x) * 0.).size() == 1u);
            CHECK(((pt1(std::numeric_limits<double>::quiet_NaN()) + x) * 0.).size() == 1u);
            CHECK((pt1(0.) * (pt1(-std::numeric_limits<double>::infinity()) + x)).size() == 1u);
            CHECK((pt1(0.) * (pt1(-std::numeric_limits<double>::quiet_NaN()) + x)).size() == 1u);
            CHECK((0. * (pt1(-std::numeric_limits<double>::infinity()) - x)).size() == 1u);
            CHECK((0. * (pt1(-std::numeric_limits<double>::quiet_NaN()) - x)).size() == 1u);
            CHECK(((pt1(-std::numeric_limits<double>::infinity()) + x) * 0.).size() == 1u);
            CHECK(((pt1(-std::numeric_limits<double>::quiet_NaN()) + x) * 0.).size() == 1u);
            CHECK((pt1(0.) / pt1(0.)).size() == 1u);
            CHECK((pt1(0.) / pt1(std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((pt1(0.) / 0.).size() == 1u);
            CHECK((pt1(0.) / std::numeric_limits<double>::quiet_NaN()).size() == 1u);
            CHECK((pt1(std::numeric_limits<double>::quiet_NaN()) / pt1(0.)).size() == 1u);
            CHECK((0. / pt1(0.)).size() == 1u);
            CHECK((std::numeric_limits<double>::quiet_NaN() / pt1(0.)).size() == 1u);
            CHECK((pt1(1.) / pt1(0.)).size() == 1u);
            CHECK((pt1(1.) / 0.).size() == 1u);
            CHECK((1. / pt1(0.)).size() == 1u);
            pt1 tmp(0);
            tmp /= 0.;
            CHECK((tmp.size() == 1u));
            tmp = 0.;
            tmp /= pt1(0.);
            CHECK((tmp.size() == 1u));
            tmp = 1.;
            tmp /= pt1(0.);
            CHECK((tmp.size() == 1u));
            tmp = 1.;
            tmp /= 0.;
            CHECK((tmp.size() == 1u));
            CHECK(piranha::pow(pt1(0.), std::numeric_limits<double>::quiet_NaN()).size() == 1u);
            CHECK(piranha::pow(pt1(0.), -1).size() == 1u);
        }
    }
#if defined(MPPP_WITH_MPFR)
    {
        using pt2 = polynomial<real, monomial<int>>;
        pt2 x{"x"};
        CHECK((pt2(0.) * pt2(real{"inf",100})).size() == 1u);
        CHECK((pt2(0.) * pt2(real{"nan",100})).size() == 1u);
        CHECK((0. * pt2(real{"inf",100})).size() == 1u);
        CHECK((0. * pt2(real{"nan",100})).size() == 1u);
        CHECK((pt2(real{"inf",100}) * 0.).size() == 1u);
        CHECK((pt2(real{"nan",100}) * 0.).size() == 1u);
        CHECK((pt2(0.) * real{"inf",100}).size() == 1u);
        CHECK((pt2(0.) * real{"nan",100}).size() == 1u);
        CHECK((real{"inf",100} * pt2(0.)).size() == 1u);
        CHECK((real{"nan",100} * pt2(0.)).size() == 1u);
        CHECK((pt2(0.) * pt2(-real{"inf",100})).size() == 1u);
        CHECK((pt2(0.) * pt2(-real{"nan",100})).size() == 1u);
        CHECK((0. * pt2(-real{"inf",100})).size() == 1u);
        CHECK((0. * pt2(-real{"nan",100})).size() == 1u);
        CHECK((pt2(-real{"inf",100}) * 0.).size() == 1u);
        CHECK((pt2(-real{"nan",100}) * 0.).size() == 1u);
        CHECK((pt2(0.) * -real{"inf",100}).size() == 1u);
        CHECK((pt2(0.) * -real{"nan",100}).size() == 1u);
        CHECK((-real{"inf",100} * pt2(0.)).size() == 1u);
        CHECK((-real{"nan",100} * pt2(0.)).size() == 1u);
        CHECK((pt2(0.) * pt2(0.)).size() == 0u);
        CHECK((pt2(0.) * pt2(1.)).size() == 0u);
        CHECK((pt2(1.) * pt2(0.)).size() == 0u);
        CHECK((pt2(0.) * (pt2(real{"inf",100}) + x)).size() == 1u);
        CHECK((pt2(0.) * (pt2(real{"nan",100}) + x)).size() == 1u);
        CHECK((0. * (pt2(real{"inf",100}) - x)).size() == 1u);
        CHECK((0. * (pt2(real{"nan",100}) - x)).size() == 1u);
        CHECK(((pt2(real{"inf",100}) + x) * 0.).size() == 1u);
        CHECK(((pt2(real{"nan",100}) + x) * 0.).size() == 1u);
        CHECK((pt2(0.) * (pt2(-real{"inf",100}) + x)).size() == 1u);
        CHECK((pt2(0.) * (pt2(-real{"nan",100}) + x)).size() == 1u);
        CHECK((0. * (pt2(-real{"inf",100}) - x)).size() == 1u);
        CHECK((0. * (pt2(-real{"nan",100}) - x)).size() == 1u);
        CHECK(((pt2(-real{"inf",100}) + x) * 0.).size() == 1u);
        CHECK(((pt2(-real{"nan",100}) + x) * 0.).size() == 1u);
        CHECK((pt2(1.) / pt2(0.)).size() == 1u);
        CHECK((pt2(1.) / 0.).size() == 1u);
        CHECK((1. / pt2(0.)).size() == 1u);
        pt2 tmp(0);
        tmp /= 0.;
        CHECK((tmp.size() == 1u));
        tmp = 0.;
        tmp /= pt2(0.);
        CHECK((tmp.size() == 1u));
        tmp = 1.;
        tmp /= pt2(0.);
        CHECK((tmp.size() == 1u));
        tmp = 1.;
        tmp /= 0.;
        CHECK((tmp.size() == 1u));
        CHECK(piranha::pow(pt2(0.), real{"nan",100}).size() == 1u);
        CHECK(piranha::pow(pt2(0.), -1).size() == 1u);
    }
#endif
    {
        using pt1 = polynomial<polynomial<double, monomial<int>>, monomial<int>>;
        pt1 x{"x"};
        if (std::numeric_limits<double>::is_iec559) {
            CHECK((pt1(0.) * pt1(std::numeric_limits<double>::infinity())).size() == 1u);
            CHECK((pt1(0.) * pt1(std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((0. * pt1(std::numeric_limits<double>::infinity())).size() == 1u);
            CHECK((0. * pt1(std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((pt1(std::numeric_limits<double>::infinity()) * 0.).size() == 1u);
            CHECK((pt1(std::numeric_limits<double>::quiet_NaN()) * 0.).size() == 1u);
            CHECK((pt1(0.) * std::numeric_limits<double>::infinity()).size() == 1u);
            CHECK((pt1(0.) * std::numeric_limits<double>::quiet_NaN()).size() == 1u);
            CHECK((std::numeric_limits<double>::infinity() * pt1(0.)).size() == 1u);
            CHECK((std::numeric_limits<double>::quiet_NaN() * pt1(0.)).size() == 1u);
            CHECK((pt1(0.) * pt1(-std::numeric_limits<double>::infinity())).size() == 1u);
            CHECK((pt1(0.) * pt1(-std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((0. * pt1(-std::numeric_limits<double>::infinity())).size() == 1u);
            CHECK((0. * pt1(-std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((pt1(-std::numeric_limits<double>::infinity()) * 0.).size() == 1u);
            CHECK((pt1(-std::numeric_limits<double>::quiet_NaN()) * 0.).size() == 1u);
            CHECK((pt1(0.) * -std::numeric_limits<double>::infinity()).size() == 1u);
            CHECK((pt1(0.) * -std::numeric_limits<double>::quiet_NaN()).size() == 1u);
            CHECK((-std::numeric_limits<double>::infinity() * pt1(0.)).size() == 1u);
            CHECK((-std::numeric_limits<double>::quiet_NaN() * pt1(0.)).size() == 1u);
            CHECK((pt1(0.) * pt1(0.)).size() == 0u);
            CHECK((pt1(0.) * pt1(1.)).size() == 0u);
            CHECK((pt1(1.) * pt1(0.)).size() == 0u);
            CHECK((pt1(0.) * (pt1(std::numeric_limits<double>::infinity()) + x)).size() == 1u);
            CHECK((pt1(0.) * (pt1(std::numeric_limits<double>::quiet_NaN()) + x)).size() == 1u);
            CHECK((0. * (pt1(std::numeric_limits<double>::infinity()) - x)).size() == 1u);
            CHECK((0. * (pt1(std::numeric_limits<double>::quiet_NaN()) - x)).size() == 1u);
            CHECK(((pt1(std::numeric_limits<double>::infinity()) + x) * 0.).size() == 1u);
            CHECK(((pt1(std::numeric_limits<double>::quiet_NaN()) + x) * 0.).size() == 1u);
            CHECK((pt1(0.) * (pt1(-std::numeric_limits<double>::infinity()) + x)).size() == 1u);
            CHECK((pt1(0.) * (pt1(-std::numeric_limits<double>::quiet_NaN()) + x)).size() == 1u);
            CHECK((0. * (pt1(-std::numeric_limits<double>::infinity()) - x)).size() == 1u);
            CHECK((0. * (pt1(-std::numeric_limits<double>::quiet_NaN()) - x)).size() == 1u);
            CHECK(((pt1(-std::numeric_limits<double>::infinity()) + x) * 0.).size() == 1u);
            CHECK(((pt1(-std::numeric_limits<double>::quiet_NaN()) + x) * 0.).size() == 1u);
            CHECK((pt1(0.) / pt1(0.)).size() == 1u);
            CHECK((pt1(0.) / pt1(std::numeric_limits<double>::quiet_NaN())).size() == 1u);
            CHECK((pt1(0.) / 0.).size() == 1u);
            CHECK((pt1(0.) / std::numeric_limits<double>::quiet_NaN()).size() == 1u);
            CHECK((pt1(std::numeric_limits<double>::quiet_NaN()) / pt1(0.)).size() == 1u);
            CHECK((0. / pt1(0.)).size() == 1u);
            CHECK((std::numeric_limits<double>::quiet_NaN() / pt1(0.)).size() == 1u);
            CHECK((pt1(1.) / pt1(0.)).size() == 1u);
            CHECK((pt1(1.) / 0.).size() == 1u);
            CHECK((1. / pt1(0.)).size() == 1u);
            pt1 tmp(0);
            tmp /= 0.;
            CHECK((tmp.size() == 1u));
            tmp = 0.;
            tmp /= pt1(0.);
            CHECK((tmp.size() == 1u));
            tmp = 1.;
            tmp /= pt1(0.);
            CHECK((tmp.size() == 1u));
            tmp = 1.;
            tmp /= 0.;
            CHECK((tmp.size() == 1u));
            CHECK(piranha::pow(pt1(0.), std::numeric_limits<double>::quiet_NaN()).size() == 1u);
            CHECK(piranha::pow(pt1(0.), -1).size() == 1u);
        }
    }
#if defined(MPPP_WITH_MPFR)
    {
        using pt2 = polynomial<polynomial<real, monomial<int>>, monomial<int>>;
        pt2 x{"x"};
        CHECK((pt2(0.) * pt2(real{"inf",100})).size() == 1u);
        CHECK((pt2(0.) * pt2(real{"nan", 100})).size() == 1u);
        CHECK((0. * pt2(real{"inf", 100})).size() == 1u);
        CHECK((0. * pt2(real{"nan", 100})).size() == 1u);
        CHECK((pt2(real{"inf", 100}) * 0.).size() == 1u);
        CHECK((pt2(real{"nan", 100}) * 0.).size() == 1u);
        CHECK((pt2(0.) * real{"inf", 100}).size() == 1u);
        CHECK((pt2(0.) * real{"nan", 100}).size() == 1u);
        CHECK((real{"inf", 100} * pt2(0.)).size() == 1u);
        CHECK((real{"nan", 100} * pt2(0.)).size() == 1u);
        CHECK((pt2(0.) * pt2(-real{"inf", 100})).size() == 1u);
        CHECK((pt2(0.) * pt2(-real{"nan", 100})).size() == 1u);
        CHECK((0. * pt2(-real{"inf", 100})).size() == 1u);
        CHECK((0. * pt2(-real{"nan", 100})).size() == 1u);
        CHECK((pt2(-real{"inf", 100}) * 0.).size() == 1u);
        CHECK((pt2(-real{"nan", 100}) * 0.).size() == 1u);
        CHECK((pt2(0.) * -real{"inf", 100}).size() == 1u);
        CHECK((pt2(0.) * -real{"nan", 100}).size() == 1u);
        CHECK((-real{"inf", 100} * pt2(0.)).size() == 1u);
        CHECK((-real{"nan", 100} * pt2(0.)).size() == 1u);
        CHECK((pt2(0.) * pt2(0.)).size() == 0u);
        CHECK((pt2(0.) * pt2(1.)).size() == 0u);
        CHECK((pt2(1.) * pt2(0.)).size() == 0u);
        CHECK((pt2(0.) * (pt2(real{"inf", 100}) + x)).size() == 1u);
        CHECK((pt2(0.) * (pt2(real{"nan", 100}) + x)).size() == 1u);
        CHECK((0. * (pt2(real{"inf", 100}) - x)).size() == 1u);
        CHECK((0. * (pt2(real{"nan", 100}) - x)).size() == 1u);
        CHECK(((pt2(real{"inf", 100}) + x) * 0.).size() == 1u);
        CHECK(((pt2(real{"nan", 100}) + x) * 0.).size() == 1u);
        CHECK((pt2(0.) * (pt2(-real{"inf", 100}) + x)).size() == 1u);
        CHECK((pt2(0.) * (pt2(-real{"nan", 100}) + x)).size() == 1u);
        CHECK((0. * (pt2(-real{"inf", 100}) - x)).size() == 1u);
        CHECK((0. * (pt2(-real{"nan", 100}) - x)).size() == 1u);
        CHECK(((pt2(-real{"inf", 100}) + x) * 0.).size() == 1u);
        CHECK(((pt2(-real{"nan", 100}) + x) * 0.).size() == 1u);
        CHECK((pt2(1.) / pt2(0.)).size() == 1u);
        CHECK((pt2(1.) / 0.).size() == 1u);
        CHECK((1. / pt2(0.)).size() == 1u);
        pt2 tmp(0);
        tmp /= 0.;
        CHECK((tmp.size() == 1u));
        tmp = 0.;
        tmp /= pt2(0.);
        CHECK((tmp.size() == 1u));
        tmp = 1.;
        tmp /= pt2(0.);
        CHECK((tmp.size() == 1u));
        tmp = 1.;
        tmp /= 0.;
        CHECK((tmp.size() == 1u));
        CHECK(piranha::pow(pt2(0.), real{"nan", 100}).size() == 1u);
        CHECK(piranha::pow(pt2(0.), -1).size() == 1u);
    }
#endif
}
