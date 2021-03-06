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

#include <piranha/print_coefficient.hpp>

#include <boost/lexical_cast.hpp>
#include <sstream>
#include <string>
#include <type_traits>

#include <mp++/config.hpp>

#include <piranha/integer.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif

#include "catch.hpp"

using namespace piranha;

struct trivial {
};

struct trivial_with_stream {
};

std::ostream &operator<<(std::ostream &, const trivial_with_stream &);

struct trivial_with_stream_wrong {
};

std::ostream &operator<<(std::ostream &, trivial_with_stream &);

TEST_CASE("print_coefficient_main_test")
{
    std::ostringstream oss;
    print_coefficient(oss, 0);
    CHECK(oss.str() == "0");
    oss.str("");
    print_coefficient(oss, integer(-5));
    CHECK(oss.str() == "-5");
    oss.str("");
#if defined(MPPP_WITH_MPFR)
    print_coefficient(oss, real("1.2345", 23));
    CHECK(oss.str() == boost::lexical_cast<std::string>(real("1.2345", 23)));
#endif
    CHECK((std::is_same<decltype(print_coefficient(oss, 42)), std::ostream &>::value));
    CHECK((std::is_same<decltype(print_coefficient(oss, integer(-5))), std::ostream &>::value));
}

TEST_CASE("print_coefficient_has_print_coefficient_test")
{
    CHECK(has_print_coefficient<int>::value);
    CHECK(has_print_coefficient<std::string>::value);
    CHECK(!has_print_coefficient<trivial>::value);
    CHECK(has_print_coefficient<trivial_with_stream>::value);
    CHECK(!has_print_coefficient<trivial_with_stream_wrong>::value);
}
