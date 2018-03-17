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

#include <mpx-tasks/MpxLocalClientProxyTask.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

EventDescriptor MpxLocalClientProxyTask::g_evntab[] =
{
	{ AnyState, MpxLocalClientEvent::EventCode, HandleLocalClientEvent },
	{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset MpxLocalClientProxyTask::g_evnset = MpxTaskBase::CreateEventSet(MpxLocalClientProxyTask::g_evntab);

MpxLocalClientProxyTask::MpxLocalClientProxyTask (MpxTaskBase* task, const char* encdeclib, MpxLocalClient* localClient) :
	MpxProxyTask (g_evnset, task, localClient), m_encdeclib (encdeclib)
{
	localClient->task (this);
}

MpxLocalClientProxyTask::~MpxLocalClientProxyTask ()
{
}

void MpxLocalClientProxyTask::StartTask ()
{
	MpxWorkingQueue::Put (new MpxOpenLibrary (this, m_encdeclib.c_str()));
}

void MpxLocalClientProxyTask::StopTask ()
{

}

void MpxLocalClientProxyTask::HandleLocalClientEvent (MpxEventBase *event)
{
	MpxLocalClientEvent* localClientEvent = dynamic_cast <MpxLocalClientEvent*> (event);
	if (localClientEvent == 0)
		return;

	MpxLocalClient* localClient = dynamic_cast <MpxLocalClient*> ((MpxLocalClient*) localClientEvent->endPoint ());
	if (localClient == 0)
		return;

	switch (localClientEvent->flags ())
	{
	case EPOLLIN:
		if ((localClientEvent->error () == 0) && (localClientEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = localClient->DecodeEvent(m_eventXDR)) != 0)
				Send (m_task, event, false);
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: local client output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: local client default" << endl;
		break;
	}
}

void MpxLocalClientProxyTask::HandleJobFinishedEvent (MpxEventBase *event)
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
