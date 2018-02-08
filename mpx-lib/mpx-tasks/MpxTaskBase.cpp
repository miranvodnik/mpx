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

#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-tasks/MpxTaskBase.h>
#include <mpx-tasks/MpxProxyTask.h>
#include <mpx-event-queues/MpxEventQueueRepository.h>
#include <mpx-events/MpxLocalTaskQueryEvent.h>
#include <mpx-events/MpxTcp4TaskQueryEvent.h>
#include <mpx-events/MpxTcp6TaskQueryEvent.h>

#include <typeinfo>
using namespace std;

namespace mpx
{

const char* MpxTaskBase::g_protocolField = "protocol:";
const char* MpxTaskBase::g_protocolLocal = "local,";
const char* MpxTaskBase::g_protocolTcp4 = "tcp4,";
const char* MpxTaskBase::g_protocolTcp6 = "tcp6,";
const char* MpxTaskBase::g_pathField = "path:";
const char* MpxTaskBase::g_portField = "port:";
const char* MpxTaskBase::g_nameField = "name:";
const char* MpxTaskBase::g_hostnameField = "hostname:";

int MpxTaskBase::g_sentCount = 0;
bool MpxTaskBase::g_enableSend = true;

MpxTaskBase::MpxTaskBase (const char* name)
{
	m_mpx = 0;
	m_name = (name != 0) ? name : "";
	m_state = StartState;
}

MpxTaskBase::~MpxTaskBase ()
{
	m_mpx = 0;
	m_evnset.clear ();
}

void MpxTaskBase::Dispose (bool release)
{
	((MpxTaskMultiplexer*) mpx ())->DisposeTask (this, release);
}

MpxTaskBase::evndata MpxTaskBase::RegisterEventHandler (unsigned int stateCode, unsigned int eventCode, evnfunc f,
	mpx_appdt_t data)
{
	evnkey key (stateCode, eventCode);
	evnset::iterator it = m_evnset.find (key);
	if (it != m_evnset.end ())
		return it->second;
	return m_evnset [key] = evndata (f, data);
}

MpxTaskBase::evndata MpxTaskBase::RetrieveEventHandler (u_int state, u_int event)
{
	evnset::iterator it = m_evnset.find (evnkey (state, event));
	if (it != m_evnset.end ())
		return it->second;
	return evndata (0, 0);
}

int MpxTaskBase::HandleEvent (MpxEventBase* event)
{
	if (false)
		cout << "HANDLE EVENT " << event->code () << endl;
	evndata data = RetrieveEventHandler (m_state, event->code ());
	if (data.first != 0)
		(*data.first) (event, data.second);
	else
	{
		data = RetrieveEventHandler (AnyState, event->code ());
		if (data.first != 0)
			(*data.first) (event, data.second);
		else
		{
			cout << "EVENT NOT HANDLED " << event->code () << endl;
			return -1;
		}
	}

	return 0;
}

int MpxTaskBase::Send (MpxTaskBase* task, MpxEventBase* event, bool invoke)
{
	++g_sentCount;

	if (task == 0)
		return -1;

	if ((!g_enableSend) && (event->code () != StopEvent))
		return -1;

	MpxProxyTaskBase* proxyTask = dynamic_cast <MpxProxyTaskBase*> (task);

	if ((proxyTask == 0) || ((int) event->code () < 0))
	{
		if (event->src () == 0)
			event->src (this);
		if (event->dst () == 0)
			event->dst (task);

		MpxTaskMultiplexer* src = (MpxTaskMultiplexer*) m_mpx;
		MpxTaskMultiplexer* dst = (MpxTaskMultiplexer*) task->mpx ();

		if ((src == dst) && (invoke == true))
		{
			if (false)
				cout << "INVOKE : " << event->code () << endl;
			if (event->Invoke () == 0)
				delete event;
			return 0;
		}

		int status = src->Send (dst, event);
		if (status < 0)
			delete event;
		return status;
	}
	else
	{
		if (false)
			cout << "send to proxy " << proxyTask << ", event " << event->code () << endl;
		return proxyTask->SendProxy (task, event, invoke);
	}
}

struct timespec MpxTaskBase::GetCurrentTime ()
{
	return ((MpxTaskMultiplexer*) m_mpx)->GetCurrentTime ();
}

void* MpxTaskBase::StartTimer (struct timespec timerStamp)
{
	return ((MpxTaskMultiplexer*) m_mpx)->StartTimer (this, timerStamp);
}

void MpxTaskBase::StopTimer (void* timer)
{
	((MpxTaskMultiplexer*) m_mpx)->StopTimer (timer);
}

int MpxTaskBase::RetrieveExternalTask (const char* connString)
{
	if (connString == 0)
		return -1;
	if (strncasecmp (connString, g_protocolField, strlen (g_protocolField)) != 0)
		return -1;

	connString += strlen (g_protocolField);
	if (strncasecmp (connString, g_protocolLocal, strlen (g_protocolLocal)) == 0)
	{
		connString += strlen (g_protocolLocal);
		return RetrieveExternalTaskLocal (connString);
	}
	else if (strncasecmp (connString, g_protocolTcp4, strlen (g_protocolTcp4)) == 0)
	{
		connString += strlen (g_protocolTcp4);
		return RetrieveExternalTaskTcp4 (connString);
	}
	else if (strncasecmp (connString, g_protocolTcp6, strlen (g_protocolTcp6)) == 0)
	{
		connString += strlen (g_protocolTcp6);
		return RetrieveExternalTaskTcp6 (connString);
	}

	return -1;
}

int MpxTaskBase::RetrieveExternalTaskLocal (const char* connString)
{
	size_t size;

	if (strncasecmp (connString, g_pathField, strlen (g_pathField)) != 0)
		return -1;

	connString += strlen (g_pathField);
	const char* path = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
		return -1;

	connString++;
	char* mpath = (char*) alloca(size = connString - path);
	strncpy (mpath, path, size);
	mpath [size - 1] = 0;

	if (strncasecmp (connString, g_nameField, strlen (g_nameField)) != 0)
		return -1;

	connString += strlen (g_nameField);
	const char* name = connString;
	connString = strchr (connString, ';');
	if (connString == 0)
		return -1;

	connString++;
	char* mname = (char*) alloca(size = connString - name);
	strncpy (mname, name, size);
	mname [size - 1] = 0;

	return Send (((MpxTaskMultiplexer*) mpx ())->externalTask (), new MpxLocalTaskQueryEvent (mpath, mname), true);
}

int MpxTaskBase::RetrieveExternalTaskTcp4 (const char* connString)
{
	size_t size;

	if (strncasecmp (connString, g_hostnameField, strlen (g_hostnameField)) != 0)
		return -1;

	connString += strlen (g_hostnameField);
	const char* hostname = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
		return -1;

	connString++;
	char* mhostname = (char*) alloca(size = connString - hostname);
	strncpy (mhostname, hostname, size);
	mhostname [size - 1] = 0;

	if (strncasecmp (connString, g_portField, strlen (g_portField)) != 0)
		return -1;

	connString += strlen (g_portField);
	const char* port = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
		return -1;

	connString++;
	char* mport = (char*) alloca(size = connString - port);
	strncpy (mport, port, size);
	mport [size - 1] = 0;

	if (strncasecmp (connString, g_nameField, strlen (g_nameField)) != 0)
		return -1;

	connString += strlen (g_nameField);
	const char* name = connString;
	connString = strchr (connString, ';');
	if (connString == 0)
		return -1;

	connString++;
	char* mname = (char*) alloca(size = connString - name);
	strncpy (mname, name, size);
	mname [size - 1] = 0;

	return Send (((MpxTaskMultiplexer*) mpx ())->externalTask (), new MpxTcp4TaskQueryEvent (mhostname, mport, mname),
		true);
}

int MpxTaskBase::RetrieveExternalTaskTcp6 (const char* connString)
{
	size_t size;

	if (strncasecmp (connString, g_hostnameField, strlen (g_hostnameField)) != 0)
		return -1;

	connString += strlen (g_hostnameField);
	const char* hostname = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
		return -1;

	connString++;
	char* mhostname = (char*) alloca(size = connString - hostname);
	strncpy (mhostname, hostname, size);
	mhostname [size - 1] = 0;

	if (strncasecmp (connString, g_portField, strlen (g_portField)) != 0)
		return -1;

	connString += strlen (g_portField);
	const char* port = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
		return -1;

	connString++;
	char* mport = (char*) alloca(size = connString - port);
	strncpy (mport, port, size);
	mport [size - 1] = 0;

	if (strncasecmp (connString, g_nameField, strlen (g_nameField)) != 0)
		return -1;

	connString += strlen (g_nameField);
	const char* name = connString;
	connString = strchr (connString, ';');
	if (connString == 0)
		return -1;

	connString++;
	char* mname = (char*) alloca(size = connString - name);
	strncpy (mname, name, size);
	mname [size - 1] = 0;

	return Send (((MpxTaskMultiplexer*) mpx ())->externalTask (), new MpxTcp6TaskQueryEvent (mhostname, mport, mname),
		true);
}

} /* namespace mpx */
