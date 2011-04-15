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

#ifndef PIRANHA_MONOMIAL_HPP
#define PIRANHA_MONOMIAL_HPP

#include <boost/concept/assert.hpp>
#include <initializer_list>
#include <unordered_set>
#include <vector>

#include "array_key.hpp"
#include "config.hpp"
#include "symbol.hpp"

namespace piranha
{

/// Monomial class.
/**
 * This class extends piranha::array_key to define a series key type suitable as monomial in polynomial terms.
 * This class is a model of the piranha::concept::Key concept.
 * 
 * \section type_requirements Type requirements
 * 
 * \p T must be suitable for use in piranha::array_key.
 * 
 * \section exception_safety Exception safety guarantee
 * 
 * This class provides the same exception safety guarantee as piranha::array_key.
 * 
 * \section move_semantics Move semantics
 * 
 * Move semantics is equivalent to piranha::array_key's move semantics.
 * 
 * @author Francesco Biscani (bluescarni@gmail.com)
 */
template <typename T>
class monomial: public array_key<T,monomial<T>>
{
		typedef array_key<T,monomial<T>> base;
	public:
		/// Defaulted default constructor.
		monomial() = default;
		/// Defaulted copy constructor.
		monomial(const monomial &) = default;
		/// Defaulted move constructor.
		monomial(monomial &&) = default;
		/// Constructor from initializer list.
		/**
		 * @param[in] list initializer list.
		 * 
		 * @see piranha::array_key's constructor from initializer list.
		 */
		monomial(std::initializer_list<T> list):base(list) {}
		/// Forwarding constructor.
		/**
		 * Will perfectly forward the arguments to a corresponding constructor in piranha::array_key.
		 * 
		 * @param[in]  params arguments to be forwarded to the constructor.
		 * 
		 * @throws unspecified any exception thrown by the invoked constructor from piranha::array_key.
		 */
		template <typename... Args>
		explicit monomial(Args && ... params):base(std::forward<Args>(params)...) {}
		/// Defaulted destructor.
		~monomial() = default;
		/// Defaulted copy assignment operator.
		monomial &operator=(const monomial &) = default;
		// NOTE: this can be defaulted in GCC >= 4.6.
		/// Move assignment operator.
		/**
		 * @param[in] other object to move from.
		 * 
		 * @return reference to \p this.
		 */
		monomial &operator=(monomial &&other) piranha_noexcept_spec(true)
		{
			base::operator=(std::move(other));
			return *this;
		}
		/// Compatibility check
		/**
		 * A monomial and a vector of arguments are compatible if their sizes coincide.
		 * 
		 * @param[in] args reference arguments vector.
		 * 
		 * @return <tt>this->size() == args.size()</tt>.
		 */
		bool is_compatible(const std::vector<symbol> &args) const
		{
			return (this->size() == args.size());
		}
		/// Ignorability check.
		/**
		 * A monomial is never ignorable by definition.
		 * 
		 * @param[in] args reference arguments vector.
		 * 
		 * @return \p false.
		 */
		bool is_ignorable(const std::vector<symbol> &args) const
		{
			(void)args;
			piranha_assert(is_compatible(args));
			return false;
		}
		/// Merge arguments.
		/**
		 * Will forward the call to piranha::array_key::base_merge_args().
		 * 
		 * @param[in] orig_args original arguments vector.
		 * @param[in] new_args new arguments vector.
		 * 
		 * @return piranha::monomial with the new arguments merged in.
		 * 
		 * @throws unspecified any exception thrown by piranha::array_key::base_merge_args().
		 */
		monomial merge_args(const std::vector<symbol> &orig_args, const std::vector<symbol> &new_args) const
		{
			monomial retval;
			static_cast<base &>(retval) = this->base_merge_args(orig_args,new_args);
			return retval;
		}
};

}

namespace std
{

/// Specialisation of \p std::hash for piranha::monomial.
/**
 * Functionally equivalent to the \p std::hash specialisation for piranha::array_key.
 */
template <typename T>
struct hash<piranha::monomial<T>>: public hash<piranha::array_key<T,piranha::monomial<T>>> {};

}

#endif
