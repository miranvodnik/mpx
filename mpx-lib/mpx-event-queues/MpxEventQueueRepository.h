//    Event Driven Task Multiplexing Library
//    Copyright (C) 2018 Miran Vodnik
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
//    contact: miran.vodnik@siol.net

#pragma once

#include <pthread.h>
#include <mpx-core/MpxUtilities.h>
#include <map>
using namespace std;

namespace mpx
{

typedef void* tskmpx_t;

/*! repository of event waiting queue names
 *
 * this class implements repository of waiting queue names associated with
 * task multiplexers. This repository is implemented in the form of an STL
 * dictionary. Its keys are opaque references to objects representing task
 * multiplexers. Knowing these keys one can easily retrieve name of  waiting
 * queue associated with arbitrary task multiplexer in system. Instances of
 * this class cannot be created in the normal way, since constructor of this
 * class is private. The only one instance of this class is created by c++
 * running environment at application startup. This object is accessed by
 * public static methods of this class to expose its functionality, which
 * is quite simple:
 * - name of event queue should be registered (inserted) within repository
 * - name of event queue should be removed from repository
 * - name of event queue should be retrieved from repository.
 */
class MpxEventQueueRepository
{
private:
	/*! dictionary of waiting queue names
	 *
	 * @param key - opaque reference to task multiplexer asociated with
	 * waiting queue, having name addressed by second parameter
	 * @param value - character string containing name of waiting queue
	 */
	typedef map <tskmpx_t, const char*> mpxset;
	MpxEventQueueRepository ();
	virtual ~MpxEventQueueRepository ();
public:

	/*! event queue name registration
	 *
	 * function transfers control to the member function _RegisterEventQueue of
	 * single global instance of this class.
	 *
	 * @param id - opaque reference to task multiplexer object
	 * @param name - address of waiting queue name associated with task multiplexer
	 * @return **name** - if parameter name references non null string, this same
	 * reference is returned to the caller
	 * @return **generated name** - if parameter name references null string, a
	 * reference to generated name string is returned
	 */
	inline static const char* RegisterEventQueue (tskmpx_t id, const char* name = 0)
	{
		return g_mpxEventQueueRepository->_RegisterEventQueue (id, name);
	}

	/*! event queue name unregistration
	 *
	 * function transfers control to the member function _UnregisterEventQueue of
	 * single global instance of this class.
	 *
	 * @param id - opaque reference to task multiplexer object
	 * @return **true** - waiting queue name has been unregistered
	 * @return **false** - waiting queue name cannot be unregistered probably
	 * because dictionary does not contain it
	 */
	inline static bool UnregisterEventQueue (tskmpx_t id)
	{
		return g_mpxEventQueueRepository->_UnregisterEventQueue (id);
	}

	/*! retrieve waiting queue name
	 *
	 * function transfers control to the member function _RetrieveEventQueue of
	 * single global instance of this class.
	 *
	 * @param id - opaque reference to task multiplexer object
	 * @return **name** - reference to waiting queue name string
	 * @return **0** - waiting queue name cannot be retrieved probably because
	 * dictionary does not contain it
	 */
	inline static const char* RetrieveEventQueue (tskmpx_t id)
	{
		return g_mpxEventQueueRepository->_RetrieveEventQueue (id);
	}
private:
	const char* _RegisterEventQueue (tskmpx_t id, const char* name = 0);
	bool _UnregisterEventQueue (tskmpx_t id);
	const char* _RetrieveEventQueue (tskmpx_t id);
private:
	static MpxEventQueueRepository* g_mpxEventQueueRepository; //!< single instance of this class
	pthread_mutex_t m_lock; //!< locking object to synchronize access to dictionary
	mpxset m_mpxset; //!< dctionary of waiting queue names
};

} /* namespace mpx */
