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

#include <piranha/power_series.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <mp++/config.hpp>

#include <piranha/config.hpp>
#include <piranha/integer.hpp>
#include <piranha/math.hpp>
#include <piranha/math/cos.hpp>
#include <piranha/math/degree.hpp>
#include <piranha/math/ldegree.hpp>
#include <piranha/math/sin.hpp>
#include <piranha/monomial.hpp>
#include <piranha/poisson_series.hpp>
#include <piranha/polynomial.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/real_trigonometric_kronecker_monomial.hpp>
#include <piranha/s11n.hpp>
#include <piranha/series.hpp>
#include <piranha/symbol_utils.hpp>

#include "catch.hpp"

using namespace piranha;

typedef boost::mpl::vector<double, integer
#if defined(MPPP_WITH_MPFR)
                           ,
                           real
#endif
                           >
    cf_types;
typedef boost::mpl::vector<int, integer> expo_types;

template <typename Cf, typename Expo>
class g_series_type : public power_series<series<Cf, monomial<Expo>, g_series_type<Cf, Expo>>, g_series_type<Cf, Expo>>
{
    using base = power_series<series<Cf, monomial<Expo>, g_series_type<Cf, Expo>>, g_series_type<Cf, Expo>>;

public:
    g_series_type() = default;
    g_series_type(const g_series_type &) = default;
    g_series_type(g_series_type &&) = default;
    g_series_type &operator=(const g_series_type &) = default;
    g_series_type &operator=(g_series_type &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type, base)
};

template <typename Cf>
class g_series_type2 : public power_series<series<Cf, rtk_monomial, g_series_type2<Cf>>, g_series_type2<Cf>>
{
    typedef power_series<series<Cf, rtk_monomial, g_series_type2<Cf>>, g_series_type2<Cf>> base;

public:
    g_series_type2() = default;
    g_series_type2(const g_series_type2 &) = default;
    g_series_type2(g_series_type2 &&) = default;
    g_series_type2 &operator=(const g_series_type2 &) = default;
    g_series_type2 &operator=(g_series_type2 &&) = default;
    PIRANHA_FORWARDING_CTOR(g_series_type2, base)
    PIRANHA_FORWARDING_ASSIGNMENT(g_series_type2, base)
};

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
    fake_int operator-(const fake_int &) const;
    fake_int &operator-=(const fake_int &);
    friend std::ostream &operator<<(std::ostream &, const fake_int &);
};

namespace piranha
{

namespace math
{

template <>
struct negate_impl<fake_int> {
    void operator()(fake_int &) const;
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
}

TEST_CASE("power_series_test_02")
{
    // Check the rational degree.
    typedef g_series_type<double, rational> stype0;
    CHECK((is_degree_type<stype0>::value));
    CHECK((is_ldegree_type<stype0>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype0>())), rational>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype0>())), rational>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype0>(), symbol_fset{})), rational>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype0>(), symbol_fset{})), rational>::value));
    typedef g_series_type<double, int> stype1;
    CHECK((is_degree_type<stype1>::value));
    CHECK((is_ldegree_type<stype1>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype1>())), int>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype1>())), int>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype1>(), symbol_fset{})), int>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype1>(), symbol_fset{})), int>::value));
    typedef g_series_type<stype1, long> stype2;
    CHECK((is_degree_type<stype2>::value));
    CHECK((is_ldegree_type<stype2>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype2>())), long>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype2>())), long>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype2>(), symbol_fset{})), long>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype2>(), symbol_fset{})), long>::value));
    typedef g_series_type2<double> stype3;
    CHECK((!is_degree_type<stype3>::value));
    CHECK((!is_ldegree_type<stype3>::value));
    typedef g_series_type2<g_series_type<g_series_type<double, int>, integer>> stype4;
    CHECK((is_degree_type<stype4>::value));
    CHECK((is_ldegree_type<stype4>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype4>())), integer>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype4>())), integer>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype4>(), symbol_fset{})), integer>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype4>(), symbol_fset{})), integer>::value));
    // Check actual instantiations as well.
    symbol_fset ss;
    CHECK(piranha::degree(stype1{}) == 0);
    CHECK(piranha::ldegree(stype1{}) == 0);
    CHECK(piranha::degree(stype1{}, ss) == 0);
    CHECK(piranha::ldegree(stype1{}, ss) == 0);
    CHECK(piranha::degree(stype2{}) == 0);
    CHECK(piranha::ldegree(stype2{}) == 0);
    CHECK(piranha::degree(stype2{}, ss) == 0);
    CHECK(piranha::ldegree(stype2{}, ss) == 0);
    CHECK(piranha::degree(stype4{}) == 0);
    CHECK(piranha::ldegree(stype4{}) == 0);
    CHECK(piranha::degree(stype4{}, ss) == 0);
    CHECK(piranha::ldegree(stype4{}, ss) == 0);
    // Tests with fake int.
    typedef g_series_type<double, fake_int> stype5;
    CHECK((is_degree_type<stype5>::value));
    CHECK((is_ldegree_type<stype5>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype5>())), fake_int>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype5>())), fake_int>::value));
    CHECK((std::is_same<decltype(piranha::degree(std::declval<stype5>(), symbol_fset{})), fake_int>::value));
    CHECK((std::is_same<decltype(piranha::ldegree(std::declval<stype5>(), symbol_fset{})), fake_int>::value));
    typedef g_series_type<stype5, int> stype6;
    // This does not have a degree type because fake_int cannot be added to integer.
    CHECK((!is_degree_type<stype6>::value));
    CHECK((!is_ldegree_type<stype6>::value));
}

