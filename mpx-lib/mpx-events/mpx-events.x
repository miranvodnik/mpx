/*
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
*/

enum MpxEventCode
{
	MpxEventBaseCode = 0,
	MpxStartEventCode = -1,
	MpxTimerEventCode = -2,
	MpxStopEventCode = -3,
	MpxTcp4ListenerEventCode = -4,
	MpxTcp4EndPointEventCode = -5,
	MpxTcp4ClientEventCode = -6,
	MpxTcp6ListenerEventCode = -7,
	MpxTcp6EndPointEventCode = -8,
	MpxTcp6ClientEventCode = -9,
	MpxLocalListenerEventCode = -10,
	MpxLocalEndPointEventCode = -11,
	MpxLocalClientEventCode = -12,
	MpxPosixMQEventCode = -13,
	MpxJobFinishedEventCode = -14,
	MpxLocalTaskQueryEventCode = -15,
	MpxPosixMQTaskQueryEventCode = -16,
	MpxTcp4TaskQueryEventCode = -17,
	MpxTcp6TaskQueryEventCode = -18,
	MpxUdp4TaskQueryEventCode = -19,
	MpxUdp6TaskQueryEventCode = -20,
	MpxExternalTaskEventCode = -21,
	MpxTaskQueryEventCode = -22,
	MpxTaskResponseEventCode = -23,

	MpxConsumerEventCode = 1
};

typedef u_long taskid_t;

struct	MpxEventBaseStruct
{
	u_int m_code;
	taskid_t m_src;
	taskid_t m_dst;
};

struct	MpxExternalTaskEventStruct
{
	MpxEventBaseStruct base;
	u_int m_flags;
	u_int m_error;
	u_int m_size;
	char m_buffer<>;
};

struct	MpxJobFinishedEventStruct
{
	MpxEventBaseStruct base;
	u_long m_job;
};

struct	MpxLocalClientEventStruct
{
	MpxEventBaseStruct base;
	u_long m_endPoint;
	u_int m_flags;
	u_int m_error;
	u_int m_size;
	char m_buffer<>;
};

struct	MpxLocalEndPointEventStruct
{
	MpxEventBaseStruct base;
	u_long m_endPoint;
	u_int m_flags;
	u_int m_error;
	u_int m_size;
	char m_buffer<>;
};

struct	MpxLocalListenerEventStruct
{
	MpxEventBaseStruct base;
	int m_fd;
};

struct	MpxLocalTaskQueryEventStruct
{
	MpxEventBaseStruct base;
	string m_path<>;
	string m_name<>;
};

struct	MpxPosixMQEventStruct
{
	MpxEventBaseStruct base;
	u_long m_listener;
	u_int m_flags;
	u_int m_error;
	u_int m_size;
	char m_buffer<>;
};

struct	MpxPosixMQTaskQueryEventStruct
{
	MpxEventBaseStruct base;
	string m_path<>;
	string m_name<>;
};

struct	MpxStartEventStruct
{
	MpxEventBaseStruct base;
};

struct	MpxStopEventStruct
{
	MpxEventBaseStruct base;
};

enum MpxTaskQueryEventType
{
	TaskQueryEventNone,
	TaskQueryEventLocal,
	TaskQueryEventPosixMQ,
	TaskQueryEventTcp4,
	TaskQueryEventUdp4,
	TaskQueryEventTcp6,
	TaskQueryEventUdp6
};

struct	MpxTaskQueryEventStruct
{
	MpxEventBaseStruct base;
	string m_taskName<>;
	MpxTaskQueryEventType m_queryType;
	u_long m_endPoint;
};

struct	MpxTaskResponseEventStruct
{
	MpxEventBaseStruct base;
	u_long m_task;
	MpxTaskQueryEventType m_queryType;
	u_long m_endPoint;
};

struct	MpxTcp4ClientEventStruct
{
	MpxEventBaseStruct base;
	u_long m_endPoint;
	u_int m_flags;
	u_int m_error;
	u_int m_size;
	char m_buffer<>;
};

struct	MpxTcp4EndPointEventStruct
{
	MpxEventBaseStruct base;
	u_long m_endPoint;
	u_int m_flags;
	u_int m_error;
	u_int m_size;
	char m_buffer<>;
};

struct	MpxTcp4ListenerEventStruct
{
	MpxEventBaseStruct base;
	int m_fd;
};

struct	MpxTcp4TaskQueryEventStruct
{
	MpxEventBaseStruct base;
	string m_hostname<>;
	string m_port<>;
	string m_name<>;
};

