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
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

const char* MpxExternalTask::g_protocolField = "protocol:";
const char* MpxExternalTask::g_protoLocal = "local,";
const char* MpxExternalTask::g_protoTcp4 = "tcp4,";
const char* MpxExternalTask::g_protoTcp6 = "tcp6,";
const char* MpxExternalTask::g_pathField = "path:";
const char* MpxExternalTask::g_addressField = "address:";
const char* MpxExternalTask::g_portField = "port:";

EventDescriptor MpxExternalTask::g_evntab [] =
{
{ AnyState, MpxLocalTaskQueryEvent::EventCode, HandleLocalTaskQueryEvent },
{ AnyState, MpxTcp4TaskQueryEvent::EventCode, HandleTcp4TaskQueryEvent },
{ AnyState, MpxTcp6TaskQueryEvent::EventCode, HandleTcp6TaskQueryEvent },
{ AnyState, MpxLocalListenerEvent::EventCode, HandleLocalListenerEvent },
{ AnyState, MpxLocalClientEvent::EventCode, HandleLocalClientEvent },
{ AnyState, MpxLocalEndPointEvent::EventCode, HandleLocalEndPointEvent },
{ AnyState, MpxTcp4ListenerEvent::EventCode, HandleTcp4ListenerEvent },
{ AnyState, MpxTcp4ClientEvent::EventCode, HandleTcp4ClientEvent },
{ AnyState, MpxTcp4EndPointEvent::EventCode, HandleTcp4EndPointEvent },
{ AnyState, MpxTcp6ListenerEvent::EventCode, HandleTcp6ListenerEvent },
{ AnyState, MpxTcp6ClientEvent::EventCode, HandleTcp6ClientEvent },
{ AnyState, MpxTcp6EndPointEvent::EventCode, HandleTcp6EndPointEvent },
{ AnyState, MpxTaskQueryEvent::EventCode, HandleTaskQueryEvent },
{ AnyState, MpxTaskResponseEvent::EventCode, HandleTaskResponseEvent },
{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
{ AnyState, MpxProxyTaskRelocationEvent::EventCode, HandleProxyTaskRelocationEvent },
{ 0, 0, 0 } };

MpxTaskBase::evnset MpxExternalTask::g_evnset = MpxTaskBase::CreateEventSet (MpxExternalTask::g_evntab);

MpxExternalTask::MpxExternalTask (const char* connStr) :
	MpxTaskBase (g_evnset)
{
	m_connStr = (connStr != 0) ? connStr : "";
	m_started = false;
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

	char* mpath = reinterpret_cast <char*> (alloca((size = connStr - path) + 1));
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
	char* maddress = reinterpret_cast <char*> (alloca((size = connStr - address) + 1));
	strncpy (maddress, address, size);
	maddress [size] = 0;
	++connStr;

	if (strncasecmp (connStr, g_portField, strlen (g_portField)) != 0)
		return 0;

	connStr += strlen (g_portField);
	const char* port = connStr;
	if ((connStr = strchr (connStr, ';')) == 0)
		return 0;
	char* mport = reinterpret_cast <char*> (alloca((size = connStr - port) + 1));
	strncpy (mport, port, size);
	mport [size] = 0;
	++connStr;

	if (strcasecmp (maddress, "any") == 0)
	{
		MpxTcp4Listener* tcp4Listener = new MpxTcp4Listener (this, false);
		if (tcp4Listener == 0)
			return connStr;

		if ((tcp4Listener->CreateListener (static_cast <in_port_t> (atoi (mport))) != 0)
			|| (tcp4Listener->StartListener () != 0))
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

			if ((tcp4Listener->CreateListener (ifptr->ifa_addr, static_cast <in_port_t> (atoi (mport))) != 0)
				|| (tcp4Listener->StartListener () != 0))
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

		if ((tcp4Listener->CreateListener (maddress, static_cast <in_port_t> (atoi (mport))) != 0)
			|| (tcp4Listener->StartListener () != 0))
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
	char* maddress = reinterpret_cast <char*> (alloca((size = connStr - address) + 1));
	strncpy (maddress, address, size);
	maddress [size] = 0;
	++connStr;

	if (strncasecmp (connStr, g_portField, strlen (g_portField)) != 0)
		return 0;

	connStr += strlen (g_portField);
	const char* port = connStr;
	if ((connStr = strchr (connStr, ';')) == 0)
		return 0;
	char* mport = reinterpret_cast <char*> (alloca((size = connStr - port) + 1));
	strncpy (mport, port, size);
	mport [size] = 0;
	++connStr;

	if (strcasecmp (maddress, "any") == 0)
	{
		MpxTcp6Listener* tcp6Listener = new MpxTcp6Listener (this, false);
		if (tcp6Listener == 0)
			return connStr;

		if ((tcp6Listener->CreateListener (static_cast <in_port_t> (atoi (mport))) != 0)
			|| (tcp6Listener->StartListener () != 0))
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

			if ((tcp6Listener->CreateListener (ifptr->ifa_addr, static_cast <in_port_t> (atoi (mport))) != 0)
				|| (tcp6Listener->StartListener () != 0))
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

		if ((tcp6Listener->CreateListener (maddress, static_cast <in_port_t> (atoi (mport))) != 0)
			|| (tcp6Listener->StartListener () != 0))
		{
			delete tcp6Listener;
			return connStr;
		}
		m_t6lset.insert (tcp6Listener);
	}

	return connStr;
}

void MpxExternalTask::StartTask ()
{
	if (m_started)
		return;
	CreateListeners (m_connStr.c_str ());
	m_started = true;
}

void MpxExternalTask::StopTask ()
{
	Release ();
}

void MpxExternalTask::HandleLocalTaskQueryEvent (MpxEventBase* event)
{
	MpxLocalTaskQueryEvent* localTaskQueryEvent = dynamic_cast <MpxLocalTaskQueryEvent*> (event);
	if (localTaskQueryEvent == 0)
	{
		cout << "event is not MpxLocalTaskQueryEvent" << endl;
		return;
	}

	MpxTaskBase* srcTask = reinterpret_cast <MpxTaskBase*> (localTaskQueryEvent->src ());
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
	const char* encdeclib = localTaskQueryEvent->encdeclib ();
	MpxMessage msg;
	msg.m_Code = ExternalTaskRequestCode;
	msg.MpxMessage_u.m_externalTaskRequest.taskName = reinterpret_cast <char*> (strdup (name));
	msg.MpxMessage_u.m_externalTaskRequest.encdeclib = reinterpret_cast <char*> (strdup (encdeclib));
	localClient->PostXdrRequest (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), reinterpret_cast <void*> (&msg));
	m_lctmap [localClient] = srcTask;

	if (false)
		cout << "EXTERNAL TASK: WRITE: " << name << endl;
}

void MpxExternalTask::HandleTcp4TaskQueryEvent (MpxEventBase* event)
{
	MpxTcp4TaskQueryEvent* tcp4TaskQueryEvent = dynamic_cast <MpxTcp4TaskQueryEvent*> (event);
	if (tcp4TaskQueryEvent == 0)
	{
		cout << "event is not MpxTcp4TaskQueryEvent" << endl;
		return;
	}

	MpxTaskBase* srcTask = reinterpret_cast <MpxTaskBase*> (tcp4TaskQueryEvent->src ());
	if (srcTask == 0)
		return;

	addrinfo hint;
	memset (&hint, 0, sizeof(addrinfo));
	hint.ai_flags = AI_ALL;
	hint.ai_family = AF_INET;

	MpxWorkingQueue::Put (
		new MpxExtTaskAddrInfo (this, tcp4TaskQueryEvent->hostname (),
			dynamic_cast <MpxEventBase*> (tcp4TaskQueryEvent->Copy ()), AF_INET, &hint));
}

void MpxExternalTask::HandleTcp6TaskQueryEvent (MpxEventBase* event)
{
	MpxTcp6TaskQueryEvent* tcp6TaskQueryEvent = dynamic_cast <MpxTcp6TaskQueryEvent*> (event);
	if (tcp6TaskQueryEvent == 0)
		return;

	MpxTaskBase* srcTask = reinterpret_cast <MpxTaskBase*> (tcp6TaskQueryEvent->src ());
	if (srcTask == 0)
		return;

	addrinfo hint;
	memset (&hint, 0, sizeof(addrinfo));
	hint.ai_flags = AI_ALL;
	hint.ai_family = AF_INET6;

	MpxWorkingQueue::Put (
		new MpxExtTaskAddrInfo (this, tcp6TaskQueryEvent->hostname (),
			reinterpret_cast <MpxEventBase*> (tcp6TaskQueryEvent->Copy ()), AF_INET6, &hint));
}

void MpxExternalTask::HandleJobFinishedEvent (MpxEventBase* event)
{
	MpxJobFinishedEvent* jobFinishedEvent = dynamic_cast <MpxJobFinishedEvent*> (event);
	if (jobFinishedEvent == 0)
		return;

	MpxExtTaskAddrInfo* extTaskAddrInfo = dynamic_cast <MpxExtTaskAddrInfo*> (jobFinishedEvent->job ());
	if (extTaskAddrInfo == 0)
		return;

	struct addrinfo* results = extTaskAddrInfo->results ();
	addrinfo* addrPtr;
	for (addrPtr = results; (addrPtr != 0) && (addrPtr->ai_protocol != IPPROTO_TCP); addrPtr = addrPtr->ai_next)
		;
	if (addrPtr == 0)
	{
		freeaddrinfo (results);
		delete extTaskAddrInfo;
		return;
	}

	sockaddr* addr = addrPtr->ai_addr;

	switch (extTaskAddrInfo->family ())
	{
	case AF_INET:
	{
		MpxTcp4TaskQueryEvent* tcp4TaskQueryEvent = dynamic_cast <MpxTcp4TaskQueryEvent*> (extTaskAddrInfo->query ());
		if (tcp4TaskQueryEvent == 0)
			break;

		MpxTcp4Client* tcp4Client = new MpxTcp4Client (this, true, -1, true);
		MpxTaskBase* consumer = reinterpret_cast <MpxTaskBase*> (tcp4TaskQueryEvent->src ());
		int port = atoi (tcp4TaskQueryEvent->port ());
		if (tcp4Client->Connect (reinterpret_cast <sockaddr_in*> (addr), port) != 0)
		{
			Send (consumer, new MpxExternalTaskEvent (0, -1, 0, 0), true);
			delete tcp4Client;
			break;
		}

		const char* name = tcp4TaskQueryEvent->name ();
		const char* encdeclib = tcp4TaskQueryEvent->encdeclib ();
		MpxMessage msg;
		msg.m_Code = ExternalTaskRequestCode;
		msg.MpxMessage_u.m_externalTaskRequest.taskName = reinterpret_cast <char*> (strdup (name));
		msg.MpxMessage_u.m_externalTaskRequest.encdeclib = reinterpret_cast <char*> (strdup (encdeclib));
		tcp4Client->PostXdrRequest (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), reinterpret_cast <void*> (&msg));
		m_t4tmap [tcp4Client] = consumer;

		if (false)
			cout << "EXTERNAL TASK: WRITE: " << name << endl;
	}
		break;
	case AF_INET6:
	{
		MpxTcp6TaskQueryEvent* tcp6TaskQueryEvent = dynamic_cast <MpxTcp6TaskQueryEvent*> (extTaskAddrInfo->query ());
		if (tcp6TaskQueryEvent == 0)
			break;

		MpxTcp6Client* tcp6Client = new MpxTcp6Client (this, true, -1, true);
		MpxTaskBase* consumer = reinterpret_cast <MpxTaskBase*> (tcp6TaskQueryEvent->src ());
		int port = atoi (tcp6TaskQueryEvent->port ());
		if (tcp6Client->Connect ((sockaddr_in6*) addr, port) != 0)
		{
			Send (consumer, new MpxExternalTaskEvent (0, -1, 0, 0), true);
			delete tcp6Client;
			break;
		}

		const char* name = tcp6TaskQueryEvent->name ();
		const char* encdeclib = tcp6TaskQueryEvent->encdeclib ();
		MpxMessage msg;
		msg.m_Code = ExternalTaskRequestCode;
		msg.MpxMessage_u.m_externalTaskRequest.taskName = reinterpret_cast <char*> (strdup (name));
		msg.MpxMessage_u.m_externalTaskRequest.encdeclib = reinterpret_cast <char*> (strdup (encdeclib));
		tcp6Client->PostXdrRequest (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), reinterpret_cast <void*> (&msg));
		m_t6tmap [tcp6Client] = consumer;

		if (false)
			cout << "EXTERNAL TASK: WRITE: " << name << endl;
	}
		break;
	default:
		break;
	}

	freeaddrinfo (results);
	delete extTaskAddrInfo;
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
	MpxLocalClient* localClient = reinterpret_cast <MpxLocalClient*> (localClientEvent->endPoint ());
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
				MpxMessage* rpl = reinterpret_cast <MpxMessage*> (malloc (sizeof(MpxMessage)));
				memset (rpl, 0, sizeof(MpxMessage));
				if (localClient->ReadXdrRequest (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), rpl) < 0)
				{
					free (rpl);
					break;
				}
				switch (rpl->m_Code)
				{
				case ExternalTaskReplyCode:
				{
					if (false)
						cout << "local external task reply" << endl;
					MpxExternalTaskReply* externalTaskReply = &(rpl->MpxMessage_u.m_externalTaskReply);
					if ((MpxTaskBase*) externalTaskReply->task == 0)
						break;

					MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
					m_lctmap.erase (it);
					MpxLocalClientProxyTask* proxyTask = new MpxLocalClientProxyTask (task,
						externalTaskReply->encdeclib, localClient);
					mpx->RegisterTask (proxyTask);
					proxyTask->Send (proxyTask, new MpxStartEvent (), true);
					if (false)
						cout << "local proxy client created" << endl;
				}
					break;
				default:
					break;
				}
				xdr_free (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), reinterpret_cast <char*> (rpl));
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

	Send (task, new MpxExternalTaskEvent (0, -1, 0, 0), true);
	m_lctmap.erase (it);
	delete localClient;
}

