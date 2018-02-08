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

#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>
#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-event-queues/MpxEventQueueRepository.h>

namespace mpx
{

MpxEventQueueRepository* MpxEventQueueRepository::g_mpxEventQueueRepository = new MpxEventQueueRepository ();

MpxEventQueueRepository::MpxEventQueueRepository ()
{
	pthread_mutexattr_t attr;

	pthread_mutexattr_init (&attr);
	pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&m_lock, &attr);
}

MpxEventQueueRepository::~MpxEventQueueRepository ()
{
	mpxset::iterator it;

	for (it = m_mpxset.begin (); it != m_mpxset.end (); ++it)
		free ((void*) it->second);
	m_mpxset.clear ();
}

const char* MpxEventQueueRepository::_RegisterEventQueue (tskmpx_t id, const char* name)
{
	mpxset::iterator it;

	pthread_mutex_lock (&m_lock);
	if ((it = m_mpxset.find (id)) != m_mpxset.end ())
	{
		pthread_mutex_unlock (&m_lock);
		return it->second;
	}

	if (name == 0)
	{
		uuid_t uuid;
		char uuidstr [40];

		uuidstr [0] = '/';
		uuid_generate (uuid);
		uuid_unparse (uuid, uuidstr + 1);

		name = (const char*) uuidstr;
	}

	if ((name = (const char*) strdup (name)) == 0)
	{
		pthread_mutex_unlock (&m_lock);
		return 0;
	}

	m_mpxset [id] = name;
	pthread_mutex_unlock (&m_lock);
	return name;
}

bool MpxEventQueueRepository::_UnregisterEventQueue (tskmpx_t id)
{
	mpxset::iterator it;

	pthread_mutex_lock (&m_lock);
	if ((it = m_mpxset.find (id)) != m_mpxset.end ())
	{
		free ((void*) it->second);
		m_mpxset.erase (it);
		pthread_mutex_unlock (&m_lock);
		return true;
	}
	pthread_mutex_unlock (&m_lock);
	return false;
}

const char* MpxEventQueueRepository::_RetrieveEventQueue (tskmpx_t id)
{
	mpxset::iterator it;
	const char* name;
	const char* dname;

	pthread_mutex_unlock (&m_lock);
	name = ((it = m_mpxset.find (id)) != m_mpxset.end ()) ? it->second : 0;
	dname = (name != 0) ? strdup (name) : 0;
	pthread_mutex_unlock (&m_lock);
	return dname;
}

} /* namespace mpx */
