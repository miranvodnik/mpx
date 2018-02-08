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
#include <mpx-events/mpx-events.h>

namespace mpx
{

enum MpxEventCode
{
	InvalidEvent,
	StartEvent = (u_int) -1,
	TimerEvent = (u_int) -2,
	StopEvent = (u_int) -3,
	Tcp4ListenerEvent = (u_int) -4,
	Tcp4EndPointEvent = (u_int) -5,
	Tcp4ClientEvent = (u_int) -6,
	Tcp6ListenerEvent = (u_int) -7,
	Tcp6EndPointEvent = (u_int) -8,
	Tcp6ClientEvent = (u_int) -9,
	LocalListenerEvent = (u_int) -10,
	LocalEndPointEvent = (u_int) -11,
	LocalClientEvent = (u_int) -12,
	PosixMQEvent = (u_int) -13,
	JobFinishedEvent = (u_int) -14,
	LocalTaskQueryEvent = (u_int) -15,
	PosixMQTaskQueryEvent = (u_int) -16,
	Tcp4TaskQueryEvent = (u_int) -17,
	Tcp6TaskQueryEvent = (u_int) -18,
	Udp4TaskQueryEvent = (u_int) -19,
	Udp6TaskQueryEvent = (u_int) -20,
	ExternalTaskEvent = (u_int) -21,
	TaskQueryEvent = (u_int) -22,
	TaskResponseEvent = (u_int) -23,
};

enum MpxTaskQueryEventType
{
	TaskQueryEventNone,
	TaskQueryEventLocal,
	TaskQueryEventPosixMQ,
	TaskQueryEventTcp4,
	TaskQueryEventUdp4,
	TaskQueryEventTcp6,
	TaskQueryEventUdp6,
};

class MpxEventBase
{
public:
	typedef void* taskid_t;
	typedef void* xdrdata_t;

public:
	MpxEventBase (u_int code) :
		m_code (code), m_src (0), m_dst (0)
	{
	}
	virtual ~MpxEventBase ()
	{
	}
	int Invoke ();
	virtual const char* Name () = 0;
	virtual MpxEventBase* Copy () = 0;
	virtual int Encode (xdrproc_t& proc, xdrdata_t& data) = 0;
	virtual int Decode (MpxEventStruct* eventStruct) = 0;

	inline u_int code ()
	{
		return m_code;
	}
	inline taskid_t src ()
	{
		return m_src;
	}
	inline void src (taskid_t src)
	{
		m_src = src;
	}
	inline taskid_t dst ()
	{
		return m_dst;
	}
	inline void dst (taskid_t dst)
	{
		m_dst = dst;
	}
private:
	u_int m_code;
	taskid_t m_src;
	taskid_t m_dst;
};

} /* namespace mpx */
