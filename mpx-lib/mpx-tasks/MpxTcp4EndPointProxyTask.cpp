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

#include <mpx-tasks/MpxTcp4EndPointProxyTask.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

EventDescriptor MpxTcp4EndPointProxyTask::g_evntab[] =
{
	{ AnyState, MpxTcp4EndPointEvent::EventCode, HandleTcp4EndPointEvent },
	{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset MpxTcp4EndPointProxyTask::g_evnset = MpxTaskBase::CreateEventSet(MpxTcp4EndPointProxyTask::g_evntab);

MpxTcp4EndPointProxyTask::MpxTcp4EndPointProxyTask (MpxTaskBase* task, const char* encdeclib, MpxTcp4EndPoint* tcp4EndPoint) :
	MpxProxyTask (g_evnset, task, tcp4EndPoint), m_encdeclib (encdeclib)
{
	tcp4EndPoint->task (this);
}

MpxTcp4EndPointProxyTask::~MpxTcp4EndPointProxyTask ()
{
}

void MpxTcp4EndPointProxyTask::StartTask ()
{
	MpxWorkingQueue::Put (new MpxOpenLibrary (this, m_encdeclib.c_str()));
}

void MpxTcp4EndPointProxyTask::StopTask ()
{

}

void MpxTcp4EndPointProxyTask::HandleTcp4EndPointEvent (MpxEventBase *event)
{
	MpxTcp4EndPointEvent* tcp4EndPointEvent = dynamic_cast <MpxTcp4EndPointEvent*> (event);
	if (tcp4EndPointEvent == 0)
		return;

	MpxTcp4EndPoint* tcp4EndPoint = dynamic_cast <MpxTcp4EndPoint*> ((MpxTcp4EndPoint*) tcp4EndPointEvent->endPoint ());
	if (tcp4EndPoint == 0)
		return;

	switch (tcp4EndPointEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp4EndPointEvent->error () == 0) && (tcp4EndPointEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = tcp4EndPoint->DecodeEvent(m_eventXDR)) != 0)
				Send (m_task, event, false);
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: tcp4 end point output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp4 end point default" << endl;
		break;
	}
}

void MpxTcp4EndPointProxyTask::HandleJobFinishedEvent (MpxEventBase *event)
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
	Dispose(false);
}

} // namespace mpx
