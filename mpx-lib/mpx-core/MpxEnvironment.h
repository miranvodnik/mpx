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
#include <map>

#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-core/MpxUtilities.h>
#include <mpx-event-queues/MpxMQTaskI.h>

namespace mpx
{

class MpxEnvironment
{
private:
	typedef set <MpxTaskMultiplexer*> tskmpxset;
	typedef set <pthread_t> thrset;
	MpxEnvironment ();
	virtual ~MpxEnvironment ();
public:
	inline static MpxTaskMultiplexer* CreateTaskMultiplexer (const char* connStr = 0, bool isWorker = false)
	{
		return g_mpxEnvironment->_CreateTaskMultiplexer (connStr, isWorker);
	}
	inline static int Start (MpxMQTaskI* mqTask)
	{
		return g_mpxEnvironment->_Start (mqTask);
	}
	inline static int WaitMultiplexerBarrier ()
	{
		return g_mpxEnvironment->_WaitMultiplexerBarrier ();
	}
	inline static int WaitTasksBarrier ()
	{
		return g_mpxEnvironment->_WaitTasksBarrier ();
	}
	inline static void Stop ()
	{
		g_mpxEnvironment->_Stop ();
	}
	inline static void Wait ()
	{
		g_mpxEnvironment->_Wait ();
	}
	inline static int BroadcastExternalEvent (MpxTaskBase* task, MpxEventBase* event)
	{
		return g_mpxEnvironment->_BroadcastExternalEvent (task, event);
	}
private:
	MpxTaskMultiplexer* _CreateTaskMultiplexer (const char* connStr, bool isWorker);
	int _Start (MpxMQTaskI* mqTask);
	int _WaitMultiplexerBarrier ();
	int _WaitTasksBarrier ();
	void _Stop ();
	void _Wait ();
	int _BroadcastExternalEvent (MpxTaskBase* task, MpxEventBase* event);
private:
	static MpxEnvironment* g_mpxEnvironment;
	tskmpxset m_tskmpxset;
	thrset m_thrset;
	pthread_mutex_t m_lock;
	pthread_barrier_t m_barrier;
	pthread_barrier_t m_tbarrier;
	bool m_barrierActive;
};

} /* namespace mpx */
