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

#include <piranha/tuning.hpp>

#include <stdexcept>
#include <thread>

#include "catch.hpp"

using namespace piranha;

TEST_CASE("tuning_parallel_memory_set_test")
{
    CHECK(tuning::get_parallel_memory_set());
    tuning::set_parallel_memory_set(false);
    CHECK(!tuning::get_parallel_memory_set());
    std::thread t1([]() noexcept {
        while (!tuning::get_parallel_memory_set()) {
        }
    });
    std::thread t2([]() noexcept { tuning::set_parallel_memory_set(true); });
    t1.join();
    t2.join();
    CHECK(tuning::get_parallel_memory_set());
    tuning::set_parallel_memory_set(false);
    CHECK(!tuning::get_parallel_memory_set());
    tuning::reset_parallel_memory_set();
    CHECK(tuning::get_parallel_memory_set());
}

TEST_CASE("tuning_block_size_test")
{
    CHECK(tuning::get_multiplication_block_size() == 256u);
    tuning::set_multiplication_block_size(512u);
    CHECK(tuning::get_multiplication_block_size() == 512u);
    std::thread t1([]() noexcept {
        while (tuning::get_multiplication_block_size() != 1024u) {
        }
    });
    std::thread t2([]() { tuning::set_multiplication_block_size(1024u); });
    t1.join();
    t2.join();
    CHECK_THROWS_AS(tuning::set_multiplication_block_size(8000u), std::invalid_argument);
    CHECK(tuning::get_multiplication_block_size() == 1024u);
    tuning::reset_multiplication_block_size();
    CHECK(tuning::get_multiplication_block_size() == 256u);
}

TEST_CASE("tuning_estimation_threshold_test")
{
    CHECK(tuning::get_estimate_threshold() == 200u);
    tuning::set_estimate_threshold(512u);
    CHECK(tuning::get_estimate_threshold() == 512u);
    std::thread t1([]() noexcept {
        while (tuning::get_estimate_threshold() != 1024u) {
        }
    });
    std::thread t2([]() noexcept { tuning::set_estimate_threshold(1024u); });
    t1.join();
    t2.join();
    CHECK(tuning::get_estimate_threshold() == 1024u);
    tuning::reset_estimate_threshold();
    CHECK(tuning::get_estimate_threshold() == 200u);
}
