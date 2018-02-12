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

#include <sys/types.h>
#include <ifaddrs.h>

#include <mpx-core/MpxEnvironment.h>
#include <mpx-tasks/MpxExternalTask.h>
#include <mpx-tasks/MpxLocalClientProxyTask.h>
#include <mpx-tasks/MpxLocalEndPointProxyTask.h>
#include <mpx-tasks/MpxTcp4ClientProxyTask.h>
#include <mpx-tasks/MpxTcp4EndPointProxyTask.h>
#include <mpx-tasks/MpxTcp6ClientProxyTask.h>
#include <mpx-tasks/MpxTcp6EndPointProxyTask.h>
#include <mpx-sockets/MpxLocalClient.h>
#include <mpx-sockets/MpxTcp4Client.h>
#include <mpx-sockets/MpxTcp6Client.h>
#include <mpx-events/MpxEvents.h>

namespace mpx
{

const char* MpxExternalTask::g_protocolField = "protocol:";
const char* MpxExternalTask::g_protoLocal = "local,";
const char* MpxExternalTask::g_protoTcp4 = "tcp4,";
const char* MpxExternalTask::g_protoTcp6 = "tcp6,";
const char* MpxExternalTask::g_pathField = "path:";
const char* MpxExternalTask::g_addressField = "address:";
const char* MpxExternalTask::g_portField = "port:";

EventDescriptor MpxExternalTask::g_evntab[] =
{
	{ AnyState, StartEvent, HandleStartEvent, 0 },
	{ AnyState, StopEvent, HandleStopEvent, 0 },
	{ AnyState, LocalTaskQueryEvent, HandleLocalTaskQueryEvent, 0 },
	{ AnyState, Tcp4TaskQueryEvent, HandleTcp4TaskQueryEvent, 0 },
	{ AnyState, Tcp6TaskQueryEvent, HandleTcp6TaskQueryEvent, 0 },
	{ AnyState, LocalListenerEvent, HandleLocalListenerEvent, 0 },
	{ AnyState, LocalClientEvent, HandleLocalClientEvent, 0 },
	{ AnyState, LocalEndPointEvent, HandleLocalEndPointEvent, 0 },
	{ AnyState, Tcp4ListenerEvent, HandleTcp4ListenerEvent, 0 },
	{ AnyState, Tcp4ClientEvent, HandleTcp4ClientEvent, 0 },
	{ AnyState, Tcp4EndPointEvent, HandleTcp4EndPointEvent, 0 },
	{ AnyState, Tcp6ListenerEvent, HandleTcp6ListenerEvent, 0 },
	{ AnyState, Tcp6ClientEvent, HandleTcp6ClientEvent, 0 },
	{ AnyState, Tcp6EndPointEvent, HandleTcp6EndPointEvent, 0 },
	{ AnyState, TaskQueryEvent, HandleTaskQueryEvent, 0 },
	{ AnyState, TaskResponseEvent, HandleTaskResponseEvent, 0 },
	{ 0, 0, 0, 0 }
};

MpxExternalTask::MpxExternalTask (const char* connStr) :
	MpxTaskBase ()
{
	m_connStr = (connStr != 0) ? connStr : "";
	m_started = false;

	RegisterEventHandlers (g_evntab);
}

MpxExternalTask::~MpxExternalTask ()
{
	Release ();
}

void MpxExternalTask::Release ()
{
	for (llset::iterator it = m_llset.begin (); it != m_llset.end (); ++it)
		delete *it;
	m_llset.clear ();
	for (leset::iterator it = m_leset.begin (); it != m_leset.end (); ++it)
		delete *it;
	m_leset.clear ();
	for (lcset::iterator it = m_lcset.begin (); it != m_lcset.end (); ++it)
		delete *it;
	m_lcset.clear ();
	for (t4lset::iterator it = m_t4lset.begin (); it != m_t4lset.end (); ++it)
		delete *it;
	m_t4lset.clear ();
	for (t4eset::iterator it = m_t4eset.begin (); it != m_t4eset.end (); ++it)
		delete *it;
	m_t4eset.clear ();
	for (t4cset::iterator it = m_t4cset.begin (); it != m_t4cset.end (); ++it)
		delete *it;
	m_t4cset.clear ();
	for (t6lset::iterator it = m_t6lset.begin (); it != m_t6lset.end (); ++it)
		delete *it;
	m_t6lset.clear ();
	for (t6eset::iterator it = m_t6eset.begin (); it != m_t6eset.end (); ++it)
		delete *it;
	m_t6eset.clear ();
	for (t6cset::iterator it = m_t6cset.begin (); it != m_t6cset.end (); ++it)
		delete *it;
	m_t6cset.clear ();

	m_lctmap.clear ();
	m_t4tmap.clear ();
	m_t6tmap.clear ();
}

void MpxExternalTask::CreateListeners (const char* connStr)
{
	while (connStr != 0)
	{
		if (strncasecmp (connStr, g_protocolField, strlen (g_protocolField)) != 0)
			break;
		connStr += strlen (g_protocolField);
		if (strncasecmp (connStr, g_protoLocal, strlen (g_protoLocal)) == 0)
		{
			connStr = CreateLocalListener (connStr + strlen (g_protoLocal));
		}
		else if (strncasecmp (connStr, g_protoTcp4, strlen (g_protoTcp4)) == 0)
		{
			connStr = CreateTcp4Listener (connStr + strlen (g_protoTcp4));
		}
		else if (strncasecmp (connStr, g_protoTcp6, strlen (g_protoTcp6)) == 0)
		{
			connStr = CreateTcp6Listener (connStr + strlen (g_protoTcp6));
		}
		else
			break;
	}
}

const char* MpxExternalTask::CreateLocalListener (const char* connStr)
{
	size_t size;

	if (strncasecmp (connStr, g_pathField, strlen (g_pathField)) != 0)
		return 0;

	connStr += strlen (g_pathField);
	const char* path = connStr;
	if ((connStr = strchr (connStr, ';')) == 0)
		return 0;

	char* mpath = (char*) alloca((size = connStr - path) + 1);
	strncpy (mpath, path, size);
	mpath [size] = 0;

	++connStr;
	MpxLocalListener* localListener = new MpxLocalListener (this, true);
	if (localListener == 0)
		return connStr;

	if ((localListener->CreateListener (mpath) != 0) || (localListener->StartListener () != 0))
	{
		delete localListener;
		return connStr;
	}

	m_llset.insert (localListener);
	return connStr;
}

const char* MpxExternalTask::CreateTcp4Listener (const char* connStr)
{
	size_t size;

	if (strncasecmp (connStr, g_addressField, strlen (g_addressField)) != 0)
		return 0;

	connStr += strlen (g_addressField);
	const char* address = connStr;
	if ((connStr = strchr (connStr, ',')) == 0)
		return 0;
	char* maddress = (char*) alloca((size = connStr - address) + 1);
	strncpy (maddress, address, size);
	maddress [size] = 0;
	++connStr;

	if (strncasecmp (connStr, g_portField, strlen (g_portField)) != 0)
		return 0;

	connStr += strlen (g_portField);
	const char* port = connStr;
	if ((connStr = strchr (connStr, ';')) == 0)
		return 0;
	char* mport = (char*) alloca((size = connStr - port) + 1);
	strncpy (mport, port, size);
	mport [size] = 0;
	++connStr;

	if (strcasecmp (maddress, "any") == 0)
	{
		MpxTcp4Listener* tcp4Listener = new MpxTcp4Listener (this, false);
		if (tcp4Listener == 0)
			return connStr;

		if ((tcp4Listener->CreateListener ((in_port_t) atoi (mport)) != 0) || (tcp4Listener->StartListener () != 0))
		{
			delete tcp4Listener;
			return connStr;
		}
		m_t4lset.insert (tcp4Listener);
	}
	else if (strcasecmp (maddress, "all") == 0)
	{
		struct ifaddrs* ifaddrs = 0;
		if (getifaddrs (&ifaddrs) < 0)
			return connStr;

		for (struct ifaddrs* ifptr = ifaddrs; ifptr != 0; ifptr = ifptr->ifa_next)
		{
			if (ifptr->ifa_addr == 0)
				continue;

			if (strcmp (ifptr->ifa_name, "lo") == 0)
				continue;

			if (ifptr->ifa_addr->sa_family != PF_INET)
				continue;

			MpxTcp4Listener* tcp4Listener = new MpxTcp4Listener (this, false);
			if (tcp4Listener == 0)
				continue;

			if ((tcp4Listener->CreateListener (ifptr->ifa_addr, (in_port_t) atoi (mport)) != 0) || (tcp4Listener->StartListener () != 0))
			{
				delete tcp4Listener;
				continue;
			}
			m_t4lset.insert (tcp4Listener);
		}
		freeifaddrs (ifaddrs);
	}
	else
	{
		MpxTcp4Listener* tcp4Listener = new MpxTcp4Listener (this, false);
		if (tcp4Listener == 0)
			return connStr;

		if ((tcp4Listener->CreateListener (maddress, (in_port_t) atoi (mport)) != 0) || (tcp4Listener->StartListener () != 0))
		{
			delete tcp4Listener;
			return connStr;
		}
		m_t4lset.insert (tcp4Listener);
	}

	return connStr;
}

const char* MpxExternalTask::CreateTcp6Listener (const char* connStr)
{
	size_t size;

	if (strncasecmp (connStr, g_addressField, strlen (g_addressField)) != 0)
		return 0;

	connStr += strlen (g_addressField);
	const char* address = connStr;
	if ((connStr = strchr (connStr, ',')) == 0)
		return 0;
	char* maddress = (char*) alloca((size = connStr - address) + 1);
	strncpy (maddress, address, size);
	maddress [size] = 0;
	++connStr;

	if (strncasecmp (connStr, g_portField, strlen (g_portField)) != 0)
		return 0;

	connStr += strlen (g_portField);
	const char* port = connStr;
	if ((connStr = strchr (connStr, ';')) == 0)
		return 0;
	char* mport = (char*) alloca((size = connStr - port) + 1);
	strncpy (mport, port, size);
	mport [size] = 0;
	++connStr;

	if (strcasecmp (maddress, "any") == 0)
	{
		MpxTcp6Listener* tcp6Listener = new MpxTcp6Listener (this, false);
		if (tcp6Listener == 0)
			return connStr;

		if ((tcp6Listener->CreateListener ((in_port_t) atoi (mport)) != 0) || (tcp6Listener->StartListener () != 0))
		{
			delete tcp6Listener;
			return connStr;
		}
		m_t6lset.insert (tcp6Listener);
	}
	else if (strcasecmp (maddress, "all") == 0)
	{
		struct ifaddrs* ifaddrs = 0;
		if (getifaddrs (&ifaddrs) < 0)
			return connStr;

		for (struct ifaddrs* ifptr = ifaddrs; ifptr != 0; ifptr = ifptr->ifa_next)
		{
			if (ifptr->ifa_addr == 0)
				continue;

			if (strcmp (ifptr->ifa_name, "lo") == 0)
				continue;

			if (ifptr->ifa_addr->sa_family != PF_INET6)
				continue;

			MpxTcp6Listener* tcp6Listener = new MpxTcp6Listener (this, false);
			if (tcp6Listener == 0)
				continue;

			if ((tcp6Listener->CreateListener (ifptr->ifa_addr, (in_port_t) atoi (mport)) != 0) || (tcp6Listener->StartListener () != 0))
			{
				delete tcp6Listener;
				continue;
			}
			m_t6lset.insert (tcp6Listener);
		}
		freeifaddrs (ifaddrs);
	}
	else
	{
		MpxTcp6Listener* tcp6Listener = new MpxTcp6Listener (this, false);
		if (tcp6Listener == 0)
			return connStr;

		if ((tcp6Listener->CreateListener (maddress, (in_port_t) atoi (mport)) != 0) || (tcp6Listener->StartListener () != 0))
		{
			delete tcp6Listener;
			return connStr;
		}
		m_t6lset.insert (tcp6Listener);
	}

	return connStr;
}

void MpxExternalTask::HandleStartEvent (MpxEventBase* event)
{
	if (m_started)
		return;
	CreateListeners (m_connStr.c_str ());
	m_started = true;
}

void MpxExternalTask::HandleStopEvent (MpxEventBase* event)
{
	Release ();
}

void MpxExternalTask::HandleLocalTaskQueryEvent (MpxEventBase* event)
{
	MpxLocalTaskQueryEvent* localTaskQueryEvent = dynamic_cast <MpxLocalTaskQueryEvent*> (event);
	if (localTaskQueryEvent == 0)
		return;

	MpxTaskBase* srcTask = dynamic_cast <MpxTaskBase*> ((MpxTaskBase*) localTaskQueryEvent->src ());
	if (srcTask == 0)
		return;

	MpxLocalClient* localClient = new MpxLocalClient (this, true, 10L * SEC_TO_NSEC, true);
	if (localClient->Connect (localTaskQueryEvent->path ()) != 0)
	{
		Send (srcTask, new MpxExternalTaskEvent (0, -1, 0, 0), true);
		delete localClient;
		return;
	}

	const char* name = localTaskQueryEvent->name ();
	MpxMessage msg;
	msg.m_Code = ExternalTaskRequestCode;
	msg.MpxMessage_u.m_externalTaskRequest.taskName = (char*) name;
	localClient->PostXdrRequest ((xdrproc_t) xdr_MpxMessage, (void*) &msg);
	m_lctmap [localClient] = srcTask;

	if (true)
		cout << "EXTERNAL TASK: WRITE: " << name << endl;
}

void MpxExternalTask::HandleTcp4TaskQueryEvent (MpxEventBase* event)
{
	MpxTcp4TaskQueryEvent* tcp4TaskQueryEvent = dynamic_cast <MpxTcp4TaskQueryEvent*> (event);
	if (tcp4TaskQueryEvent == 0)
		return;

	MpxTaskBase* srcTask = dynamic_cast <MpxTaskBase*> ((MpxTaskBase*) tcp4TaskQueryEvent->src ());
	if (srcTask == 0)
		return;

	MpxTcp4Client* tcp4Client = new MpxTcp4Client (this, true, 10L * SEC_TO_NSEC, true);
	if (tcp4Client->Connect (tcp4TaskQueryEvent->hostname (), atoi (tcp4TaskQueryEvent->port ())) != 0)
	{
		Send (srcTask, new MpxExternalTaskEvent (0, -1, 0, 0), true);
		delete tcp4Client;
		return;
	}

	const char* name = tcp4TaskQueryEvent->name ();
	MpxMessage msg;
	msg.m_Code = ExternalTaskRequestCode;
	msg.MpxMessage_u.m_externalTaskRequest.taskName = (char*) name;
	tcp4Client->PostXdrRequest ((xdrproc_t) xdr_MpxMessage, (void*) &msg);
	m_t4tmap [tcp4Client] = srcTask;

	if (true)
		cout << "EXTERNAL TASK: WRITE: " << name << endl;
}

void MpxExternalTask::HandleTcp6TaskQueryEvent (MpxEventBase* event)
{
	MpxTcp6TaskQueryEvent* tcp6TaskQueryEvent = dynamic_cast <MpxTcp6TaskQueryEvent*> (event);
	if (tcp6TaskQueryEvent == 0)
		return;

	MpxTaskBase* srcTask = dynamic_cast <MpxTaskBase*> ((MpxTaskBase*) tcp6TaskQueryEvent->src ());
	if (srcTask == 0)
		return;

	MpxTcp6Client* tcp6Client = new MpxTcp6Client (this, true, 10L * SEC_TO_NSEC, true);
	if (tcp6Client->Connect (tcp6TaskQueryEvent->hostname (), atoi (tcp6TaskQueryEvent->port ())) != 0)
	{
		Send (srcTask, new MpxExternalTaskEvent (0, -1, 0, 0), true);
		delete tcp6Client;
		return;
	}

	const char* name = tcp6TaskQueryEvent->name ();
	MpxMessage msg;
	msg.m_Code = ExternalTaskRequestCode;
	msg.MpxMessage_u.m_externalTaskRequest.taskName = (char*) name;
	tcp6Client->PostXdrRequest ((xdrproc_t) xdr_MpxMessage, (void*) &msg);
	m_t6tmap [tcp6Client] = srcTask;

	if (true)
		cout << "EXTERNAL TASK: WRITE: " << name << endl;
}

void MpxExternalTask::HandleLocalListenerEvent (MpxEventBase* event)
{
	MpxLocalListenerEvent* localListenerEvent = dynamic_cast <MpxLocalListenerEvent*> (event);
	if (localListenerEvent == 0)
		return;
	m_leset.insert (new MpxLocalEndPoint (this, localListenerEvent->fd (), true, -1L, false));
}

void MpxExternalTask::HandleLocalClientEvent (MpxEventBase* event)
{
	MpxLocalClientEvent* localClientEvent = dynamic_cast <MpxLocalClientEvent*> (event);
	if (localClientEvent == 0)
		return;
	MpxLocalClient* localClient = dynamic_cast <MpxLocalClient*> ((MpxLocalClient*) localClientEvent->endPoint ());
	if (localClient == 0)
		return;

	lctmap::iterator it = m_lctmap.find (localClient);
	if (it == m_lctmap.end ())
		return;

	MpxTaskBase* task = it->second;

	switch (localClientEvent->flags ())
	{
	case 0:
		break;
	case EPOLLIN:
		if (false)
			cout << "EXTERNAL TASK: LOCAL CLIENT EVENT IN" << endl;

		if ((localClientEvent->error () == 0) && (localClientEvent->size () != 0))
		{
			while (true)
			{
				MpxMessage* rpl = (MpxMessage*) malloc (sizeof(MpxMessage));
				memset (rpl, 0, sizeof(MpxMessage));
				if (localClient->ReadXdrRequest ((xdrproc_t) xdr_MpxMessage, rpl) < 0)
				{
					free (rpl);
					break;
				}
				switch (rpl->m_Code)
				{
				case ExternalTaskReplyCode:
				{
					MpxExternalTaskReply* externalTaskReply = &(rpl->MpxMessage_u.m_externalTaskReply);
					if ((MpxTaskBase*) externalTaskReply->task == 0)
						break;

					MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
					m_lctmap.erase (it);
					MpxLocalClientProxyTask* proxyTask = new MpxLocalClientProxyTask (
						(MpxTaskBase*) externalTaskReply->task, localClient);
					mpx->RegisterTask (proxyTask);
					proxyTask->Send (proxyTask, new MpxStartEvent (), true);
					proxyTask->Send (task, new MpxExternalTaskEvent (EPOLLIN, 0, 0, 0), false);
				}
					break;
				default:
					break;
				}
				xdr_free ((xdrproc_t) xdr_MpxMessage, (char*) rpl);
			}
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "EXTERNAL TASK: LOCAL CLIENT EVENT OUT" << endl;

		if ((localClientEvent->error () == 0) && (localClientEvent->size () != 0))
		{
			Send (task, new MpxExternalTaskEvent (EPOLLOUT, 0, 0, 0), true);
			return;
		}
		break;
	default:
		if (false)
			cout << "EXTERNAL TASK: LOCAL CLIENT EVENT default" << endl;

		break;
	}

	Send (task, new MpxExternalTaskEvent (localClientEvent->flags (), localClientEvent->error (), 0, 0), true);
	m_lctmap.erase (it);
	delete localClient;
}

void MpxExternalTask::HandleLocalEndPointEvent (MpxEventBase* event)
{
	MpxLocalEndPointEvent* localEndPointEvent = dynamic_cast <MpxLocalEndPointEvent*> (event);
	if (localEndPointEvent == 0)
		return;

	MpxLocalEndPoint* localEndPoint =
		dynamic_cast <MpxLocalEndPoint*> ((MpxLocalEndPoint*) localEndPointEvent->endPoint ());
	if (localEndPoint == 0)
		return;

	leset::iterator it = m_leset.find (localEndPoint);
	if (it == m_leset.end ())
		return;

	switch (localEndPointEvent->flags ())
	{
	case 0:
		if (false)
			cout << "EXTERNAL TASK: LOCAL END POINT EVENT 0" << endl;

		break;
	case EPOLLIN:
		if (false)
			cout << "EXTERNAL TASK: LOCAL END POINT EVENT IN" << endl;

		if ((localEndPointEvent->error () == 0) && (localEndPointEvent->size () != 0))
		{
			while (true)
			{
				MpxMessage* req = (MpxMessage*) malloc (sizeof(MpxMessage));
				memset (req, 0, sizeof(MpxMessage));
				if (localEndPoint->ReadXdrRequest ((xdrproc_t) xdr_MpxMessage, req) < 0)
				{
					free (req);
					break;
				}
				switch (req->m_Code)
				{
				case ExternalTaskRequestCode:
				{
					MpxExternalTaskRequest* externalTaskRequest = &(req->MpxMessage_u.m_externalTaskRequest);
					MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
					MpxEnvironment::BroadcastExternalEvent (mpx->externalTask (),
						new MpxTaskQueryEvent (externalTaskRequest->taskName, TaskQueryEventLocal,
							(void*) localEndPoint));
				}
					break;
				default:
					break;
				}
				xdr_free ((xdrproc_t) xdr_MpxMessage, (char*) req);
			}
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "EXTERNAL TASK: LOCAL END POINT EVENT OUT" << endl;

		if ((localEndPointEvent->error () == 0) && (localEndPointEvent->size () != 0))
		{
			return;
		}
		break;
	default:
		if (false)
			cout << "EXTERNAL TASK: LOCAL END POINT EVENT default" << endl;

		break;
	}

	m_leset.erase (it);
	delete localEndPoint;
}

void MpxExternalTask::HandleTcp4ListenerEvent (MpxEventBase* event)
{
	MpxTcp4ListenerEvent* tcp4ListenerEvent = dynamic_cast <MpxTcp4ListenerEvent*> (event);
	if (tcp4ListenerEvent == 0)
		return;
	m_t4eset.insert (new MpxTcp4EndPoint (this, tcp4ListenerEvent->fd (), true, -1L, false));
}

void MpxExternalTask::HandleTcp4ClientEvent (MpxEventBase* event)
{
	MpxTcp4ClientEvent* tcp4ClientEvent = dynamic_cast <MpxTcp4ClientEvent*> (event);
	if (tcp4ClientEvent == 0)
		return;
	MpxTcp4Client* tcp4Client = dynamic_cast <MpxTcp4Client*> ((MpxTcp4Client*) tcp4ClientEvent->endPoint ());
	if (tcp4Client == 0)
		return;

	t4tmap::iterator it = m_t4tmap.find (tcp4Client);
	if (it == m_t4tmap.end ())
		return;

	MpxTaskBase* task = it->second;

	switch (tcp4ClientEvent->flags ())
	{
	case 0:
		break;
	case EPOLLIN:
		if (false)
			cout << "EXTERNAL TASK: TCP4 CLIENT EVENT IN" << endl;

		if ((tcp4ClientEvent->error () == 0) && (tcp4ClientEvent->size () != 0))
		{
			while (true)
			{
				MpxMessage* rpl = (MpxMessage*) malloc (sizeof(MpxMessage));
				memset (rpl, 0, sizeof(MpxMessage));
				if (tcp4Client->ReadXdrRequest ((xdrproc_t) xdr_MpxMessage, rpl) < 0)
				{
					free (rpl);
					break;
				}
				switch (rpl->m_Code)
				{
				case ExternalTaskReplyCode:
				{
					MpxExternalTaskReply* externalTaskReply = &(rpl->MpxMessage_u.m_externalTaskReply);
					if ((MpxTaskBase*) externalTaskReply->task == 0)
						break;

					MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
					m_t4tmap.erase (it);
					MpxTcp4ClientProxyTask* proxyTask = new MpxTcp4ClientProxyTask (
						(MpxTaskBase*) externalTaskReply->task, tcp4Client);
					mpx->RegisterTask (proxyTask);
					proxyTask->Send (proxyTask, new MpxStartEvent (), true);
					proxyTask->Send (task, new MpxExternalTaskEvent (EPOLLIN, 0, 0, 0), false);
				}
					break;
				default:
					break;
				}
				xdr_free ((xdrproc_t) xdr_MpxMessage, (char*) rpl);
			}
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "EXTERNAL TASK: TCP4 CLIENT EVENT OUT" << endl;

		if ((tcp4ClientEvent->error () == 0) && (tcp4ClientEvent->size () != 0))
		{
			Send (task, new MpxExternalTaskEvent (EPOLLOUT, 0, 0, 0), true);
			return;
		}
		break;
	default:
		if (false)
			cout << "EXTERNAL TASK: TCP4 CLIENT EVENT default" << endl;

		break;
	}

	Send (task, new MpxExternalTaskEvent (tcp4ClientEvent->flags (), tcp4ClientEvent->error (), 0, 0), true);
	m_t4tmap.erase (it);
	delete tcp4Client;
}

void MpxExternalTask::HandleTcp4EndPointEvent (MpxEventBase* event)
{
	MpxTcp4EndPointEvent* tcp4EndPointEvent = dynamic_cast <MpxTcp4EndPointEvent*> (event);
	if (tcp4EndPointEvent == 0)
		return;

	MpxTcp4EndPoint* tcp4EndPoint = dynamic_cast <MpxTcp4EndPoint*> ((MpxTcp4EndPoint*) tcp4EndPointEvent->endPoint ());
	if (tcp4EndPoint == 0)
		return;

	t4eset::iterator it = m_t4eset.find (tcp4EndPoint);
	if (it == m_t4eset.end ())
		return;

	switch (tcp4EndPointEvent->flags ())
	{
	case 0:
		if (false)
			cout << "EXTERNAL TASK: TCP4 END POINT EVENT 0" << endl;

		break;
	case EPOLLIN:
		if (false)
			cout << "EXTERNAL TASK: TCP4 END POINT EVENT IN" << endl;

		if ((tcp4EndPointEvent->error () == 0) && (tcp4EndPointEvent->size () != 0))
		{
			while (true)
			{
				MpxMessage* req = (MpxMessage*) malloc (sizeof(MpxMessage));
				memset (req, 0, sizeof(MpxMessage));
				if (tcp4EndPoint->ReadXdrRequest ((xdrproc_t) xdr_MpxMessage, req) < 0)
				{
					free (req);
					break;
				}
				switch (req->m_Code)
				{
				case ExternalTaskRequestCode:
				{
					MpxExternalTaskRequest* externalTaskRequest = &(req->MpxMessage_u.m_externalTaskRequest);
					MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
					MpxEnvironment::BroadcastExternalEvent (mpx->externalTask (),
						new MpxTaskQueryEvent (externalTaskRequest->taskName, TaskQueryEventTcp4,
							(void*) tcp4EndPoint));
				}
					break;
				default:
					break;
				}
				xdr_free ((xdrproc_t) xdr_MpxMessage, (char*) req);
			}
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "EXTERNAL TASK: TCP4 END POINT EVENT OUT" << endl;

		if ((tcp4EndPointEvent->error () == 0) && (tcp4EndPointEvent->size () != 0))
		{
			return;
		}
		break;
	default:
		if (false)
			cout << "EXTERNAL TASK: TCP4 END POINT EVENT default" << endl;

		break;
	}

	m_t4eset.erase (it);
	delete tcp4EndPoint;
}

void MpxExternalTask::HandleTcp6ListenerEvent (MpxEventBase* event)
{
	MpxTcp6ListenerEvent* tcp6ListenerEvent = dynamic_cast <MpxTcp6ListenerEvent*> (event);
	if (tcp6ListenerEvent == 0)
		return;
	m_t6eset.insert (new MpxTcp6EndPoint (this, tcp6ListenerEvent->fd (), true, -1L, false));
}

void MpxExternalTask::HandleTcp6ClientEvent (MpxEventBase* event)
{
	MpxTcp6ClientEvent* tcp6ClientEvent = dynamic_cast <MpxTcp6ClientEvent*> (event);
	if (tcp6ClientEvent == 0)
		return;
	MpxTcp6Client* tcp6Client = dynamic_cast <MpxTcp6Client*> ((MpxTcp6Client*) tcp6ClientEvent->endPoint ());
	if (tcp6Client == 0)
		return;

	t6tmap::iterator it = m_t6tmap.find (tcp6Client);
	if (it == m_t6tmap.end ())
		return;

	MpxTaskBase* task = it->second;

	switch (tcp6ClientEvent->flags ())
	{
	case 0:
		break;
	case EPOLLIN:
		if (false)
			cout << "EXTERNAL TASK: TCP6 CLIENT EVENT IN" << endl;

		if ((tcp6ClientEvent->error () == 0) && (tcp6ClientEvent->size () != 0))
		{
			while (true)
			{
				MpxMessage* rpl = (MpxMessage*) malloc (sizeof(MpxMessage));
				memset (rpl, 0, sizeof(MpxMessage));
				if (tcp6Client->ReadXdrRequest ((xdrproc_t) xdr_MpxMessage, rpl) < 0)
				{
					free (rpl);
					break;
				}
				switch (rpl->m_Code)
				{
				case ExternalTaskReplyCode:
				{
					MpxExternalTaskReply* externalTaskReply = &(rpl->MpxMessage_u.m_externalTaskReply);
					if ((MpxTaskBase*) externalTaskReply->task == 0)
						break;

					MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
					m_t6tmap.erase (it);
					MpxTcp6ClientProxyTask* proxyTask = new MpxTcp6ClientProxyTask (
						(MpxTaskBase*) externalTaskReply->task, tcp6Client);
					mpx->RegisterTask (proxyTask);
					proxyTask->Send (proxyTask, new MpxStartEvent (), true);
					proxyTask->Send (task, new MpxExternalTaskEvent (EPOLLIN, 0, 0, 0), false);
				}
					break;
				default:
					break;
				}
				xdr_free ((xdrproc_t) xdr_MpxMessage, (char*) rpl);
			}
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "EXTERNAL TASK: TCP6 CLIENT EVENT OUT" << endl;

		if ((tcp6ClientEvent->error () == 0) && (tcp6ClientEvent->size () != 0))
		{
			Send (task, new MpxExternalTaskEvent (EPOLLOUT, 0, 0, 0), true);
			return;
		}
		break;
	default:
		if (false)
			cout << "EXTERNAL TASK: TCP6 CLIENT EVENT default" << endl;

		break;
	}

	Send (task, new MpxExternalTaskEvent (tcp6ClientEvent->flags (), tcp6ClientEvent->error (), 0, 0), true);
	m_t6tmap.erase (it);
	delete tcp6Client;
}

void MpxExternalTask::HandleTcp6EndPointEvent (MpxEventBase* event)
{
	MpxTcp6EndPointEvent* tcp6EndPointEvent = dynamic_cast <MpxTcp6EndPointEvent*> (event);
	if (tcp6EndPointEvent == 0)
		return;

	MpxTcp6EndPoint* tcp6EndPoint = dynamic_cast <MpxTcp6EndPoint*> ((MpxTcp6EndPoint*) tcp6EndPointEvent->endPoint ());
	if (tcp6EndPoint == 0)
		return;

	t6eset::iterator it = m_t6eset.find (tcp6EndPoint);
	if (it == m_t6eset.end ())
		return;

	switch (tcp6EndPointEvent->flags ())
	{
	case 0:
		if (false)
			cout << "EXTERNAL TASK: TCP4 END POINT EVENT 0" << endl;

		break;
	case EPOLLIN:
		if (false)
			cout << "EXTERNAL TASK: TCP4 END POINT EVENT IN" << endl;

		if ((tcp6EndPointEvent->error () == 0) && (tcp6EndPointEvent->size () != 0))
		{
			while (true)
			{
				MpxMessage* req = (MpxMessage*) malloc (sizeof(MpxMessage));
				memset (req, 0, sizeof(MpxMessage));
				if (tcp6EndPoint->ReadXdrRequest ((xdrproc_t) xdr_MpxMessage, req) < 0)
				{
					free (req);
					break;
				}
				switch (req->m_Code)
				{
				case ExternalTaskRequestCode:
				{
					MpxExternalTaskRequest* externalTaskRequest = &(req->MpxMessage_u.m_externalTaskRequest);
					MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
					MpxEnvironment::BroadcastExternalEvent (mpx->externalTask (),
						new MpxTaskQueryEvent (externalTaskRequest->taskName, TaskQueryEventTcp6,
							(void*) tcp6EndPoint));
				}
					break;
				default:
					break;
				}
				xdr_free ((xdrproc_t) xdr_MpxMessage, (char*) req);
			}
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "EXTERNAL TASK: TCP4 END POINT EVENT OUT" << endl;

		if ((tcp6EndPointEvent->error () == 0) && (tcp6EndPointEvent->size () != 0))
		{
			return;
		}
		break;
	default:
		if (false)
			cout << "EXTERNAL TASK: TCP4 END POINT EVENT default" << endl;

		break;
	}

	m_t6eset.erase (it);
	delete tcp6EndPoint;
}

void MpxExternalTask::HandleTaskQueryEvent (MpxEventBase* event)
{
	MpxTaskQueryEvent* taskQueryEvent = dynamic_cast <MpxTaskQueryEvent*> (event);
	if (taskQueryEvent == 0)
		return;
	const char* taskName = taskQueryEvent->taskName ();
	MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
	MpxTaskBase* task = mpx->RetrieveTask (taskName);
	Send ((MpxTaskBase*) event->src (),
		new MpxTaskResponseEvent (task, taskQueryEvent->queryType (), taskQueryEvent->endPoint ()));
}

void MpxExternalTask::HandleTaskResponseEvent (MpxEventBase* event)
{
	MpxTaskResponseEvent* taskResponseEvent = dynamic_cast <MpxTaskResponseEvent*> (event);
	if (taskResponseEvent == 0)
		return;
	MpxTaskBase* task = taskResponseEvent->task ();
	if (task != 0)
		cout << "task = " << task << endl;
	switch (taskResponseEvent->queryType ())
	{
	case TaskQueryEventLocal:
	{
		MpxLocalEndPoint* localEndPoint = (MpxLocalEndPoint*) taskResponseEvent->endPoint ();
		leset::iterator it = m_leset.find (localEndPoint);
		if (it == m_leset.end ())
			break;

		MpxMessage msg;
		msg.m_Code = ExternalTaskReplyCode;
		msg.MpxMessage_u.m_externalTaskReply.task = (long) task;
		if (localEndPoint->PostXdrRequest ((xdrproc_t) xdr_MpxMessage, &msg) < 0)
			break;

		MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
		m_leset.erase (it);
		MpxLocalEndPointProxyTask* proxyTask = new MpxLocalEndPointProxyTask (task, localEndPoint);
		mpx->RegisterTask (proxyTask);
		Send (proxyTask, new MpxStartEvent (), true);
	}
		break;
	case TaskQueryEventTcp4:
	{
		MpxTcp4EndPoint* tcp4EndPoint = (MpxTcp4EndPoint*) taskResponseEvent->endPoint ();
		t4eset::iterator it = m_t4eset.find (tcp4EndPoint);
		if (it == m_t4eset.end ())
			break;

		MpxMessage msg;
		msg.m_Code = ExternalTaskReplyCode;
		msg.MpxMessage_u.m_externalTaskReply.task = (long) task;
		if (tcp4EndPoint->PostXdrRequest ((xdrproc_t) xdr_MpxMessage, &msg) < 0)
			break;

		MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
		m_t4eset.erase (it);
		MpxTcp4EndPointProxyTask* proxyTask = new MpxTcp4EndPointProxyTask (task, tcp4EndPoint);
		mpx->RegisterTask (proxyTask);
		Send (proxyTask, new MpxStartEvent (), true);
	}
		break;
	case TaskQueryEventTcp6:
	{
		MpxTcp6EndPoint* tcp6EndPoint = (MpxTcp6EndPoint*) taskResponseEvent->endPoint ();
		t6eset::iterator it = m_t6eset.find (tcp6EndPoint);
		if (it == m_t6eset.end ())
			break;

		MpxMessage msg;
		msg.m_Code = ExternalTaskReplyCode;
		msg.MpxMessage_u.m_externalTaskReply.task = (long) task;
		if (tcp6EndPoint->PostXdrRequest ((xdrproc_t) xdr_MpxMessage, &msg) < 0)
			break;

		MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) this->mpx ();
		m_t6eset.erase (it);
		MpxTcp6EndPointProxyTask* proxyTask = new MpxTcp6EndPointProxyTask (task, tcp6EndPoint);
		mpx->RegisterTask (proxyTask);
		Send (proxyTask, new MpxStartEvent (), true);
	}
		break;
	default:
		break;
	}
}

} /* namespace mpx */
