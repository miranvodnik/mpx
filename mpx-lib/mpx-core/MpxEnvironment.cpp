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

#include <mpx-core/MpxEnvironment.h>
#include <mpx-core/MpxXDRProcRegistry.h>
#include <mpx-event-queues/MpxLocalMQTask.h>
#include <mpx-working-threads/MpxWorkerTask.h>
#include <mpx-events/MpxEvents.h>

namespace mpx
{

MpxEnvironment* MpxEnvironment::g_mpxEnvironment = new MpxEnvironment ();

MpxEnvironment::MpxEnvironment ()
{
	pthread_mutexattr_t attr;

	pthread_mutexattr_init (&attr);
	pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&m_lock, &attr);

	m_barrierActive = false;
}

MpxEnvironment::~MpxEnvironment ()
{
}

MpxTaskMultiplexer* MpxEnvironment::_CreateTaskMultiplexer (const char* connStr, bool isWorker)
{
	int fd [2];
	MpxTaskMultiplexer * mpx;

	pthread_mutex_lock (&m_lock);
	pipe (fd);
	mpx = new MpxTaskMultiplexer (fd, connStr, isWorker);
	if (mpx != 0)
		m_tskmpxset.insert (mpx);
	pthread_mutex_unlock (&m_lock);

	return mpx;
}

int MpxEnvironment::_Start (MpxMQTaskI* mqTask)
{
	MpxXDRProcRegistry::Register (MpxExternalTaskEvent::g_xdrId, (xdrproc_t) xdr_MpxEventStruct,
		MpxExternalTaskEvent::XdrAllocator, MpxExternalTaskEvent::ObjAllocator);

	for (int i = 0; i < 4; ++i)
	{
		MpxTaskMultiplexer* mpx = MpxEnvironment::CreateTaskMultiplexer (0, true);
		MpxWorkerTask* task = new MpxWorkerTask ();
		mpx->RegisterTask (task);
	}

	pthread_mutex_lock (&m_lock);
	m_barrierActive = true;
	pthread_barrier_init (&m_barrier, 0, m_tskmpxset.size () + 1);
	pthread_barrier_init (&m_tbarrier, 0, m_tskmpxset.size () + 1);

	for (tskmpxset::iterator it = m_tskmpxset.begin (); it != m_tskmpxset.end (); ++it)
	{
		(*it)->mqImplTask (mqTask);
		(*it)->Start ();
		m_thrset.insert ((*it)->getHandle ());
	}

	pthread_mutex_unlock (&m_lock);
	pthread_barrier_wait (&m_barrier);
	pthread_barrier_wait (&m_tbarrier);
	m_barrierActive = false;
	return 0;
}

int MpxEnvironment::_WaitMultiplexerBarrier ()
{
	pthread_mutex_lock (&m_lock);
	if (!m_barrierActive)
	{
		pthread_mutex_unlock (&m_lock);
		return -1;
	}
	pthread_mutex_unlock (&m_lock);
	pthread_barrier_wait (&m_barrier);
	return 0;
}

int MpxEnvironment::_WaitTasksBarrier ()
{
	pthread_mutex_lock (&m_lock);
	if (!m_barrierActive)
	{
		pthread_mutex_unlock (&m_lock);
		return -1;
	}
	pthread_mutex_unlock (&m_lock);
	pthread_barrier_wait (&m_tbarrier);
	return 0;
}

void MpxEnvironment::_Stop ()
{
	pthread_mutex_lock (&m_lock);

	MpxTaskBase::DisableSend ();

	for (tskmpxset::iterator it = m_tskmpxset.begin (); it != m_tskmpxset.end (); ++it)
		(*it)->Stop ();

	for (thrset::iterator it = m_thrset.begin (); it != m_thrset.end (); ++it)
		pthread_join (*it, 0);

	for (tskmpxset::iterator it = m_tskmpxset.begin (); it != m_tskmpxset.end (); ++it)
		delete (*it);

	m_tskmpxset.clear ();

	pthread_mutex_unlock (&m_lock);
}

void MpxEnvironment::_Wait ()
{
	for (thrset::iterator it = m_thrset.begin (); it != m_thrset.end (); ++it)
		pthread_join (*it, 0);
}

int MpxEnvironment::_BroadcastExternalEvent (MpxTaskBase* task, MpxEventBase* event)
{
	int count = 0;

	pthread_mutex_lock (&m_lock);

	for (tskmpxset::iterator it = m_tskmpxset.begin (); it != m_tskmpxset.end (); ++it, ++count)
	{
		(*it)->externalTask ()->Send (task, event->Copy ());
	}
	delete event;

	pthread_mutex_unlock (&m_lock);

	return count;
}

} /* namespace mpx */
