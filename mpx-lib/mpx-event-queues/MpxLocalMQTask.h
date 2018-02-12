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

#include <mpx-core/MpxUtilities.h>
#include <mpx-event-queues/MpxMQTaskI.h>
#include <mpx-events/MpxLocalClientEvent.h>
#include <mpx-events/MpxLocalEndPointEvent.h>
#include <mpx-events/MpxLocalListenerEvent.h>
#include <mpx-events/MpxStartEvent.h>
#include <mpx-events/MpxStopEvent.h>
#include <mpx-sockets/MpxLocalClient.h>
#include <mpx-sockets/MpxLocalEndPoint.h>
#include <mpx-sockets/MpxLocalListener.h>
using namespace mpx;

#include <map>
using namespace std;

namespace mpx
{

class MpxLocalMQTask: public mpx::MpxMQTaskI
{
public:
	typedef set <MpxLocalEndPoint*> epset;
	typedef map <void*, MpxLocalClient*> clnset;
	MpxLocalMQTask ();
	virtual ~MpxLocalMQTask ();
	virtual MpxMQTaskI* Copy (tskmpx_t mpx);
	virtual int MQSend (tskmpx_t mpx, MpxEventBase* event);
	virtual void UnlinkMQ (tskmpx_t mpx);
private:
	virtual MpxLocalClient* Connect (tskmpx_t mpx);
	void Release ();
private:
	mpx_event_handler (StartEventHandler, MpxLocalMQTask)
	;mpx_event_handler (StopEventHandler, MpxLocalMQTask)
	;mpx_event_handler (LocalListenerEventHandler, MpxLocalMQTask)
	;mpx_event_handler (LocalEndPointEventHandler, MpxLocalMQTask)
	;mpx_event_handler (LocalClientEventHandler, MpxLocalMQTask)
	;
private:
	static EventDescriptor g_evntab[];
	static const char* g_localPath;
	MpxLocalListener* m_listener;
	epset m_epset;
	clnset m_clnset;
};

} /* namespace mpx */
