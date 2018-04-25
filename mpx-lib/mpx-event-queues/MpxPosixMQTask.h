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
#include <mpx-events/MpxStartEvent.h>
#include <mpx-events/MpxStopEvent.h>
#include "mpx-events/MpxPosixMQEvent.h"
#include <mpx-sockets/MpxPosixMQ.h>
using namespace mpx;

#include <map>
using namespace std;

namespace mpx
{

class MpxPosixMQTask: public mpx::MpxMQTaskI
{
public:
	typedef map <void*, MpxPosixMQ*> mqset;
	MpxPosixMQTask ();
	virtual ~MpxPosixMQTask ();
	virtual void StartTask ();
	virtual void StopTask ();
private:
	void Release ();
	MpxPosixMQ* Connect (tskmpx_t mpx);
	virtual MpxMQTaskI* Copy (tskmpx_t mpx);
	virtual int MQSend (tskmpx_t mpx, MpxEventBase* event);
	virtual void UnlinkMQ (tskmpx_t mpx);
private:
	;mpx_event_handler (PosixMQEventHandler, MpxPosixMQTask)
	;
private:
	static EventDescriptor g_evntab [];
	static evnset g_evnset;
	MpxPosixMQ* m_listener;
	mqset m_mqset;
};

} /* namespace mpx */
