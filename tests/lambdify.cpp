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

#include <piranha/lambdify.hpp>

#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <piranha/exceptions.hpp>
#include <piranha/integer.hpp>
#include <piranha/kronecker_monomial.hpp>
#include <piranha/math.hpp>
#include <piranha/polynomial.hpp>
#include <piranha/rational.hpp>

#include "catch.hpp"

using namespace piranha;
using math::evaluate;
using math::lambdify;

static std::mt19937 rng;

static const int ntrials = 100;

struct callable_42 {
    double operator()(const std::vector<double> &) const
    {
        return 42.;
    }
};

struct callable_12 {
    double operator()(const std::vector<double> &) const
    {
        return 12.;
    }
};

struct callable_generic {
    template <typename T>
    T operator()(const std::vector<T> &) const
    {
        return T{0};
    }
};

TEST_CASE("lambdify_test_00")
{
    {
        using p_type = polynomial<integer, k_monomial>;
        p_type x{"x"}, y{"y"}, z{"z"};
        CHECK((has_lambdify<p_type, integer>::value));
        auto l0 = lambdify<integer>(x + y + z, {"x", "y", "z"});
        CHECK(!std::is_copy_assignable<decltype(l0)>::value);
        CHECK(!std::is_move_assignable<decltype(l0)>::value);
        CHECK(std::is_copy_constructible<decltype(l0)>::value);
        CHECK(std::is_move_constructible<decltype(l0)>::value);
        CHECK((std::is_same<decltype(l0({})), integer>::value));
        CHECK(l0({1_z, 2_z, 3_z}) == 6);
        auto l1 = lambdify<integer>(x + 2 * y + 3 * z, {"y", "z", "x"});
        CHECK(l1({1_z, 2_z, 3_z}) == 2 * 1 + 3 * 2 + 3);
        CHECK_THROWS_AS(lambdify<integer>(x + 2 * y + 3 * z, {"y", "z", "x", "x"}), std::invalid_argument);
        CHECK((has_lambdify<p_type, rational>::value));
        auto l2 = lambdify<rational>(x * x - 2 * y + 3 * z * z * z, {"x", "y", "z", "a"});
        CHECK((std::is_same<decltype(l2({})), rational>::value));
        CHECK_THROWS_AS(l2({1_q, 2_q, 3_q}), std::invalid_argument);
        CHECK_THROWS_AS(l2({1_q, 2_q, 3_q, 4_q, 5_q}), std::invalid_argument);
        CHECK(l2({1 / 7_q, -2 / 5_q, 2 / 3_q, 15_q}) ==  1 / 7_q * 1 / 7_q - 2 * -2 / 5_q + 3 * 2 / 3_q * 2 / 3_q * 2 / 3_q);
        CHECK((has_lambdify<p_type, double>::value));
        auto l3 = lambdify<double>(x * x - 2 * y + 3 * z * z * z, {"x", "y", "z"});
        CHECK((std::is_same<decltype(l3({})), double>::value));
        CHECK((has_lambdify<p_type, p_type>::value));
        auto l4 = lambdify<p_type>(x * x - 2 * y + 3 * z * z * z, {"x", "y", "z"});
        CHECK((std::is_same<decltype(l4({})), p_type>::value));
        // Try with copy construction as well.
        auto tmp = x - z;
        auto l5 = lambdify<double>(tmp, {"x", "y", "z"});
        CHECK((std::is_same<decltype(l5({})), double>::value));
        CHECK(l5({1., 2., 3.}) == 1. - 3.);
        CHECK_THROWS_AS(l5({1., 3.}) , std::invalid_argument);
    }
    {
        CHECK((has_lambdify<double, integer>::value));
        CHECK((has_lambdify<double &&, integer>::value));
        CHECK((has_lambdify<double &&, const integer>::value));
        CHECK((has_lambdify<double &&, const integer &>::value));
        CHECK((has_lambdify<double, std::string>::value));
        CHECK((has_lambdify<double, rational>::value));
        auto l0 = lambdify<integer>(3.4, {});
        CHECK((std::is_same<double, decltype(l0({}))>::value));
        CHECK(l0({}) == 3.4);
        CHECK_THROWS_AS(l0({1_z, 2_z, 3_z}), std::invalid_argument);
    }
    {
        // Various checks with the extra symbol map.
        using p_type = polynomial<integer, k_monomial>;
        p_type x{"x"}, y{"y"}, z{"z"};
        auto l0 = lambdify<integer>(x + y + z, {"x"},
                                    {{"z",
                                      [](const std::vector<integer> &v) -> integer {
                                          CHECK(v.size() == 1u);
                                          // z is 3*x.
                                          return v[0] * 3_z;
                                      }},
                                     {"y", [](const std::vector<integer> &v) -> integer {
                                          CHECK(v.size() == 1u);
                                          // y is 2*x.
                                          return v[0] * 2_z;
                                      }}});
        CHECK(l0({1_z}) == 6);
        CHECK(l0({2_z}) == 12);
        CHECK(l0({0_z}) == 0);
        CHECK(l0({-3_z}) == -18);
        auto l1 = lambdify<integer>(x + y + z, {"x"}, {{"z", [](const std::vector<integer> &v) -> integer {
                                                            CHECK(v.size() == 1u);
                                                            return 3_z;
                                                        }}});
        // We cannot evaluate as the y evaluation value is missing.
        CHECK_THROWS_AS(l1({1_z}), std::invalid_argument);
        // Too many values provided.
        CHECK_THROWS_AS(l1({1_z, 2_z}), std::invalid_argument);
        // Check an init list that contains duplicates.
        CHECK(lambdify<integer>(x + y,  {"x"},
                                            {{"y",
                                              [](const std::vector<integer> &v) -> integer {
                                                  CHECK(v.size() == 1u);
                                                  return 4_z;
                                              }},
                                             {"y",
                                              [](const std::vector<integer> &v) -> integer {
                                                  CHECK(v.size() == 1u);
                                                  return 3_z;
                                              }}})({1_z}) ==
                          5);
        // Check with extra non-evaluated args.
        CHECK(lambdify<integer>(x + y, {"x", "z"},
                                            {{"y",
                                              [](const std::vector<integer> &v) -> integer {
                                                  CHECK(v.size() == 2u);
                                                  return 4_z;
                                              }},
                                             {"t",
                                              [](const std::vector<integer> &v) -> integer {
                                                  CHECK(v.size() == 2u);
                                                  return 3_z;
                                              }}})({1_z, 123_z}) ==
                          5);
        // Check with extra symbol already in positional args.
        CHECK_THROWS_AS(lambdify<integer>(x + y, {"x", "y"},
                                            {{"y",
                                              [](const std::vector<integer> &v) -> integer {
                                                  CHECK(v.size() == 2u);
                                                  return 4_z;
                                              }}})({1_z, 123_z}),
                          std::invalid_argument);
        // Another error check.
        CHECK_THROWS_AS(lambdify<integer>(x + y, {"x"},
                                            {{"y",
                                              [](const std::vector<integer> &v) -> integer {
                                                  CHECK(v.size() == 2u);
                                                  return 4_z;
                                              }}})({1_z, 123_z}),
                          std::invalid_argument);
        // A test with only custom symbols.
        CHECK(lambdify<integer>(x + y, {},
                                            {{"x",
                                              [](const std::vector<integer> &v) -> integer {
                                                  CHECK(v.size() == 0u);
                                                  return 4_z;
                                              }},
                                             {"y",
                                              [](const std::vector<integer> &v) -> integer {
                                                  CHECK(v.size() == 0u);
                                                  return 3_z;
                                              }}})({}) ==
                          7);
        // A couple of tests with nothing.
        CHECK(lambdify<integer>(p_type{}, {})({}) == 0);
        CHECK(lambdify<integer>(p_type{}, {"x", "y"},
                                            {{"z", [](const std::vector<integer> &) { return 1_z; }}})({1_z, 2_z}) ==
                          0);
        // Check with non-lambda callables.
        CHECK(lambdify<double>(x + y, {"x"}, {{"y", callable_42{}}})({1.}) == 43.);
        CHECK(lambdify<double>(x + y, {"x"}, {{"y", callable_generic{}}})({1.}) == 1.);
        CHECK(lambdify<integer>(x + y, {"x"}, {{"y", callable_generic{}}})({2_z}) == 2);
        CHECK(lambdify<double>(x + y, {"x"}, {{"y", callable_12{}}})({-1.}) == 11.);
        CHECK(lambdify<double>(x + y + z, {"x"}, {{"y", callable_12{}}, {"z", callable_42{}}})({-1.}) ==
                          -1 + 42. + 12.);
    }
}

