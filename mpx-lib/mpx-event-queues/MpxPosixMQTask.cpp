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

#include <mpx-event-queues/MpxEventQueueRepository.h>
#include <mpx-event-queues/MpxPosixMQTask.h>

namespace mpx
{

EventDescriptor MpxPosixMQTask::g_evntab[] =
{
	{ AnyState, MpxPosixMQEvent::EventCode , PosixMQEventHandler },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset MpxPosixMQTask::g_evnset = MpxTaskBase::CreateEventSet(MpxPosixMQTask::g_evntab);

MpxPosixMQTask::MpxPosixMQTask () :
	MpxMQTaskI (g_evnset)
{
	m_listener = 0;
}

MpxPosixMQTask::~MpxPosixMQTask ()
{
	Release ();
}

MpxMQTaskI* MpxPosixMQTask::Copy (tskmpx_t mpx)
{
	MpxPosixMQTask* task = new MpxPosixMQTask ();
	task->mpx (mpx);
	return task;
}

void MpxPosixMQTask::UnlinkMQ (tskmpx_t mpx)
{
	const char* name = MpxEventQueueRepository::RetrieveEventQueue (this);
	MpxEventQueueRepository::UnregisterEventQueue (this);
	if (name == 0)
		return;

	mq_unlink (name);
	free ((void*) name);
}

void MpxPosixMQTask::Release ()
{
	if (m_listener != 0)
		delete m_listener;
	m_listener = 0;
	for (mqset::iterator it = m_mqset.begin (); it != m_mqset.end (); ++it)
		delete it->second;
	m_mqset.clear ();
	UnlinkMQ (mpx ());
}

MpxPosixMQ* MpxPosixMQTask::Connect (tskmpx_t mpx)
{
	if (this == 0)
		return 0;

	mqset::iterator it = m_mqset.find (mpx);
	if (it != m_mqset.end ())
		return it->second;

	MpxPosixMQ* posixMQ = new MpxPosixMQ (this, true, -1);

	if (posixMQ == 0)
		return 0;

	const char* name = MpxEventQueueRepository::RetrieveEventQueue (mpx);
	if (name == 0)
	{
		delete posixMQ;
		return 0;
	}

	if (posixMQ->Connect (name, sizeof(MpxEventBase*)) != 0)
	{
		free ((void*) name);
		delete posixMQ;
		return 0;
	}
	free ((void*) name);

	if (posixMQ->Start () < 0)
	{
		delete posixMQ;
		return 0;
	}

	return m_mqset [mpx] = posixMQ;
}

int MpxPosixMQTask::MQSend (tskmpx_t mpx, MpxEventBase* event)
{
	MpxPosixMQ* dstq = Connect (mpx);

	if (dstq == 0)
		return -1;

	struct msg
	{
		MpxEventBase* event;
	}*msg = new struct msg;
	msg->event = event;
	return dstq->Write ((u_char*) msg, sizeof *msg);
}

void MpxPosixMQTask::StartTask ()
{
	if ((m_listener = new MpxPosixMQ (this, true, -1)) == 0)
		return;

	const char* name = MpxEventQueueRepository::RetrieveEventQueue (this->mpx ());
	if (name == 0)
	{
		delete m_listener;
		m_listener = 0;
		return;
	}

	if (m_listener->Create (name, sizeof(MpxEventBase*)) < 0)
	{
		delete m_listener;
		m_listener = 0;
		free ((void*) name);
		return;
	}
	free ((void*) name);

	if (m_listener->Start () < 0)
	{
		delete m_listener;
		m_listener = 0;
		return;
	}
}

void MpxPosixMQTask::StopTask ()
{
	Release ();
}

void MpxPosixMQTask::PosixMQEventHandler (MpxEventBase* event)
{
	MpxPosixMQEvent* posixMQEvent = (MpxPosixMQEvent*) event;

	switch (posixMQEvent->flags ())
	{
	case 0:
		break;
	case EPOLLIN:
		if ((posixMQEvent->error () == 0) && (posixMQEvent->size () != 0))
		{
			struct msg
			{
				MpxEventBase* event;
			} msg = *((struct msg*) posixMQEvent->buffer ());

			msg.event->Invoke ();
			delete msg.event;
			return;
		}
		break;
	case EPOLLOUT:
		if ((posixMQEvent->error () == 0) && (posixMQEvent->size () != 0))
		{
			if (false)
				cout << "POSIX MQ OUT" << endl;
			return;
		}
		break;
	default:
		break;
	}
}

} /* namespace mpx */