void MpxExternalTask::HandleLocalEndPointEvent (MpxEventBase* event)
{
	MpxLocalEndPointEvent* localEndPointEvent = dynamic_cast <MpxLocalEndPointEvent*> (event);
	if (localEndPointEvent == 0)
		return;

	MpxLocalEndPoint* localEndPoint = reinterpret_cast <MpxLocalEndPoint*> (localEndPointEvent->endPoint ());
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
				MpxMessage* req = reinterpret_cast <MpxMessage*> (malloc (sizeof(MpxMessage)));
				memset (req, 0, sizeof(MpxMessage));
				if (localEndPoint->ReadXdrRequest (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), req) < 0)
				{
					free (req);
					break;
				}
				switch (req->m_Code)
				{
				case ExternalTaskRequestCode:
				{
					MpxExternalTaskRequest* externalTaskRequest = &(req->MpxMessage_u.m_externalTaskRequest);
					MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
					MpxEnvironment::BroadcastExternalEvent (mpx->externalTask (),
						new MpxTaskQueryEvent (externalTaskRequest->taskName, externalTaskRequest->encdeclib,
							TaskQueryEventLocal, (void*) localEndPoint));
				}
					break;
				default:
					break;
				}
				xdr_free (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), reinterpret_cast <char*> (req));
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
	MpxTcp4Client* tcp4Client = reinterpret_cast <MpxTcp4Client*> (tcp4ClientEvent->endPoint ());
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
				MpxMessage* rpl = reinterpret_cast <MpxMessage*> (malloc (sizeof(MpxMessage)));
				memset (rpl, 0, sizeof(MpxMessage));
				if (tcp4Client->ReadXdrRequest (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), rpl) < 0)
				{
					free (rpl);
					break;
				}
				switch (rpl->m_Code)
				{
				case ExternalTaskReplyCode:
				{
					if (false)
						cout << "tcp4 external task reply" << endl;
					MpxExternalTaskReply* externalTaskReply = &(rpl->MpxMessage_u.m_externalTaskReply);
					if (reinterpret_cast <MpxTaskBase*> (externalTaskReply->task) == 0)
						break;

					MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
					m_t4tmap.erase (it);
					MpxTcp4ClientProxyTask* proxyTask = new MpxTcp4ClientProxyTask (task, externalTaskReply->encdeclib,
						tcp4Client);
					mpx->RegisterTask (proxyTask);
					proxyTask->Send (proxyTask, new MpxStartEvent (), true);
					if (true)
						cout << "tcp4 proxy client created" << endl;
				}
					break;
				default:
					break;
				}
				xdr_free (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), reinterpret_cast <char*> (rpl));
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

	Send (task, new MpxExternalTaskEvent (0, -1, 0, 0), true);
	m_t4tmap.erase (it);
	delete tcp4Client;
}