TEST_CASE("lambdify_test_01")
{
    {
        // A few tests with copies and moves.
        using p_type = polynomial<integer, k_monomial>;
        p_type x{"x"}, y{"y"}, z{"z"};
        auto l0 = lambdify<integer>(x + y + z, {"x", "y", "z"});
        auto l1(l0);
        CHECK(l0({1_z, 2_z, 3_z}) == l1({1_z, 2_z, 3_z}));
        auto l2(std::move(l1));
        CHECK(l0({1_z, 2_z, 3_z}) == l2({1_z, 2_z, 3_z}));
        // Random testing.
        std::uniform_int_distribution<int> dist(-10, 10);
        const auto tmp = x * x - 6 * y + z * y * x;
        auto l = lambdify<integer>(tmp, {"y", "x", "z"});
        for (int i = 0; i < ntrials; ++i) {
            auto xn = integer(dist(rng)), yn = integer(dist(rng)), zn = integer(dist(rng));
            CHECK(l({yn, xn, zn}) == evaluate<integer>(tmp, {{"x", xn}, {"y", yn}, {"z", zn}}));
        }
    }
    {
        using p_type = polynomial<integer, k_monomial>;
        p_type x{"x"}, y{"y"}, z{"z"};
        auto l0 = lambdify<integer>(x + y + z, {"x", "y"}, {{"z", [](const std::vector<integer> &v) -> integer {
                                                                 CHECK(v.size() == 2u);
                                                                 return v[0] * v[1];
                                                             }}});
        auto l1(l0);
        CHECK(l0({1_z, 2_z}) == l1({1_z, 2_z}));
        CHECK(l0({1_z, 2_z}) == 5);
        auto l2(std::move(l1));
        CHECK(l0({1_z, 2_z}) == l2({1_z, 2_z}));
        CHECK(l0({1_z, 2_z}) == 5);
        // Random testing.
        std::uniform_int_distribution<int> dist(-10, 10);
        const auto tmp = x * x - 6 * y + z * y * x;
        auto l = lambdify<integer>(tmp, {"y", "x"}, {{"z", [](const std::vector<integer> &v) -> integer {
                                                          CHECK(v.size() == 2u);
                                                          return v[0] * v[1];
                                                      }}});
        for (int i = 0; i < ntrials; ++i) {
            auto xn = integer(dist(rng)), yn = integer(dist(rng));
            CHECK(l({yn, xn}) == evaluate<integer>(tmp, {{"x", xn}, {"y", yn}, {"z", xn * yn}}));
        }
    }
}

