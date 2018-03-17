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

#include <mpx-tasks/MpxLocalEndPointProxyTask.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

EventDescriptor MpxLocalEndPointProxyTask::g_evntab[] =
{
	{ AnyState, MpxLocalEndPointEvent::EventCode, HandleLocalEndPointEvent},
	{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
	{ 0, 0, 0}
};

MpxTaskBase::evnset MpxLocalEndPointProxyTask::g_evnset = MpxTaskBase::CreateEventSet(MpxLocalEndPointProxyTask::g_evntab);

MpxLocalEndPointProxyTask::MpxLocalEndPointProxyTask (MpxTaskBase* task, const char* encdeclib, MpxLocalEndPoint* localEndPoint) :
	MpxProxyTask (g_evnset, task, localEndPoint), m_encdeclib (encdeclib)
{
	localEndPoint->task (this);
}

MpxLocalEndPointProxyTask::~MpxLocalEndPointProxyTask ()
{
}

void MpxLocalEndPointProxyTask::StartTask ()
{
	MpxWorkingQueue::Put (new MpxOpenLibrary (this, m_encdeclib.c_str()));
}

void MpxLocalEndPointProxyTask::StopTask ()
{

}

void MpxLocalEndPointProxyTask::HandleLocalEndPointEvent (MpxEventBase *event)
{
	MpxLocalEndPointEvent* localEndPointEvent = dynamic_cast <MpxLocalEndPointEvent*> (event);
	if (localEndPointEvent == 0)
		return;

	MpxLocalEndPoint* localEndPoint =
		dynamic_cast <MpxLocalEndPoint*> ((MpxLocalEndPoint*) localEndPointEvent->endPoint ());
	if (localEndPoint == 0)
		return;

	switch (localEndPointEvent->flags ())
	{
	case EPOLLIN:
		if ((localEndPointEvent->error () == 0) && (localEndPointEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = localEndPoint->DecodeEvent(m_eventXDR)) != 0)
				Send (m_task, event, false);
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: local end point output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: local end point default" << endl;
		break;
	}
}

void MpxLocalEndPointProxyTask::HandleJobFinishedEvent (MpxEventBase *event)
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

		MpxMessage msg;
		msg.m_Code = ExternalTaskReplyCode;
		msg.MpxMessage_u.m_externalTaskReply.task = (long) m_task;
		msg.MpxMessage_u.m_externalTaskReply.encdeclib = strdup (m_encdeclib.c_str());
		if (m_socket->PostXdrRequest ((xdrproc_t) xdr_MpxMessage, &msg) < 0)
			break;

		return;
	}
	Dispose (false);
}

} // namespace mpx
