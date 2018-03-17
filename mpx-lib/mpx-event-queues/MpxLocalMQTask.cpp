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

#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-event-queues/MpxEventQueueRepository.h>
#include <mpx-event-queues/MpxLocalMQTask.h>
#include <string>
using namespace std;

namespace mpx
{

const char* MpxLocalMQTask::g_localPath = "/var/run/";

/*! event table for every instance of this class
 */
EventDescriptor MpxLocalMQTask::g_evntab[] =
{
	{ AnyState, MpxLocalListenerEvent::EventCode, LocalListenerEventHandler },
	{ AnyState, MpxLocalEndPointEvent::EventCode, LocalEndPointEventHandler },
	{ AnyState, MpxLocalClientEvent::EventCode, LocalClientEventHandler },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset MpxLocalMQTask::g_evnset = MpxTaskBase::CreateEventSet(MpxLocalMQTask::g_evntab);

MpxLocalMQTask::MpxLocalMQTask () :
	MpxMQTaskI (g_evnset)
{
	m_listener = 0;
}

MpxLocalMQTask::~MpxLocalMQTask ()
{
	Release ();
}

MpxMQTaskI* MpxLocalMQTask::Copy (tskmpx_t mpx)
{
	MpxLocalMQTask* localTask = new MpxLocalMQTask ();
	localTask->mpx (mpx);
	return localTask;
}

void MpxLocalMQTask::UnlinkMQ (tskmpx_t mpx)
{
	const char* name = MpxEventQueueRepository::RetrieveEventQueue (this);
	MpxEventQueueRepository::UnregisterEventQueue (this);
	if (name == 0)
		return;

	string localPath = g_localPath;
	localPath += name;
	free ((void*) name);
	unlink (localPath.c_str ());
}

void MpxLocalMQTask::Release ()
{
	if (m_listener != 0)
		delete m_listener;
	m_listener = 0;
	for (epset::iterator it = m_epset.begin (); it != m_epset.end (); ++it)
		delete *it;
	m_epset.clear ();
	for (clnset::iterator it = m_clnset.begin (); it != m_clnset.end (); ++it)
		delete it->second;
	m_clnset.clear ();
	UnlinkMQ (mpx ());
}

MpxLocalClient* MpxLocalMQTask::Connect (tskmpx_t mpx)
{
	if (this == 0)
		return 0;

	clnset::iterator it = m_clnset.find (mpx);
	if (it != m_clnset.end ())
		return it->second;

	MpxLocalClient* localClient = new MpxLocalClient (this, true, -1, true);

	if (localClient == 0)
		return 0;

	const char* name = MpxEventQueueRepository::RetrieveEventQueue (mpx);
	if (name == 0)
		return 0;

	string localPath = g_localPath;
	localPath += name;
	free ((void*) name);
	if (localClient->Connect (localPath.c_str ()) != 0)
	{
		delete localClient;
		return 0;
	}

	return m_clnset [mpx] = localClient;
}

int MpxLocalMQTask::MQSend (tskmpx_t mpx, MpxEventBase* event)
{
	MpxLocalClient* dstq = Connect (mpx);

	if (dstq == 0)
		return -1;

	struct msg
	{
		MpxEventBase* event;
	} msg;
	msg.event = event;
	return dstq->Write ((u_char*) &msg, sizeof msg);
}

void MpxLocalMQTask::StartTask ()
{
	const char* name = MpxEventQueueRepository::RetrieveEventQueue ((tskmpx_t) mpx ());
	if (name == 0)
		return;

	if ((m_listener = new MpxLocalListener (this, true)) == 0)
	{
		free ((void*) name);
		return;
	}

	string localPath = g_localPath;
	localPath += name;
	free ((void*) name);
	if (m_listener->CreateListener (localPath.c_str ()) != 0)
	{
		delete m_listener;
		m_listener = 0;
		return;
	}

	if (m_listener->StartListener () < 0)
	{
		delete m_listener;
		m_listener = 0;
		return;
	}
}

void MpxLocalMQTask::StopTask ()
{
	Release ();
}

void MpxLocalMQTask::LocalListenerEventHandler (MpxEventBase *event)
{
	MpxLocalListenerEvent* localListenerEvent = dynamic_cast <MpxLocalListenerEvent*> (event);
	MpxLocalEndPoint* localEndPoint = new MpxLocalEndPoint (this, localListenerEvent->fd (), true, -1, true);
	if (localEndPoint->endPoint () <= 0)
	{
		delete localEndPoint;
		return;
	}

	if (false)
		cout << "CREATED LOCAL END POINT" << endl;
	m_epset.insert (localEndPoint);
}

void MpxLocalMQTask::LocalEndPointEventHandler (MpxEventBase *event)
{
	MpxLocalEndPointEvent* localEndPointEvent = dynamic_cast <MpxLocalEndPointEvent*> (event);
	MpxLocalEndPoint* localEndPoint =
		dynamic_cast <MpxLocalEndPoint*> ((MpxLocalEndPoint*) localEndPointEvent->endPoint ());

	if (false)
		cout << "LOCAL END POINT EVENT: FLAGS = " << localEndPointEvent->flags () << ", ERROR = "
			<< localEndPointEvent->error () << ", SIZE = " << localEndPointEvent->size () << ", BUFFER = "
			<< ((void*) localEndPointEvent->buffer ()) << endl;
	switch (localEndPointEvent->flags ())
	{
	case 0:
		if (false)
			cout << "LOCAL END POINT EVENT: TIMEOUT" << endl;
		break;
	case EPOLLIN:
		if (false)
			cout << "LOCAL END POINT EVENT: INPUT" << endl;
		if ((localEndPointEvent->error () == 0) && (localEndPointEvent->size () != 0))
		{
			size_t size = localEndPointEvent->size ();
			u_char* data = localEndPoint->fast () ? localEndPoint->FastRead (size) : localEndPointEvent->buffer ();
			struct msg
			{
				MpxEventBase* event;
			} msg;
			for (; size > 0; size -= sizeof msg, data += sizeof msg)
			{
				msg = *((struct msg*) data);
				if (msg.event->Invoke () != 0)
					cout << "INVOKE FAILED" << endl;
				delete msg.event;
			}
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "LOCAL END POINT EVENT: OUTPUT" << endl;
		if ((localEndPointEvent->error () == 0) && (localEndPointEvent->size () != 0))
			return;
		break;
	default:
		if (false)
			cout << "LOCAL END POINT EVENT: UNKNOWN" << endl;
		break;
	}
	if (false)
		cout << "LOCAL END POINT ERASED" << endl;
	epset::iterator it = m_epset.find (localEndPoint);
	if (it != m_epset.end ())
		m_epset.erase (it);
	delete localEndPoint;
}

void MpxLocalMQTask::LocalClientEventHandler (MpxEventBase *event)
{
	MpxLocalClientEvent* localClientEvent = dynamic_cast <MpxLocalClientEvent*> (event);

	if (false)
		cout << "LOCAL CLIENT EVENT: FLAGS = " << localClientEvent->flags () << ", ERROR = "
			<< localClientEvent->error () << ", SIZE = " << localClientEvent->size () << ", BUFFER = "
			<< ((void*) localClientEvent->buffer ()) << endl;
	switch (localClientEvent->flags ())
	{
	case 0:
		if (false)
			cout << "LOCAL CLIENT EVENT: TIMEOUT" << endl;
		break;
	case EPOLLIN:
		if (false)
			cout << "LOCAL CLIENT EVENT: INPUT" << endl;
		if ((localClientEvent->error () == 0) && (localClientEvent->size () != 0))
			return;
		break;
	case EPOLLOUT:
		if (false)
			cout << "LOCAL CLIENT EVENT: OUTPUT" << endl;
		if ((localClientEvent->error () == 0) && (localClientEvent->size () != 0))
			return;
		break;
	default:
		if (false)
			cout << "LOCAL CLIENT EVENT: UNKNOWN" << endl;
		break;
	}
}

} /* namespace mpx */