TEST_CASE("lambdify_test_02")
{
    // Test getters.
    using p_type = polynomial<integer, k_monomial>;
    p_type x{"x"}, y{"y"}, z{"z"};
    auto l0 = lambdify<integer>(x + y + z, {"z", "y", "x"});
    CHECK(!std::is_copy_assignable<decltype(l0)>::value);
    CHECK(!std::is_move_assignable<decltype(l0)>::value);
    CHECK(x + y + z == l0.get_evaluable());
    const auto v = std::vector<std::string>({"z", "y", "x"});
   // BOOST_CHECK_EQUAL_COLLECTIONS(v.begin(), v.end(), l0.get_names().begin(), l0.get_names().end()); //TODDO how todo such thing with cath??
    CHECK(v == l0.get_names());  // temporarily
    auto en = l0.get_extra_names();
    CHECK(en.empty());
    auto l1 = lambdify<integer>(x + y + z, {"z", "y", "x"}, {{"t", [](const std::vector<integer> &) { return 1_z; }}});
    en = l1.get_extra_names();
    CHECK(en == std::vector<std::string>{"t"});
    auto l2 = lambdify<integer>(x + y + z, {"z", "y", "x"},
                                {{"t", [](const std::vector<integer> &) { return 1_z; }},
                                 {"a", [](const std::vector<integer> &) { return 1_z; }}});
    en = l2.get_extra_names();
    CHECK((en == std::vector<std::string>{"t", "a"} || en == std::vector<std::string>{"a", "t"}));
}