void MpxExternalTask::HandleTcp4EndPointEvent (MpxEventBase* event)
{
	MpxTcp4EndPointEvent* tcp4EndPointEvent = dynamic_cast <MpxTcp4EndPointEvent*> (event);
	if (tcp4EndPointEvent == 0)
		return;

	MpxTcp4EndPoint* tcp4EndPoint = reinterpret_cast <MpxTcp4EndPoint*> (tcp4EndPointEvent->endPoint ());
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
				MpxMessage* req = reinterpret_cast <MpxMessage*> (malloc (sizeof(MpxMessage)));
				memset (req, 0, sizeof(MpxMessage));
				if (tcp4EndPoint->ReadXdrRequest (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), req) < 0)
				{
					free (req);
					break;
				}
				switch (req->m_Code)
				{
				case ExternalTaskRequestCode:
				{
					MpxExternalTaskRequest* externalTaskRequest = &(req->MpxMessage_u.m_externalTaskRequest);
					MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
					MpxEnvironment::BroadcastExternalEvent (mpx->externalTask (),
						new MpxTaskQueryEvent (externalTaskRequest->taskName, externalTaskRequest->encdeclib,
							TaskQueryEventTcp4, (void*) tcp4EndPoint));
				}
					break;
				default:
					break;
				}
				xdr_free (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), reinterpret_cast <char*> (req));
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
	MpxTcp6Client* tcp6Client = reinterpret_cast <MpxTcp6Client*> (tcp6ClientEvent->endPoint ());
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
				MpxMessage* rpl = reinterpret_cast <MpxMessage*> (malloc (sizeof(MpxMessage)));
				memset (rpl, 0, sizeof(MpxMessage));
				if (tcp6Client->ReadXdrRequest (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), rpl) < 0)
				{
					free (rpl);
					break;
				}
				switch (rpl->m_Code)
				{
				case ExternalTaskReplyCode:
				{
					if (false)
						cout << "tcp6 external task reply" << endl;
					MpxExternalTaskReply* externalTaskReply = &(rpl->MpxMessage_u.m_externalTaskReply);
					if (reinterpret_cast <MpxTaskBase*> (externalTaskReply->task) == 0)
						break;

					MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
					m_t6tmap.erase (it);
					MpxTcp6ClientProxyTask* proxyTask = new MpxTcp6ClientProxyTask (task, externalTaskReply->encdeclib,
						tcp6Client);
					mpx->RegisterTask (proxyTask);
					proxyTask->Send (proxyTask, new MpxStartEvent (), true);
					if (false)
						cout << "tcp6 proxy client created" << endl;
				}
					break;
				default:
					break;
				}
				xdr_free (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), reinterpret_cast <char*> (rpl));
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

	Send (task, new MpxExternalTaskEvent (0, -1, 0, 0), true);
	m_t6tmap.erase (it);
	delete tcp6Client;
}

