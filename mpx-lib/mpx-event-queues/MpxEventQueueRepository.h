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

class MpxEventQueueRepository
{
private:
	typedef map <tskmpx_t, const char*> mpxset;
	MpxEventQueueRepository ();
	virtual ~MpxEventQueueRepository ();
public:
	inline static const char* RegisterEventQueue (tskmpx_t id, const char* name = 0)
	{
		return g_mpxEventQueueRepository->_RegisterEventQueue (id, name);
	}
	inline static bool UnregisterEventQueue (tskmpx_t id)
	{
		return g_mpxEventQueueRepository->_UnregisterEventQueue (id);
	}
	inline static const char* RetrieveEventQueue (tskmpx_t id)
	{
		return g_mpxEventQueueRepository->_RetrieveEventQueue (id);
	}
private:
	const char* _RegisterEventQueue (tskmpx_t id, const char* name = 0);
	bool _UnregisterEventQueue (tskmpx_t id);
	const char* _RetrieveEventQueue (tskmpx_t id);
private:
	static MpxEventQueueRepository* g_mpxEventQueueRepository;
	pthread_mutex_t m_lock;
	mpxset m_mpxset;
};

} /* namespace mpx */
