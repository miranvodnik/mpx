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

#include <mpx-tasks/MpxTcp4ClientProxyTask.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

EventDescriptor MpxTcp4ClientProxyTask::g_evntab[] =
{
	{ AnyState, MpxTcp4ClientEvent::EventCode, HandleTcp4ClientEvent },
	{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset MpxTcp4ClientProxyTask::g_evnset = MpxTaskBase::CreateEventSet(MpxTcp4ClientProxyTask::g_evntab);

MpxTcp4ClientProxyTask::MpxTcp4ClientProxyTask (MpxTaskBase* task, const char* encdeclib, MpxTcp4Client* tcp4Client) :
	MpxProxyTask (g_evnset, task, tcp4Client), m_encdeclib (encdeclib)
{
	tcp4Client->task (this);
}

MpxTcp4ClientProxyTask::~MpxTcp4ClientProxyTask ()
{
}

void MpxTcp4ClientProxyTask::StartTask ()
{
	MpxWorkingQueue::Put (new MpxOpenLibrary (this, m_encdeclib.c_str()));
}

void MpxTcp4ClientProxyTask::StopTask ()
{

}

void MpxTcp4ClientProxyTask::HandleTcp4ClientEvent (MpxEventBase *event)
{
	MpxTcp4ClientEvent* tcp4ClientEvent = dynamic_cast <MpxTcp4ClientEvent*> (event);
	if (tcp4ClientEvent == 0)
		return;

	MpxTcp4Client* tcp4Client = dynamic_cast <MpxTcp4Client*> ((MpxTcp4Client*) tcp4ClientEvent->endPoint ());
	if (tcp4Client == 0)
		return;

	switch (tcp4ClientEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp4ClientEvent->error () == 0) && (tcp4ClientEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = tcp4Client->DecodeEvent(m_eventXDR)) != 0)
				Send (m_task, event, false);
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: tcp4 client output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp4 client default" << endl;
		break;
	}
}

void MpxTcp4ClientProxyTask::HandleJobFinishedEvent (MpxEventBase *event)
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