#if defined(PIRANHA_WITH_BOOST_S11N)

TEST_CASE("power_series_serialization_test")
{
    typedef g_series_type<polynomial<rational, monomial<rational>>, rational> stype;
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

TEST_CASE("power_series_truncation_test")
{
    // A test with polynomial, degree only in the key.
    {
        typedef polynomial<double, monomial<rational>> stype0;
        CHECK((has_truncate_degree<stype0, int>::value));
        CHECK((has_truncate_degree<stype0, rational>::value));
        CHECK((has_truncate_degree<stype0, integer>::value));
        CHECK((!has_truncate_degree<stype0, std::string>::value));
        stype0 x{"x"}, y{"y"}, z{"z"};
        stype0 s0;
        CHECK((std::is_same<stype0, decltype(s0.truncate_degree(5))>::value));
        CHECK(s0.truncate_degree(5) == s0);
        s0 = x.pow(rational(10, 3));
        CHECK(s0.truncate_degree(5) == s0);
        CHECK(s0.truncate_degree(3 / 2_q) == 0);
        // x**5*y+1/2*z**-5*x*y+x*y*z/4
        s0 = x.pow(5) * y + z.pow(-5) / 2 * x * y + x * y * z / 4;
        CHECK(s0.truncate_degree(3) == z.pow(-5) / 2 * x * y + x * y * z / 4);
        CHECK(math::truncate_degree(s0, -1) == z.pow(-5) / 2 * x * y);
        CHECK(math::truncate_degree(s0, 2, {"x"}) == z.pow(-5) / 2 * x * y + x * y * z / 4);
        CHECK(math::truncate_degree(s0, 5, {"x", "y"}) == z.pow(-5) / 2 * x * y + x * y * z / 4);
        CHECK(math::truncate_degree(s0, 5, {"y", "x", "y"}) == z.pow(-5) / 2 * x * y + x * y * z / 4);
        CHECK(math::truncate_degree(s0, 5, {"z", "x"}) == s0);
        // Test with non-existing variable.
        CHECK(math::truncate_degree(s0, 0, {"a", "b"}) == s0);
    }
    {
        // Poisson series, degree only in the coefficient.
        typedef poisson_series<polynomial<rational, monomial<rational>>> st;
        CHECK((has_truncate_degree<st, int>::value));
        CHECK((has_truncate_degree<st, rational>::value));
        CHECK((has_truncate_degree<st, integer>::value));
        CHECK((!has_truncate_degree<st, std::string>::value));
        st x("x"), y("y"), z("z"), a("a"), b("b");
        // (x + y**2/4 + 3*x*y*z/7) * cos(a) + (x*y+y*z/3+3*z**2*x/8) * sin(a+b)
        st s0 = (x + y * y / 4 + 3 * z * x * y / 7) * piranha::cos(a)
                + (x * y + z * y / 3 + 3 * z * z * x / 8) * piranha::sin(a + b);
        CHECK(s0.truncate_degree(2) ==
                          (x + y * y / 4) * piranha::cos(a) + (x * y + z * y / 3) * piranha::sin(a + b));
        CHECK(math::truncate_degree(s0, 1l) == x * piranha::cos(a));
        CHECK(math::truncate_degree(s0, -1ll) == 0);
        CHECK(math::truncate_degree(s0, 1l, {"x"}) ==
                          (x + y * y / 4 + 3 * z * x * y / 7) * piranha::cos(a)
                              + (x * y + z * y / 3 + 3 * z * z * x / 8) * piranha::sin(a + b));
        CHECK(math::truncate_degree(s0, char(0), {"x"}) ==
                          y * y / 4 * piranha::cos(a) + z * y / 3 * piranha::sin(a + b));
        CHECK(math::truncate_degree(s0, char(1), {"y", "x"}) ==
                          x * piranha::cos(a) + (z * y / 3 + 3 * z * z * x / 8) * piranha::sin(a + b));
        CHECK(math::truncate_degree(s0, integer(1), {"z"}) ==
                          (x + y * y / 4 + 3 * z * x * y / 7) * piranha::cos(a)
                              + (x * y + z * y / 3) * piranha::sin(a + b));
        // Test with non-existing variable.
        CHECK(math::truncate_degree(s0, 0, {"foo", "bar"}) == s0);
    }
    {
        // Recursive poly.
        typedef polynomial<rational, monomial<rational>> st0;
        typedef polynomial<st0, monomial<rational>> st1;
        CHECK((has_truncate_degree<st1, int>::value));
        CHECK((has_truncate_degree<st1, rational>::value));
        CHECK((has_truncate_degree<st1, integer>::value));
        CHECK((!has_truncate_degree<st1, std::string>::value));
        // (x*y+x**2+x+1/4)*z + (x+y**2+x**2*y)*z**2 + 3
        st0 x{"x"}, y{"y"};
        st1 z{"z"};
        auto s0 = (x * y + x * x + x + 1_q / 4) * z + (x + y * y + x * x * y) * z * z + 3;
        CHECK(s0.truncate_degree(1) == 1 / 4_q * z + 3);
        CHECK(s0.truncate_degree(0) == 3);
        CHECK(s0.truncate_degree(2) == (x + 1_q / 4) * z + 3);
        CHECK(math::truncate_degree(s0, -3) == 0);
        CHECK(math::truncate_degree(s0, 3_q) == (x * y + x * x + x + 1_q / 4) * z + x * z * z + 3);
        CHECK(math::truncate_degree(s0, 1, {"x"}) == (x * y + x + 1_q / 4) * z + (x + y * y) * z * z + 3);
        CHECK(math::truncate_degree(s0, 1ll, {"x", "y"}) == (x + 1_q / 4) * z + x * z * z + 3);
        CHECK(math::truncate_degree(s0, 1, {"x", "z"}) == 1_q / 4 * z + 3);
        CHECK(math::truncate_degree(s0, 2, {"x", "z"}) == (x * y + x + 1_q / 4) * z + y * y * z * z + 3);
        CHECK(math::truncate_degree(s0, 3, {"x", "z"}) ==
                          (x * y + x * x + x + 1_q / 4) * z + (x + y * y) * z * z + 3);
        // Test with non-existing variable.
        CHECK(math::truncate_degree(s0, 0, {"foo", "bar"}) == s0);
    }
    {
        // Recursive poly, integers and rational exponent mixed, same example as above.
        typedef polynomial<rational, monomial<integer>> st0;
        typedef polynomial<st0, monomial<rational>> st1;
        CHECK((has_truncate_degree<st1, int>::value));
        CHECK((has_truncate_degree<st1, rational>::value));
        CHECK((has_truncate_degree<st1, integer>::value));
        CHECK((!has_truncate_degree<st1, std::string>::value));
        // (x*y+x**2+x+1/4)*z + (x+y**2+x**2*y)*z**2 + 3
        st0 x{"x"}, y{"y"};
        st1 z{"z"};
        auto s0 = (x * y + x * x + x + 1_q / 4) * z + (x + y * y + x * x * y) * z * z + 3;
        CHECK(s0.truncate_degree(1) == 1 / 4_q * z + 3);
        CHECK(s0.truncate_degree(0) == 3);
        CHECK(s0.truncate_degree(2) == (x + 1_q / 4) * z + 3);
        CHECK(math::truncate_degree(s0, -3) == 0);
        CHECK(math::truncate_degree(s0, 3_q) == (x * y + x * x + x + 1_q / 4) * z + x * z * z + 3);
        CHECK(math::truncate_degree(s0, 1, {"x"}) == (x * y + x + 1_q / 4) * z + (x + y * y) * z * z + 3);
        CHECK(math::truncate_degree(s0, 1ll, {"x", "y"}) == (x + 1_q / 4) * z + x * z * z + 3);
        CHECK(math::truncate_degree(s0, 1, {"x", "z"}) == 1_q / 4 * z + 3);
        CHECK(math::truncate_degree(s0, 2, {"x", "z"}) == (x * y + x + 1_q / 4) * z + y * y * z * z + 3);
        CHECK(math::truncate_degree(s0, 3, {"x", "z"}) ==
                          (x * y + x * x + x + 1_q / 4) * z + (x + y * y) * z * z + 3);
        // Test with non-existing variable.
        CHECK(math::truncate_degree(s0, 0_q, {"foo", "bar"}) == s0);
    }
    {
        // Recursive poly, integers and rational exponent mixed, same example as above but switched.
        typedef polynomial<rational, monomial<rational>> st0;
        typedef polynomial<st0, monomial<integer>> st1;
        CHECK((has_truncate_degree<st1, int>::value));
        CHECK((has_truncate_degree<st1, rational>::value));
        CHECK((has_truncate_degree<st1, integer>::value));
        CHECK((!has_truncate_degree<st1, std::string>::value));
        // (x*y+x**2+x+1/4)*z + (x+y**2+x**2*y)*z**2 + 3
        st0 x{"x"}, y{"y"};
        st1 z{"z"};
        auto s0 = (x * y + x * x + x + 1_q / 4) * z + (x + y * y + x * x * y) * z * z + 3;
        CHECK(s0.truncate_degree(1) == 1 / 4_q * z + 3);
        CHECK(s0.truncate_degree(0) == 3);
        CHECK(s0.truncate_degree(2) == (x + 1_q / 4) * z + 3);
        CHECK(math::truncate_degree(s0, -3) == 0);
        CHECK(math::truncate_degree(s0, 3_q) == (x * y + x * x + x + 1_q / 4) * z + x * z * z + 3);
        CHECK(math::truncate_degree(s0, 1, {"x"}) == (x * y + x + 1_q / 4) * z + (x + y * y) * z * z + 3);
        CHECK(math::truncate_degree(s0, 1ll, {"x", "y"}) == (x + 1_q / 4) * z + x * z * z + 3);
        CHECK(math::truncate_degree(s0, 1, {"x", "z"}) == 1_q / 4 * z + 3);
        CHECK(math::truncate_degree(s0, 2, {"x", "z"}) == (x * y + x + 1_q / 4) * z + y * y * z * z + 3);
        CHECK(math::truncate_degree(s0, 3, {"x", "z"}) ==
                          (x * y + x * x + x + 1_q / 4) * z + (x + y * y) * z * z + 3);
        // Test with non-existing variable.
        CHECK(math::truncate_degree(s0, 0_z, {"foo", "bar"}) == s0);
    }
}

TEST_CASE("power_series_degree_overflow_test")
{
    using p_type = polynomial<integer, monomial<int>>;
    using pp_type = polynomial<p_type, monomial<int>>;
    p_type x{"x"};
    pp_type y{"y"};
    CHECK_THROWS_AS((x * y.pow(std::numeric_limits<int>::max())).degree(), std::overflow_error);
    CHECK_THROWS_AS((x.pow(-1) * y.pow(std::numeric_limits<int>::min())).degree(), std::overflow_error);
    CHECK((x * y.pow(std::numeric_limits<int>::min())).degree() == std::numeric_limits<int>::min() + 1);
}

TEST_CASE("power_series_mixed_degree_test")
{
    using p_type = polynomial<integer, monomial<int>>;
    using pp_type = polynomial<p_type, monomial<integer>>;
    using pp_type2 = polynomial<p_type, monomial<long>>;
    using pp_type3 = polynomial<p_type, monomial<int>>;
    using pp_type4 = polynomial<polynomial<rational, monomial<rational>>, monomial<long long>>;
    p_type x{"x"};
    pp_type y{"y"};
    pp_type2 z{"z"};
    pp_type3 a{"a"};
    pp_type4 b{"b"};
    CHECK((std::is_same<decltype(x.degree()), int>::value));
    CHECK((std::is_same<decltype(y.degree()), integer>::value));
    CHECK((std::is_same<decltype(z.degree()), long>::value));
    CHECK((std::is_same<decltype(a.degree()), int>::value));
    CHECK((std::is_same<decltype(b.degree()), rational>::value));
}
