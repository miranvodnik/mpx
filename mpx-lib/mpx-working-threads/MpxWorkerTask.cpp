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

#include <mpx-working-threads/MpxWorkerTask.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-events/MpxJobFinishedEvent.h>
#include <mpx-events/MpxEvents.h>

namespace mpx
{

EventDescriptor MpxWorkerTask::g_evntab[] =
{
	{ AnyState, MpxTimerEvent::EventCode, TimerEventHandler},
	{ 0, 0, 0}
};

MpxTaskBase::evnset MpxWorkerTask::g_evnset = MpxTaskBase::CreateEventSet(MpxWorkerTask::g_evntab);

MpxWorkerTask::MpxWorkerTask (const char* name) :
	MpxTaskBase (g_evnset, name)
{
	m_getTimer = 0;
	m_sendTimer = 0;
	m_job = 0;
}

MpxWorkerTask::~MpxWorkerTask ()
{
	if (m_getTimer != 0)
		StopTimer (m_getTimer);
	if (m_sendTimer != 0)
		StopTimer (m_sendTimer);
	if (m_job != 0)
		delete m_job;

	m_getTimer = 0;
	m_sendTimer = 0;
	m_job = 0;
}

void MpxWorkerTask::StartTask ()
{
	m_getTimer = StartTimer (GetCurrentTime ());
}

void MpxWorkerTask::StopTask ()
{

}

void MpxWorkerTask::TimerEventHandler (MpxEventBase *event)
{
	MpxTimerEvent* timerEvent = (MpxTimerEvent*) event;

	if (timerEvent == m_getTimer)
	{
		m_getTimer = 0;
		while (true)
		{
			MpxJob* job = 0;

			job = MpxWorkingQueue::Get ();

			if (job == 0)
				continue;

			pthread_cleanup_push(CleanupExecute, this);
				m_job = job;
				job->Execute ();
				Send (job->task (), new MpxJobFinishedEvent (job), false);
				m_job = 0;
				pthread_cleanup_pop(0);

			m_sendTimer = StartTimer (GetCurrentTime ());
			break;
		}
	}
	else if (timerEvent == m_sendTimer)
	{
		m_sendTimer = 0;
		m_getTimer = StartTimer (GetCurrentTime ());
	}
	else
	{

	}
}

void MpxWorkerTask::_CleanupExecute ()
{
	delete m_job;
	m_job = 0;
}

} /* namespace mpx */
