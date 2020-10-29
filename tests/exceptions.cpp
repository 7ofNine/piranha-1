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

#include <piranha/exceptions.hpp>


#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <boost/algorithm/string/predicate.hpp>

#include <piranha/config.hpp>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

struct exc0 {
    exc0(int, double) {}
};

struct exc1 {
    exc1(int) {}
};

template <int N>
inline void foo()
{
    foo<N - 1>();
}

template <>
inline void foo<0>()
{
    piranha_throw(std::runtime_error, "here we are!");
}

TEST_CASE("exception_test_00")
{
    // not_implemented_error.
    CHECK((std::is_constructible<not_implemented_error, std::string>::value));
    CHECK((std::is_constructible<not_implemented_error, char *>::value));
    CHECK((std::is_constructible<not_implemented_error, const char *>::value));
    CHECK((!std::is_constructible<not_implemented_error>::value));
    CHECK_THROWS_MATCHES(piranha_throw(not_implemented_error, "foobar"), not_implemented_error,
                          test::ExceptionMatcher<not_implemented_error>(std::string("foobar")));
    CHECK_THROWS_MATCHES(piranha_throw(not_implemented_error, std::string("foobar")), not_implemented_error,
                          test::ExceptionMatcher<not_implemented_error>(std::string("foobar")));
    CHECK_THROWS_MATCHES(piranha_throw(not_implemented_error, "foobar"), std::runtime_error,
                          test::ExceptionMatcher<std::runtime_error>(std::string("foobar")));
    CHECK_THROWS_MATCHES(piranha_throw(not_implemented_error, std::string("foobar")), std::runtime_error,
                          test::ExceptionMatcher<std::runtime_error>(std::string("foobar")));
    // A couple of tests with exceptions that do not accept string ctor.
    CHECK_THROWS_AS(piranha_throw(std::bad_alloc, ), std::bad_alloc);
    CHECK_THROWS_AS(piranha_throw(exc0, 1, 2.3), exc0);
    CHECK_THROWS_AS(piranha_throw(exc1, 1), exc1);
#if defined(PIRANHA_WITH_BOOST_STACKTRACE)
    CHECK(!stacktrace_statics<>::enabled.load());
    stacktrace_statics<>::enabled.store(true);
    CHECK_THROWS_AS(piranha_throw(exc1, 1), exc1);
    bool caught = false;
    try {
        foo<100>();
    } catch (const std::runtime_error &re) {
        std::cout << re.what() << '\n';
        CHECK(boost::contains(re.what(), "here we are!"));
        caught = true;
    }
    CHECK(caught);
#endif
}

// Disable the assertion test on Windows, not sure what is going on
// with those signal handlers.
#if defined(PIRANHA_WITH_BOOST_STACKTRACE) && !defined(_WIN32)

#include <csignal>
#include <cstdlib>

extern "C" {

inline void signal_handler(int)
{
    // NOTE: we call _Exit here because it's guaranteed to be
    // async-safe:
    // https://stackoverflow.com/questions/8493095/what-constitutes-asynchronous-safeness
    std::_Exit(EXIT_SUCCESS);
}
}

TEST_CASE("assert_test_00")
{
    // Here are going to trigger an assertion failure to verify
    // visually that the stacktrace is actually printed. We need to
    // override the default abort handler in order to have the process
    // return success.
    std::signal(SIGABRT, signal_handler);
    piranha_assert(false);
}

#endif
