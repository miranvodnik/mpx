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

#pragma once

#include <mpx-tasks/MpxTaskBase.h>
#include <mpx-sockets/MpxSocket.h>
#include <mpx-events/MpxEvents.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

#include <string>
using namespace std;

namespace mpx
{

//
// MpxProxyTaskBase class is used in dynamic casting
//

class MpxProxyTaskBase: public MpxTaskBase
{
protected:
	MpxProxyTaskBase (evnset& e) :
		MpxTaskBase (e)
	{
	}
	virtual ~MpxProxyTaskBase ()
	{
	}
};

template <typename T, typename V> class MpxProxyTask: public MpxProxyTaskBase
{
public:
	typedef T MpxSocketEvent;
	typedef V MpxSocketTask;
	typedef MpxEventXDRItf* (*edfunc) ();
	MpxProxyTask (MpxTaskBase* task, MpxSocket <MpxSocketEvent> * socket, const char* encdeclib, bool client) :
		MpxProxyTaskBase (g_evnset), m_task (task), m_socket (socket), m_encdeclib (encdeclib), m_client (client), m_lib (0), m_fcn (
			0), m_eventXDR (0)
	{
		if (false)
			cout << "create proxy " << this << endl;
	}
	virtual ~MpxProxyTask ()
	{
		if (m_socket != 0)
			delete m_socket;
		m_socket = 0;
		if (m_eventXDR != 0)
			delete m_eventXDR;
		m_eventXDR = 0;
	}
	virtual void StartTask ()
	{
		MpxWorkingQueue::Put (new MpxOpenLibrary (this, m_encdeclib.c_str ()));
	}
	virtual void StopTask ()
	{

	}
	virtual int HandleEvent (MpxEventBase* event)
	{
		if ((int) event->code () < 0)
			return MpxTaskBase::HandleEvent (event);
		if (m_eventXDR == 0)
			return -1;
		size_t xdrSize;
		size_t size = xdrSize = m_eventXDR->Encode (event, 0, 0);
		if (size < 0)
		{
			cout << "EVENT = " << event->code () << ", size = " << size << endl;
		}
		char* buffer = (char*) alloca(size);
		if ((size = m_eventXDR->Encode (event, buffer, size)) < 0)
		{
			cout << "EVENT = " << event->code () << ", size = " << size << endl;
		}
		m_socket->Write ((u_char*) buffer, xdrSize);
		return 0;
	}
	inline MpxTaskBase* task ()
	{
		return m_task;
	}
	inline void* socket ()
	{
		return m_socket;
	}

protected:
	inline static void HandleJobFinishedEvent (MpxEventBase *event, mpx_appdt_t appdata)
	{
		(dynamic_cast <MpxProxyTask*> ((MpxTaskBase*) appdata))->HandleJobFinishedEvent (event);
	}
	void HandleJobFinishedEvent (MpxEventBase *event)
	{
		while (true)
		{
			MpxJobFinishedEvent* jobFinishedEvent = dynamic_cast <MpxJobFinishedEvent*> (event);
			if (jobFinishedEvent == 0)
				break;
			MpxOpenLibrary* openLibrary = dynamic_cast <MpxOpenLibrary*> (jobFinishedEvent->job ());
			if (openLibrary == 0)
				break;
			if ((m_lib = openLibrary->lib ()) == 0)
				break;
			if ((m_fcn = (edfunc) openLibrary->fcn ()) == 0)
				break;
			if ((m_eventXDR = (*m_fcn) ()) == 0)
				break;

			if (Send (m_task, new MpxExternalTaskEvent (EPOLLIN, 0, 0, 0), false) < 0)
				break;

			if (m_client)
			{
			}
			else
			{
				MpxMessage msg;
				msg.m_Code = ExternalTaskReplyCode;
				msg.MpxMessage_u.m_externalTaskReply.task = (long) m_task;
				msg.MpxMessage_u.m_externalTaskReply.encdeclib = strdup (m_encdeclib.c_str ());
				if (m_socket->PostXdrRequest ((xdrproc_t) xdr_MpxMessage, &msg) < 0)
					break;
			}

			return;
		}
		Dispose (false);
	}

protected:
	static EventDescriptor g_evntab[];
	static evnset g_evnset;
	MpxTaskBase* m_task;
	MpxSocket <MpxSocketEvent> * m_socket;
	string m_encdeclib;
	bool m_client;
	void* m_lib;
	edfunc m_fcn;
	MpxEventXDRItf* m_eventXDR;
};

template <typename T, typename V> EventDescriptor MpxProxyTask < T, V > ::g_evntab[] =
{
	{ AnyState, T::EventCode, V::HandleSocketEvent },
	{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
	{ 0, 0, 0 }
};
template <typename T, typename V> MpxTaskBase::evnset MpxProxyTask < T, V > ::g_evnset = MpxTaskBase::CreateEventSet(g_evntab);

} // namespace mpx
