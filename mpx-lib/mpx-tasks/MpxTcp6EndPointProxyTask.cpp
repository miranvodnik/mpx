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

#include <mpx-tasks/MpxTcp6EndPointProxyTask.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

EventDescriptor MpxTcp6EndPointProxyTask::g_evntab [] =
{
	{ AnyState, MpxTcp6EndPointEvent::EventCode, HandleTcp6EndPointEvent },
	{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset MpxTcp6EndPointProxyTask::g_evnset = MpxTaskBase::CreateEventSet(MpxTcp6EndPointProxyTask::g_evntab);

MpxTcp6EndPointProxyTask::MpxTcp6EndPointProxyTask (MpxTaskBase* task, const char* encdeclib, MpxTcp6EndPoint* tcp6EndPoint) :
	MpxProxyTask (g_evnset, task, tcp6EndPoint), m_encdeclib (encdeclib)
{
	tcp6EndPoint->task (this);
}

MpxTcp6EndPointProxyTask::~MpxTcp6EndPointProxyTask ()
{
}

void MpxTcp6EndPointProxyTask::StartTask ()
{
	MpxWorkingQueue::Put (new MpxOpenLibrary (this, m_encdeclib.c_str()));
}

void MpxTcp6EndPointProxyTask::StopTask ()
{

}

void MpxTcp6EndPointProxyTask::HandleTcp6EndPointEvent (MpxEventBase *event)
{
	MpxTcp6EndPointEvent* tcp6EndPointEvent = dynamic_cast <MpxTcp6EndPointEvent*> (event);
	if (tcp6EndPointEvent == 0)
		return;

	MpxTcp6EndPoint* tcp6EndPoint = dynamic_cast <MpxTcp6EndPoint*> ((MpxTcp6EndPoint*) tcp6EndPointEvent->endPoint ());
	if (tcp6EndPoint == 0)
		return;

	switch (tcp6EndPointEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp6EndPointEvent->error () == 0) && (tcp6EndPointEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = tcp6EndPoint->DecodeEvent(m_eventXDR)) != 0)
				Send (m_task, event, false);
			break;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: tcp6 end point output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp6 end point default" << endl;
		break;
	}
}

void MpxTcp6EndPointProxyTask::HandleJobFinishedEvent (MpxEventBase *event)
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
