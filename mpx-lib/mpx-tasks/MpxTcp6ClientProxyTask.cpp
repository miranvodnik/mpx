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

#include <mpx-tasks/MpxTcp6ClientProxyTask.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

EventDescriptor MpxTcp6ClientProxyTask::g_evntab[] =
{
	{ AnyState, MpxTcp6ClientEvent::EventCode, HandleTcp6ClientEvent },
	{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset MpxTcp6ClientProxyTask::g_evnset = MpxTaskBase::CreateEventSet(MpxTcp6ClientProxyTask::g_evntab);

MpxTcp6ClientProxyTask::MpxTcp6ClientProxyTask (MpxTaskBase* task, const char* encdeclib, MpxTcp6Client* tcp6Client) :
	MpxProxyTask (g_evnset, task, tcp6Client), m_encdeclib (encdeclib)
{
	tcp6Client->task (this);
}

MpxTcp6ClientProxyTask::~MpxTcp6ClientProxyTask ()
{
}

void MpxTcp6ClientProxyTask::StartTask ()
{
	MpxWorkingQueue::Put (new MpxOpenLibrary (this, m_encdeclib.c_str()));
}

void MpxTcp6ClientProxyTask::StopTask ()
{

}

void MpxTcp6ClientProxyTask::HandleTcp6ClientEvent (MpxEventBase *event)
{
	MpxTcp6ClientEvent* tcp6ClientEvent = dynamic_cast <MpxTcp6ClientEvent*> (event);
	if (tcp6ClientEvent == 0)
		return;

	MpxTcp6Client* tcp6Client = dynamic_cast <MpxTcp6Client*> ((MpxTcp6Client*) tcp6ClientEvent->endPoint ());
	if (tcp6Client == 0)
		return;

	switch (tcp6ClientEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp6ClientEvent->error () == 0) && (tcp6ClientEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = tcp6Client->DecodeEvent(m_eventXDR)) != 0)
				Send (m_task, event, false);
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: tcp6 client output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp6 client default" << endl;
		break;
	}
}

void MpxTcp6ClientProxyTask::HandleJobFinishedEvent (MpxEventBase *event)
{
	while (true)
	{
		MpxJobFinishedEvent* jobFinishedEvent = dynamic_cast < MpxJobFinishedEvent* > (event);
		if (jobFinishedEvent == 0)
			break;
		MpxOpenLibrary* openLibrary = dynamic_cast < MpxOpenLibrary* > (jobFinishedEvent->job());
		if (openLibrary == 0)
			break;
		if ((m_lib = openLibrary->lib()) == 0)
			break;
		if ((m_fcn = (edfunc) openLibrary->fcn()) == 0)
			break;
		if ((m_eventXDR = (*m_fcn) ()) == 0)
			break;
		Send (m_task, new MpxExternalTaskEvent (EPOLLIN, 0, 0, 0), false);
		return;
	}
	Dispose (false);
}

} // namespace mpx
