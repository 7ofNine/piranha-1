/***************************************************************************
 *   Copyright (C) 2009-2011 by Francesco Biscani                          *
 *   bluescarni@gmail.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "../src/exceptions.hpp"

#define BOOST_TEST_MODULE exceptions_test
#include <boost/test/unit_test.hpp>

#include <string>

using namespace piranha;

struct custom_exception0: public base_exception
{
	explicit custom_exception0():base_exception(std::string()) {}
};

struct custom_exception1: public base_exception
{
	explicit custom_exception1():base_exception(std::string()) {}
	explicit custom_exception1(const std::string &msg):base_exception(std::string(msg)) {}
};

struct custom_exception2: public base_exception
{
	explicit custom_exception2():base_exception(std::string()) {}
	explicit custom_exception2(const std::string &msg):base_exception(msg) {}
	explicit custom_exception2(const std::string &msg, int):base_exception(std::string(msg)) {}
};

struct custom_exception3: public base_exception
{
	explicit custom_exception3():base_exception(std::string()) {}
	explicit custom_exception3(const std::string &msg):base_exception(msg) {}
	explicit custom_exception3(const std::string &msg, int):base_exception(std::string(msg)) {}
	explicit custom_exception3(int, const std::string &msg):base_exception(std::string(msg)) {}
};

struct custom_exception4: public base_exception
{
	explicit custom_exception4(const char *msg):base_exception(std::string(msg)) {}
};

BOOST_AUTO_TEST_CASE(exception_main_test)
{
	BOOST_CHECK_THROW(piranha_throw(custom_exception0,),custom_exception0);
	BOOST_CHECK_THROW(piranha_throw(custom_exception1,),custom_exception1);
	BOOST_CHECK_THROW(piranha_throw(custom_exception1,std::string("")),custom_exception1);
	BOOST_CHECK_THROW(piranha_throw(custom_exception1,""),custom_exception1);
	BOOST_CHECK_THROW(piranha_throw(custom_exception2,),custom_exception2);
	BOOST_CHECK_THROW(piranha_throw(custom_exception2,std::string("")),custom_exception2);
	BOOST_CHECK_THROW(piranha_throw(custom_exception2,std::string(""),3),custom_exception2);
	BOOST_CHECK_THROW(piranha_throw(custom_exception3,),custom_exception3);
	BOOST_CHECK_THROW(piranha_throw(custom_exception3,std::string("")),custom_exception3);
	BOOST_CHECK_THROW(piranha_throw(custom_exception3,std::string(""),3),custom_exception3);
	BOOST_CHECK_THROW(piranha_throw(custom_exception3,3,std::string("")),custom_exception3);
	const std::string empty;
	try {
		piranha_throw(custom_exception0,);
	} catch (const base_exception &e) {
		BOOST_CHECK_EQUAL(e.what(),empty);
	}
	try {
		piranha_throw(custom_exception1,);
	} catch (const base_exception &e) {
		BOOST_CHECK_EQUAL(e.what(),empty);
	}
	try {
		piranha_throw(custom_exception1,std::string(""));
	} catch (const base_exception &e) {
		BOOST_CHECK(e.what() != empty);
	}
	try {
		piranha_throw(custom_exception1,"");
	} catch (const base_exception &e) {
		BOOST_CHECK(e.what() != empty);
	}
	try {
		piranha_throw(custom_exception2,);
	} catch (const base_exception &e) {
		BOOST_CHECK(e.what() == empty);
	}
	try {
		piranha_throw(custom_exception2,"");
	} catch (const base_exception &e) {
		BOOST_CHECK(e.what() != empty);
	}
	try {
		piranha_throw(custom_exception2,"",3);
	} catch (const base_exception &e) {
		BOOST_CHECK(e.what() != empty);
	}
	try {
		piranha_throw(custom_exception3,"",3);
	} catch (const base_exception &e) {
		BOOST_CHECK(e.what() != empty);
	}
	try {
		piranha_throw(custom_exception3,3,empty);
	} catch (const base_exception &e) {
		BOOST_CHECK(e.what() == empty);
	}
	try {
		piranha_throw(custom_exception4,"");
	} catch (const base_exception &e) {
		BOOST_CHECK(e.what() == empty);
	}
}