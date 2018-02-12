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

namespace mpx
{

//
// MpxProxyTaskBase class is used in dynamic casting
//

class MpxProxyTaskBase: public MpxTaskBase
{
public:
	MpxProxyTaskBase () :
		MpxTaskBase ()
	{
	}
	~MpxProxyTaskBase ()
	{
	}
	virtual int SendProxy (MpxTaskBase* task, MpxEventBase* event, bool invoke = false) = 0;
};

template <typename T> class MpxProxyTask: public MpxProxyTaskBase
{
public:
	MpxProxyTask (MpxTaskBase* task, MpxSocket <T> * socket) :
		MpxProxyTaskBase (), m_task (task), m_socket (socket)
	{
		if (false)
			cout << "create proxy " << this << endl;
		static EventDescriptor g_evntab[] =
		{
			{ AnyState, StartEvent, HandleStartEvent, 0 },
			{ AnyState, StopEvent, HandleStopEvent, 0 },
			{ 0, 0, 0, 0 }
		};
		RegisterEventHandlers (g_evntab);
	}
	virtual ~MpxProxyTask ()
	{
		delete m_socket;
	}
	virtual int SendProxy (MpxTaskBase* task, MpxEventBase* event, bool invoke = false)
	{
		if (false)
			cout << "proxy event: " << event->code () << endl;
		xdrproc_t proc;
		void* data;
		if (event->Encode (proc, data) < 0)
			return -1;
		int n = m_socket->PostXdrRequest (proc, data);
		xdr_free (proc, (char*) data);
		delete event;
		return n;
	}
	inline MpxTaskBase* task ()
	{
		return m_task;
	}
	inline void* socket ()
	{
		return m_socket;
	}
private:

	inline static void HandleStartEvent (MpxEventBase *event, mpx_appdt_t appdata)
	{
		((MpxProxyTask*) appdata)->HandleStartEvent (event);
	}
	void HandleStartEvent (MpxEventBase *event)
	{
		if (false)
			cout << "proxy event: start" << endl;
	}

	inline static void HandleStopEvent (MpxEventBase *event, mpx_appdt_t appdata)
	{
		((MpxProxyTask*) appdata)->HandleStopEvent (event);
	}
	void HandleStopEvent (MpxEventBase *event)
	{
		if (false)
			cout << "proxy event: stop" << endl;
	}

protected:
	MpxTaskBase* m_task;
	MpxSocket <T> * m_socket;
};

} // namespace mpx
