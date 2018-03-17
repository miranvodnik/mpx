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

template <typename T> class MpxProxyTask: public MpxProxyTaskBase
{
public:
	typedef T MpxSocketEvent;
	typedef MpxEventXDRItf* (*edfunc) ();
	MpxProxyTask (evnset& e, MpxTaskBase* task, MpxSocket <MpxSocketEvent> * socket) :
		MpxProxyTaskBase (e), m_task (task), m_socket (socket), m_lib (0), m_fcn (0), m_eventXDR (0)
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
	MpxTaskBase* m_task;
	MpxSocket <MpxSocketEvent> * m_socket;
	void* m_lib;
	edfunc m_fcn;
	MpxEventXDRItf* m_eventXDR;
};

} // namespace mpx
