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
#include <mpx-sockets/MpxLocalClient.h>
#include <mpx-sockets/MpxLocalEndPoint.h>
#include <mpx-sockets/MpxLocalListener.h>
#include <mpx-sockets/MpxTcp4Client.h>
#include <mpx-sockets/MpxTcp4EndPoint.h>
#include <mpx-sockets/MpxTcp4Listener.h>
#include <mpx-sockets/MpxTcp6Client.h>
#include <mpx-sockets/MpxTcp6EndPoint.h>
#include <mpx-sockets/MpxTcp6Listener.h>
#include <mpx-events/MpxLocalClientEvent.h>
#include <mpx-events/MpxLocalEndPointEvent.h>
#include <mpx-events/MpxLocalListenerEvent.h>
#include <mpx-events/MpxTcp4ClientEvent.h>
#include <mpx-events/MpxTcp4EndPointEvent.h>
#include <mpx-events/MpxTcp4ListenerEvent.h>
#include <mpx-events/MpxTcp6ClientEvent.h>
#include <mpx-events/MpxTcp6EndPointEvent.h>
#include <mpx-events/MpxTcp6ListenerEvent.h>

namespace mpx
{

enum MpxExternalTaskType
{
	InvalidTaskType,
	LocalListenerTaskType,
	Tcp4ListenerTaskType,
	Tcp6ListenerTaskType,
};

class MpxExternalTask: public mpx::MpxTaskBase
{
	typedef set <MpxLocalListener*> llset;
	typedef set <MpxLocalEndPoint*> leset;
	typedef set <MpxLocalClient*> lcset;
	typedef set <MpxTcp4Listener*> t4lset;
	typedef set <MpxTcp4EndPoint*> t4eset;
	typedef set <MpxTcp4Client*> t4cset;
	typedef set <MpxTcp6Listener*> t6lset;
	typedef set <MpxTcp6EndPoint*> t6eset;
	typedef set <MpxTcp6Client*> t6cset;

	typedef map <MpxLocalClient*, MpxTaskBase*> lctmap;
	typedef map <MpxTcp4Client*, MpxTaskBase*> t4tmap;
	typedef map <MpxTcp6Client*, MpxTaskBase*> t6tmap;

public:
	MpxExternalTask (const char* connStr = 0);
	virtual ~MpxExternalTask ();
	virtual void StartTask ();
	virtual void StopTask ();
private:
	void CreateListeners (const char* connStr);
	const char* CreateLocalListener (const char* connStr);
	const char* CreateTcp4Listener (const char* connStr);
	const char* CreateTcp6Listener (const char* connStr);
	void Release ();

	mpx_event_handler(HandleLocalTaskQueryEvent, MpxExternalTask)
	mpx_event_handler(HandleTcp4TaskQueryEvent, MpxExternalTask)
	mpx_event_handler(HandleTcp6TaskQueryEvent, MpxExternalTask)

	mpx_event_handler(HandleLocalListenerEvent, MpxExternalTask)
	mpx_event_handler(HandleLocalClientEvent, MpxExternalTask)
	mpx_event_handler(HandleLocalEndPointEvent, MpxExternalTask)

	mpx_event_handler(HandleTcp4ListenerEvent, MpxExternalTask)
	mpx_event_handler(HandleTcp4ClientEvent, MpxExternalTask)
	mpx_event_handler(HandleTcp4EndPointEvent, MpxExternalTask)

	mpx_event_handler(HandleTcp6ListenerEvent, MpxExternalTask)
	mpx_event_handler(HandleTcp6ClientEvent, MpxExternalTask)
	mpx_event_handler(HandleTcp6EndPointEvent, MpxExternalTask)

	mpx_event_handler(HandleTaskQueryEvent, MpxExternalTask)
	mpx_event_handler(HandleTaskResponseEvent, MpxExternalTask)

	mpx_event_handler(HandleJobFinishedEvent, MpxExternalTask)

private:
	static const char* g_protocolField;
	static const char* g_protoLocal;
	static const char* g_protoTcp4;
	static const char* g_protoTcp6;
	static const char* g_pathField;
	static const char* g_addressField;
	static const char* g_portField;
	static EventDescriptor g_evntab[];
	static evnset g_evnset;

	string m_connStr;
	bool m_started;

	llset m_llset;
	leset m_leset;
	lcset m_lcset;
	t4lset m_t4lset;
	t4eset m_t4eset;
	t4cset m_t4cset;
	t6lset m_t6lset;
	t6eset m_t6eset;
	t6cset m_t6cset;

	lctmap m_lctmap;
	t4tmap m_t4tmap;
	t6tmap m_t6tmap;
};

} /* namespace mpx */