void MpxExternalTask::HandleTcp6EndPointEvent (MpxEventBase* event)
{
	MpxTcp6EndPointEvent* tcp6EndPointEvent = dynamic_cast <MpxTcp6EndPointEvent*> (event);
	if (tcp6EndPointEvent == 0)
		return;

	MpxTcp6EndPoint* tcp6EndPoint = reinterpret_cast <MpxTcp6EndPoint*> (tcp6EndPointEvent->endPoint ());
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
				MpxMessage* req = reinterpret_cast <MpxMessage*> (malloc (sizeof(MpxMessage)));
				memset (req, 0, sizeof(MpxMessage));
				if (tcp6EndPoint->ReadXdrRequest (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), req) < 0)
				{
					free (req);
					break;
				}
				switch (req->m_Code)
				{
				case ExternalTaskRequestCode:
				{
					MpxExternalTaskRequest* externalTaskRequest = &(req->MpxMessage_u.m_externalTaskRequest);
					MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
					MpxEnvironment::BroadcastExternalEvent (mpx->externalTask (),
						new MpxTaskQueryEvent (externalTaskRequest->taskName, externalTaskRequest->encdeclib,
							TaskQueryEventTcp6, (void*) tcp6EndPoint));
				}
					break;
				default:
					break;
				}
				xdr_free (reinterpret_cast <xdrproc_t> (xdr_MpxMessage), reinterpret_cast <char*> (req));
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
	MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
	MpxTaskBase* task = mpx->RetrieveTask (taskName);
	Send (reinterpret_cast <MpxTaskBase*> (event->src ()),
		new MpxTaskResponseEvent (task, taskQueryEvent->encdeclib (), taskQueryEvent->queryType (),
			taskQueryEvent->endPoint ()));
}

