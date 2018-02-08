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

#include <mpx-working-threads/MpxWorkingQueue.h>

namespace mpx
{

MpxWorkingQueue* MpxWorkingQueue::g_workingQueue = new MpxWorkingQueue ();

MpxWorkingQueue::MpxWorkingQueue ()
{
	pthread_mutexattr_t attr;

	pthread_mutexattr_init (&attr);
	pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&m_mutex, &attr);
	pthread_cond_init (&m_cond, 0);
}

MpxWorkingQueue::~MpxWorkingQueue ()
{
	pthread_mutex_destroy (&m_mutex);
	pthread_cond_destroy (&m_cond);
}

int MpxWorkingQueue::_Put (MpxJob* job)
{
	pthread_mutex_lock (&m_mutex);
	m_wqueue.push_back (job);
	pthread_mutex_unlock (&m_mutex);
	pthread_cond_signal (&m_cond);
	return 0;
}

MpxJob* MpxWorkingQueue::_Get ()
{
	MpxJob* job = 0;

	pthread_cleanup_push (Cleanup, this);
		pthread_mutex_lock (&m_mutex);
		do
		{
			if (!m_wqueue.empty ())
			{
				job = m_wqueue.front ();
				m_wqueue.pop_front ();
				break;
			}
		}
		while (pthread_cond_wait (&m_cond, &m_mutex) == 0);
		pthread_mutex_unlock (&m_mutex);
		pthread_cleanup_pop(0);
	return job;
}

void MpxWorkingQueue::_Cleanup ()
{
	pthread_mutex_unlock (&m_mutex);
}

} /* namespace mpx */