struct	MpxTcp6ClientEventStruct
{
	MpxEventBaseStruct base;
	u_long m_endPoint;
	u_int m_flags;
	u_int m_error;
	u_int m_size;
	char m_buffer<>;
};

struct	MpxTcp6EndPointEventStruct
{
	MpxEventBaseStruct base;
	u_long m_endPoint;
	u_int m_flags;
	u_int m_error;
	u_int m_size;
	char m_buffer<>;
};

struct	MpxTcp6ListenerEventStruct
{
	MpxEventBaseStruct base;
	int m_fd;
};

struct	MpxTcp6TaskQueryEventStruct
{
	MpxEventBaseStruct base;
	string m_hostname<>;
	string m_port<>;
	string m_name<>;
};

struct	MpxTimerEventStruct
{
	MpxEventBaseStruct base;
	u_long m_task;
	u_long m_timerStamp;
	u_long m_timer;
};

struct	MpxUdp4TaskQueryEventStruct
{
	MpxEventBaseStruct base;
	string m_hostname<>;
	string m_port<>;
	string m_name<>;
};

struct	MpxUdp6TaskQueryEventStruct
{
	MpxEventBaseStruct base;
	string m_hostname<>;
	string m_port<>;
	string m_name<>;
};

struct	MpxConsumerEventStruct
{
	MpxEventBaseStruct base;
	string m_string<>;
};

union MpxEventStruct
switch (MpxEventCode m_code)
{
	case MpxEventBaseCode:
		MpxEventBaseStruct m_EventBasw;
	case MpxExternalTaskEventCode:
		MpxExternalTaskEventStruct m_ExternalTaskEvent;
	case MpxJobFinishedEventCode:
		MpxJobFinishedEventStruct m_JobFinishedEvent;
	case MpxLocalClientEventCode:
		MpxLocalClientEventStruct m_LocalClientEvent;
	case MpxLocalEndPointEventCode:
		MpxLocalEndPointEventStruct m_LocalEndPointEvent;
	case MpxLocalListenerEventCode:
		MpxLocalListenerEventStruct m_LocalListenerEvent;
	case MpxLocalTaskQueryEventCode:
		MpxLocalTaskQueryEventStruct m_LocalTaskQueryEvent;
	case MpxPosixMQEventCode:
		MpxPosixMQEventStruct m_PosixMQEvent;
	case MpxPosixMQTaskQueryEventCode:
		MpxPosixMQTaskQueryEventStruct m_PosixMQTaskQueryEvent;
	case MpxStartEventCode:
		MpxStartEventStruct m_StartEvent;
	case MpxStopEventCode:
		MpxStopEventStruct m_StopEvent;
	case MpxTaskQueryEventCode:
		MpxTaskQueryEventStruct m_TaskQueryEvent;
	case MpxTaskResponseEventCode:
		MpxTaskResponseEventStruct m_TaskResponseEvent;
	case MpxTcp4ClientEventCode:
		MpxTcp4ClientEventStruct m_Tcp4ClientEvent;
	case MpxTcp4EndPointEventCode:
		MpxTcp4EndPointEventStruct m_Tcp4EndPointEvent;
	case MpxTcp4ListenerEventCode:
		MpxTcp4ListenerEventStruct m_Tcp4ListenerEvent;
	case MpxTcp4TaskQueryEventCode:
		MpxTcp4TaskQueryEventStruct m_Tcp4TaskQueryEvent;
	case MpxTcp6ClientEventCode:
		MpxTcp6ClientEventStruct m_Tcp6ClientEvent;
	case MpxTcp6EndPointEventCode:
		MpxTcp6EndPointEventStruct m_Tcp6EndPointEvent;
	case MpxTcp6ListenerEventCode:
		MpxTcp6ListenerEventStruct m_Tcp6ListenerEvent;
	case MpxTcp6TaskQueryEventCode:
		MpxTcp6TaskQueryEventStruct m_Tcp6TaskQueryEvent;
	case MpxTimerEventCode:
		MpxTimerEventStruct m_TimerEvent;
	case MpxUdp4TaskQueryEventCode:
		MpxUdp4TaskQueryEventStruct m_Udp4TaskQueryEvent;
	case MpxUdp6TaskQueryEventCode:
		MpxUdp6TaskQueryEventStruct m_Udp6TaskQueryEvent;
	case MpxConsumerEventCode:
		MpxConsumerEventStruct m_ConsumerEvent;
};