void MpxExternalTask::HandleTaskResponseEvent (MpxEventBase* event)
{
	MpxTaskResponseEvent* taskResponseEvent = dynamic_cast <MpxTaskResponseEvent*> (event);
	if (taskResponseEvent == 0)
		return;

	MpxTaskBase* task = taskResponseEvent->task ();
	const char* encdeclib = taskResponseEvent->encdeclib ();
	if (task != 0)
		switch (taskResponseEvent->queryType ())
		{
		case TaskQueryEventLocal:
		{
			MpxLocalEndPoint* localEndPoint = reinterpret_cast <MpxLocalEndPoint*> (taskResponseEvent->endPoint ());
			leset::iterator it = m_leset.find (localEndPoint);
			if (it == m_leset.end ())
				break;

			m_leset.erase (it);
			MpxLocalEndPointProxyTask *proxyTask = new MpxLocalEndPointProxyTask (task, encdeclib, localEndPoint);
			if (task->mpx () != mpx ())
			{
				proxyTask->DisconnectFromContext ();
				Send (reinterpret_cast <MpxTaskBase*> (event->src ()), new MpxProxyTaskRelocationEvent (proxyTask));
			}
			else
			{
				MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
				mpx->RegisterTask (proxyTask);
				Send (proxyTask, new MpxStartEvent (), true);
			}
			if (false)
				cout << "local proxy end-point created" << endl;
		}
			break;
		case TaskQueryEventTcp4:
		{
			MpxTcp4EndPoint* tcp4EndPoint = reinterpret_cast <MpxTcp4EndPoint*> (taskResponseEvent->endPoint ());
			t4eset::iterator it = m_t4eset.find (tcp4EndPoint);
			if (it == m_t4eset.end ())
				break;

			m_t4eset.erase (it);
			MpxTcp4EndPointProxyTask *proxyTask = new MpxTcp4EndPointProxyTask (task, encdeclib, tcp4EndPoint);
			if (task->mpx () != mpx ())
			{
				proxyTask->DisconnectFromContext ();
				Send (reinterpret_cast <MpxTaskBase*> (event->src ()), new MpxProxyTaskRelocationEvent (proxyTask));
			}
			else
			{
				MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
				mpx->RegisterTask (proxyTask);
				Send (proxyTask, new MpxStartEvent (), true);
			}
			if (true)
				cout << "tcp4 proxy end-point created" << endl;
		}
			break;
		case TaskQueryEventTcp6:
		{
			MpxTcp6EndPoint* tcp6EndPoint = reinterpret_cast <MpxTcp6EndPoint*> (taskResponseEvent->endPoint ());
			t6eset::iterator it = m_t6eset.find (tcp6EndPoint);
			if (it == m_t6eset.end ())
				break;

			m_t6eset.erase (it);
			MpxTcp6EndPointProxyTask *proxyTask = new MpxTcp6EndPointProxyTask (task, encdeclib, tcp6EndPoint);
			if (task->mpx () != mpx ())
			{
				proxyTask->DisconnectFromContext ();
				Send (reinterpret_cast <MpxTaskBase*> (event->src ()), new MpxProxyTaskRelocationEvent (proxyTask));
			}
			else
			{
				MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
				mpx->RegisterTask (proxyTask);
				Send (proxyTask, new MpxStartEvent (), true);
			}
			if (false)
				cout << "tcp6 proxy end-point created" << endl;
		}
			break;
		default:
			break;
		}
	else
	{
		if (false)
			cout << "NO TASK FOUND" << endl;
	}
}

void MpxExternalTask::HandleProxyTaskRelocationEvent (MpxEventBase* event)
{
	MpxProxyTaskRelocationEvent *proxyTaskRelocationEvent = dynamic_cast <MpxProxyTaskRelocationEvent*> (event);
	if (proxyTaskRelocationEvent == 0)
		return;

	MpxProxyTaskBase* proxyTask = dynamic_cast <MpxProxyTaskBase*> (proxyTaskRelocationEvent->proxyTask ());
	MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (this->mpx ());
	mpx->RegisterTask (proxyTask);
	proxyTask->ConnectToContext ();
	Send (proxyTask, new MpxStartEvent (), true);
}

} /* namespace mpx */
