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

#include <piranha/settings.hpp>

#include <stdexcept>

#include <piranha/runtime_info.hpp>

#include "catch.hpp"

using namespace piranha;

// Check getting and setting number of threads.
TEST_CASE("settings_thread_number_test")
{
    const auto original = settings::get_n_threads();
    //CHECK_PREDICATE([](unsigned n) { return n != 0; }, (settings::get_n_threads()));
    CHECK((settings::get_n_threads() != 0));
    for (unsigned i = 0; i < runtime_info::get_hardware_concurrency(); ++i) {
        settings::set_n_threads(i + 1u);
        CHECK(settings::get_n_threads() == i + 1u);
    }
    CHECK_THROWS_AS(settings::set_n_threads(0u), std::invalid_argument);
    settings::set_n_threads(10u);
    settings::reset_n_threads();
    CHECK(original == settings::get_n_threads());
}

TEST_CASE("settings_cache_line_size_test")
{
    const auto original = settings::get_cache_line_size();
    CHECK(settings::get_cache_line_size() == original);
    CHECK(settings::get_cache_line_size() == runtime_info::get_cache_line_size());
    settings::set_cache_line_size(512u);
    CHECK(settings::get_cache_line_size() == 512u);
    settings::set_cache_line_size(0u);
    CHECK(settings::get_cache_line_size() == 0u);
    settings::reset_cache_line_size();
    CHECK(original == settings::get_cache_line_size());
}

TEST_CASE("settings_max_term_output_test")
{
    settings::set_max_term_output(10u);
    CHECK(10u == settings::get_max_term_output());
    settings::reset_max_term_output();
    CHECK(20u == settings::get_max_term_output());
}

TEST_CASE("settings_min_work_per_thread_test")
{
    const auto def = settings::get_min_work_per_thread();
    CHECK_THROWS_AS(settings::set_min_work_per_thread(0u), std::invalid_argument);
    CHECK_NOTHROW(settings::set_min_work_per_thread(1u));
    CHECK(settings::get_min_work_per_thread() == 1u);
    CHECK_NOTHROW(settings::set_min_work_per_thread(10u));
    CHECK(settings::get_min_work_per_thread() == 10u);
    CHECK_NOTHROW(settings::reset_min_work_per_thread());
    CHECK(settings::get_min_work_per_thread() == def);
}
