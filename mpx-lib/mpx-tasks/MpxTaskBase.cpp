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
#include <mpx-events/MpxStartEvent.h>
#include <mpx-events/MpxStopEvent.h>
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

EventDescriptor MpxTaskBase::g_evntab [] =
{
	{ StartState, MpxStartEvent::EventCode, StartEventHandler },
	{ AnyState, MpxStopEvent::EventCode, StopEventHandler },
	{ 0, 0, 0 },
};

/** Task instance constructor
 *
 * initializes task: set its name, initial state becomes StartState,
 * start and stop event handlers are registered for the task. This
 * constructor is useful, when some subclass implementing MpxTaskBase
 * want to provide its own event callback dictionary for some or for
 * all instances of this subclass. An example should be globally
 * defined event callback dictionary used for all instances for that
 * subclass. This should be the common practice for all subclasses
 * implementing this interface, since there is rarely need for every
 * task instance to use its own event event callback dictionary, unless
 * they implement so different functionalities that this is necessary
 * to do. But in that case it would be better to define different
 * subclasses implementing different functionalities.
 *
 * @param e - reference to an external event callback dictionary
 * @param name - task name, it should be null
 */
MpxTaskBase::MpxTaskBase (evnset& e, const char* name) : m_evnset (e)
{
	m_extset = true;
	m_mpx = 0;
	m_name = (name != 0) ? name : "";
	m_state = StartState;
}

/*! Task instance constructor
 *
 * This constructor is basically equivalent to the previous one, with
 * an important difference: it will use instance specific event callback
 * dictionary. Every instance will have its own dictionary. As already
 * said, this is rarely (if ever) needed, since the nature of event
 * callback dictionaries is such, that they are constructed at the
 * beginning of system life span once for ever and are the same for all
 * instances of the same subclass. Since they are rarely (if ever) changed
 * it is better practice to use only one instance of this dictionary for
 * all instances of the same subclass.
 *
 * @param name - task name, it should be null
 */
MpxTaskBase::MpxTaskBase (const char* name) : m_evnset (m_locevnset)
{
	m_extset = false;
	m_mpx = 0;
	m_name = (name != 0) ? name : "";
	m_state = StartState;
	RegisterEventHandlers (g_evntab);
}

/** Task destructor
 *
 * empties event handlers set
 *
 */
MpxTaskBase::~MpxTaskBase ()
{
	m_mpx = 0;
	if (m_extset == false)
		m_evnset.clear ();
}

/** Dispose task
 *
 * this routine is used to notify task multiplexer (one which owns
 * this task) about the fact that this task has finished its activity
 * and will be deleted immediately after this announcement
 *
 * @param release
 */
void MpxTaskBase::Dispose (bool release)
{
	(reinterpret_cast <MpxTaskMultiplexer*> (mpx ()))->DisposeTask (this, release);
}

MpxTaskBase::evnset& MpxTaskBase::CreateEventSet (EventDescriptor eventTab[])
{
	evnset* eset = new evnset;

	for (EventDescriptor* e = g_evntab; (e->stateCode != 0) && (e->eventCode != 0) && (e->f != 0); ++e)
	{
		evnkey key (e->stateCode, e->eventCode);
		evnset::iterator it = eset->find (key);
		if (it != eset->end ())
			continue;
		(*eset) [key] = e->f;
	}

	for (EventDescriptor* e = eventTab; (e->stateCode != 0) && (e->eventCode != 0) && (e->f != 0); ++e)
	{
		evnkey key (e->stateCode, e->eventCode);
		evnset::iterator it = eset->find (key);
		if (it != eset->end ())
			continue;
		(*eset) [key] = e->f;
	}
	return *eset;
}

/*! event callback function registration
 *
 * This is one of the most important functions of the MPX system, since it
 * embeds task functionality into the system. This function is used to register
 * one event callback at a time. Although this is not the problem, it is probably
 * better to use function RegisterEventHandlers, which registers many event
 * callback functions at a time. This function registers event callback function
 * for given (stateCode, eventCode) combination. If this combination is already
 * registered, this callback will not be registered
 *
 * @param stateCode - state code
 * @param eventCode - event code
 * @param evncb - function to be invoked when event with event code equal to value
 * of parameter eventCode is delivered to task which state code is equal to the
 * value of parameter stateCode
 * @return callback function reference, either new or previous one in which case
 * new one will not be registered
 */
evnfunc MpxTaskBase::RegisterEventHandler (unsigned int stateCode, unsigned int eventCode, evnfunc evncb)
{
	evnkey key (stateCode, eventCode);
	evnset::iterator it = m_evnset.find (key);
	if (it != m_evnset.end ())
		return it->second;
	return m_evnset [key] = evncb;
}

/*! register any number of event callback functions at a time
 *
 * function iterates throug table of triples (stateCode, eventCode, callback reference)
 * and registeres one at a time using function RegisterEventHandler.
 *
 * @param evntab - table of triples (stateCode, eventCode, callback reference)
 */
void MpxTaskBase::RegisterEventHandlers (EventDescriptor evntab [])
{
	for (EventDescriptor* evnptr = evntab; evnptr->f != 0; ++evnptr)
		RegisterEventHandler (evnptr->stateCode, evnptr->eventCode, evnptr->f);
}

/*! Retrieve event callback for given combination (state, event)
 *
 * @param state - state code
 * @param event - event code
 * @return **callback reference** - reference to callback function registered for
 * (state, event) combination
 * @return **0** - there is no callback function registered for (state, event)
 * combination
 */
evnfunc MpxTaskBase::RetrieveEventHandler (u_int state, u_int event)
{
	evnset::iterator it = m_evnset.find (evnkey (state, event));
	if (it != m_evnset.end ())
		return it->second;
	return 0;
}

/*! Handle an event explicitly
 *
 * It sounds weird, but it is legitimate to call event handler explicitly in the
 * name of given task. Function is heavily used by MPX environment, but it is also
 * possible to use it by application itself. Function should be used very carefully,
 * since it is very dangerous to cross the task multiplexer's boundary. It is in fact
 * very dangerous to perform this function in the name of task which is owned by
 * another task multiplexer, since this multiplexer lives in another thread of
 * execution. Because of that, it is almost always better to send event rather to
 * invoke it explicitly. That is why is better to avoid usage of this function.
 *
 * @param event - event which will be triggered explicitly
 * @return **0* - call succeeded
 * @return **-1* - call failed
 */
int MpxTaskBase::HandleEvent (MpxEventBase* event)
{
	if (false)
		cout << "HANDLE EVENT " << event->code () << endl;
	evnfunc func = RetrieveEventHandler (m_state, event->code ());
	if (func != 0)
		(*func) (event, this);
	else
	{
		func = RetrieveEventHandler (AnyState, event->code ());
		if (func != 0)
			(*func) (event, this);
		else
		{
			cout << "EVENT NOT HANDLED " << event->code () << endl;
			return -1;
		}
	}

	return 0;
}

/*! send event to some task
 *
 * This is another very important function of the MPX environment. If function
 * RegisterEventHandler is used to build the structure of the system, this one
 * is designed to give it dynamic behavior. This function should be freely used
 * by callback functions whenever they decide to inform other participants (tasks)
 * in the system (or across system boundary) about just anything that should be
 * expressed with events.
 *
 * @param task - destination task reference
 * @param event - reference to an object implementing MpxEventBase interface, which
 * will be delivered to destination task. After being triggered, it will be deleted
 * by MPX environment. That is why this reference must always address new instance
 * of this object and must never point to statically or automatically allocated object.
 * @param invoke - indicates if delivery of this event should bypass normal event
 * queuing mechanism. MPX framework will check if it is possible to do that. If it
 * is, event will not be queued and callback function for that event will be invoked
 * as a subroutine of the callback function sending this event. This means that an
 * iteration step (which is synonym for callback invocation) of destination task will
 * be made in the middle of iteration of calling task. This is generally not the
 * problem, but it should lead to infinite or recursive loops, concurrent data
 * access, reversing of order of task execution (at least partially) and causing
 * general confusions in system. Although this possibility is used by MPX
 * environment it is better to avoid it using on application level.
 * @return **0* - event was sent successfully
 * @return **-1* - sending (or invocation) of event failed
 */
int MpxTaskBase::Send (MpxTaskBase* task, MpxEventBase* event, bool invoke)
{
	++g_sentCount;

	if (task == 0)
	{
		cout << "Send() failed, missing destination task" << endl;
		return -1;
	}

	if ((!g_enableSend) && (event->code () != MpxStopEvent::EventCode))
	{
		cout << "Send() failed, send disabled" << endl;
		return -1;
	}

	if (event->src () == 0)
		event->src (this);
	if (event->dst () == 0)
		event->dst (task);

	int status;
	MpxTaskMultiplexer* src = (MpxTaskMultiplexer*) m_mpx;
	MpxTaskMultiplexer* dst = (MpxTaskMultiplexer*) task->mpx ();

	if (src == dst)
	{
		if ((invoke == true) ||
			(dynamic_cast < MpxProxyTaskBase* > (task) != 0) ||
			(dynamic_cast < MpxProxyTaskBase* > (this) != 0))
		{
			if (false)
				cout << "INVOKE : " << event->code () << endl;
			status = event->Invoke ();
			delete event;
			return status;
		}
	}

	if ((status = src->Send (dst, event)) < 0)
		delete event;
	return status;
}

/*! start event callback function
 *
 * This function is a callback function for start event sent by MPX environment
 * at the start of the system. It calls StartTask() function which must be implemented
 * by subclass implementing MpxTaskBase. This is a way to tell the task when it was
 * started.
 *
 * @param event - start event reference
 */
void MpxTaskBase::StartEventHandler (MpxEventBase* event)
{
	StartTask ();
}

/*! stop event callback function
 *
 * This function is a callback function for stop event sent by MPX environment just
 * before the system shutdown. It calls StopTask() function which must be implemented
 * by subclass implementing MpxTaskBase. This is a way to tell the task when it must
 * terminate.
 *
 * @param event
 */
void MpxTaskBase::StopEventHandler (MpxEventBase* event)
{
	StopTask ();
}

/*! get current time
 *
 * Function retrieves time stamp stored in current iteration of object implementing
 * I/O multiplexing for task multiplexer owning this task instance. This time stamp
 * is equal for all tasks which make this function call in the same iteration of
 * object mentioned previously. Using time in this manner is very useful to achieve
 * consistent behavior of periodical activities within system in the sense, that
 * periods of these activities does not shrink or stretch on the long run.
 *
 * @return time stamp, pair (second, nanosecond) of current time
 */
struct timespec MpxTaskBase::GetCurrentTime ()
{
	return (reinterpret_cast <MpxTaskMultiplexer*> (m_mpx))->GetCurrentTime ();
}

/*! start timer
 *
 * Every task should use as many timers as it needs them. Function StartTimer() serves
 * just this purpose. It creates timer within object implementing I/O multiplexing
 * for task multiplexer owning this task. After that timer is started and an opaque
 * reference to this timer is returned to calling function.
 *
 * @param timerStamp - expiration time of timer. When this timer expires, an instance
 * of MpxTimerEvent will be delivered to this task, containing just this time stamp.
 * This time will of course slightly differ from the current time stamp, but if wee want
 * to implement accurate periodical activity, wee should not take in account current
 * time to compute next iteration, but time stamp returned by MpxTimerEvent
 * @param ctx - context information which enable differentiation between different
 * occurances of timers and to provide them with additional data when they expire. This
 * data is also returned by an instance of MpxTimerEvent when timer expires
 *
 * @return an opaque reference to objct representing running timer
 */
void* MpxTaskBase::StartTimer (struct timespec timerStamp, void* ctx)
{
	return (reinterpret_cast <MpxTaskMultiplexer*> (m_mpx))->StartTimer (this, timerStamp, ctx);
}

/*! stop timer
 *
 * this function is used to stop timer which has not yet expired. Once expired, reference
 * to it must not be used any more by any task in the system, since object referenced by
 * it is already deleted by MPX environment as soon as MpxTimerEvent is delivered and
 * consumed by some task. This is is the case also after this function has been executed,
 * since it deletes referenced object by itself.
 *
 * @param timer - reference to an object representing timer to be stopped
 */
void MpxTaskBase::StopTimer (void* timer)
{
	(reinterpret_cast <MpxTaskMultiplexer*> (m_mpx))->StopTimer (timer);
}

/*! retrieve reference of an external task
 *
 * Very important function which enable to extend system functionality between
 * different processes on the same machine and across network. Function just initiates
 * retrieval process and returns immediately after that indicating success of this
 * initiation. Function works by analyzing input parameter containing connection string,
 * which fully describes how to retrieve external task reference. This reference is
 * retrieved with the help server which address of some kind is included in this
 * connection string. Depending on the protocol used in retrieval process there are
 * also additional informations needed to retrieve this reference using specific
 * protocol. Current implementation supports three protocols, which will be described
 * later:
 * - local: this protocol uses unix domain sockets and can be used to locate external
 * tasks on the process on the same machine
 * - tcp4: used to locate external task references through IP network using TCP version
 * 4 protocol.
 * - tcp4: used to locate external task references through IP network using TCP version
 * 6 protocol.
 *
 * @param connString - connection string
 * @return **0* - retrieval was initiated
 * @return **-1* - retrieval was not initiated
 */
int MpxTaskBase::RetrieveExternalTask (const char* connString, const char* encdeclib)
{
	if (connString == 0)
	{
		cout << "RetrieveExternalTask () failed, missing connection string" << endl;
		return -1;
	}
	if (strncasecmp (connString, g_protocolField, strlen (g_protocolField)) != 0)
	{
		cout << "RetrieveExternalTask () failed, cannot find protocol specifier in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString += strlen (g_protocolField);
	if (strncasecmp (connString, g_protocolLocal, strlen (g_protocolLocal)) == 0)
	{
		connString += strlen (g_protocolLocal);
		return RetrieveExternalTaskLocal (connString, encdeclib);
	}
	else if (strncasecmp (connString, g_protocolTcp4, strlen (g_protocolTcp4)) == 0)
	{
		connString += strlen (g_protocolTcp4);
		return RetrieveExternalTaskTcp4 (connString, encdeclib);
	}
	else if (strncasecmp (connString, g_protocolTcp6, strlen (g_protocolTcp6)) == 0)
	{
		connString += strlen (g_protocolTcp6);
		return RetrieveExternalTaskTcp6 (connString, encdeclib);
	}

	cout << "RetrieveExternalTask () failed, unknown protocol specifier in connection string '" << connString << "'" << endl;
	return -1;
}

/*! retrieve reference of external task using local protocol
 *
 * Connection string for local protocol retrieval consists of three parts:
 * - **protocol:local**
 * - **path:<file-name>** <file-name> is name of 'file' located in /var/run and is used
 * to establish connection with local server listening on local 'port' <file-name>. This
 * server is nothing but an instance of 'external task' inhabited in another (or even the
 * same) MPX application on the same machine.
 * - **name:<external-task-name>** name of some task in another (or same) MPX application
 * on the same machine. Example of local connection string is:
 *
 * protocol:local,path:mpx-ext-link,name:ext-task;
 *
 * This connection string means this: on local machine search MPX application listening on
 * local socket 'mpx-ext-link' to find task named 'ext-task'
 *
 * @param connString - connection string of the form protocol:local,path:<path>,name:<name>;
 * @return **0* - retrieval was initiated
 * @return **-1* - retrieval was not initiated
 */
int MpxTaskBase::RetrieveExternalTaskLocal (const char* connString, const char* encdeclib)
{
	size_t size;

	if (strncasecmp (connString, g_pathField, strlen (g_pathField)) != 0)
	{
		cout << "RetrieveExternalTaskLocal () failed, cannot find path specifier in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString += strlen (g_pathField);
	const char* path = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
	{
		cout << "RetrieveExternalTaskLocal () failed, cannot find path delimiter in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString++;
	char* mpath = reinterpret_cast <char*> (alloca(size = connString - path));
	strncpy (mpath, path, size);
	mpath [size - 1] = 0;

	if (strncasecmp (connString, g_nameField, strlen (g_nameField)) != 0)
	{
		cout << "RetrieveExternalTaskLocal () failed, cannot find task name specifier in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString += strlen (g_nameField);
	const char* name = connString;
	connString = strchr (connString, ';');
	if (connString == 0)
	{
		cout << "RetrieveExternalTaskLocal () failed, cannot find task name delimiter in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString++;
	char* mname = reinterpret_cast <char*> (alloca(size = connString - name));
	strncpy (mname, name, size);
	mname [size - 1] = 0;

	return Send ((reinterpret_cast <MpxTaskMultiplexer*> (mpx ()))->externalTask (), new MpxLocalTaskQueryEvent (mpath, mname, encdeclib), true);
}

/*! retrieve reference of external task using tcp4 protocol
 *
 * Connection string for tcp4 protocol retrieval consists of four parts:
 * - **protocol:tcp4**
 * - **hostname:<hostname>** where <hostname> represents name of machine on the IP network
 * which should be searched to find MPX application which contains external task. This host
 * name can also be valid IP4 address.
 * - **port:<port>** where <port> represents listen port of server on remote machine. This
 * server is nothing but an instance of 'external task' inhabited in some MPX application on
 * remote machine
 * - **name:<external-task-name>** name of some task in remote MPX application.
 *
 * Example of tcp4 connection string is:
 *
 * protocol:tcp4,hostname:mpx-rmt-machine,port:12345,name:ext-task;
 *
 * This connection string means this: on remote machine named mpx-rmt-machine search MPX
 * application listening on tcp4 port 12345 to find task named 'ext-task'
 *
 * @param connString - connection string of the form protocol:tcp4,hostname:<hostname>,port:<port>,name:<name>;
 * @return **0* - retrieval was initiated
 * @return **-1* - retrieval was not initiated
 */
int MpxTaskBase::RetrieveExternalTaskTcp4 (const char* connString, const char* encdeclib)
{
	size_t size;

	if (strncasecmp (connString, g_hostnameField, strlen (g_hostnameField)) != 0)
	{
		cout << "RetrieveExternalTaskTcp4 () failed, cannot find hostname specifier in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString += strlen (g_hostnameField);
	const char* hostname = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
	{
		cout << "RetrieveExternalTaskTcp4 () failed, cannot find hostname delimiter in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString++;
	char* mhostname = reinterpret_cast <char*> (alloca(size = connString - hostname));
	strncpy (mhostname, hostname, size);
	mhostname [size - 1] = 0;

	if (strncasecmp (connString, g_portField, strlen (g_portField)) != 0)
	{
		cout << "RetrieveExternalTaskTcp4 () failed, cannot find port specifier in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString += strlen (g_portField);
	const char* port = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
	{
		cout << "RetrieveExternalTaskTcp4 () failed, cannot find port delimiter in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString++;
	char* mport = reinterpret_cast <char*> (alloca(size = connString - port));
	strncpy (mport, port, size);
	mport [size - 1] = 0;

	if (strncasecmp (connString, g_nameField, strlen (g_nameField)) != 0)
	{
		cout << "RetrieveExternalTaskTcp4 () failed, cannot find task name specifier in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString += strlen (g_nameField);
	const char* name = connString;
	connString = strchr (connString, ';');
	if (connString == 0)
	{
		cout << "RetrieveExternalTaskTcp4 () failed, cannot find task name delimiter in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString++;
	char* mname = reinterpret_cast <char*> (alloca(size = connString - name));
	strncpy (mname, name, size);
	mname [size - 1] = 0;

	return Send ((reinterpret_cast <MpxTaskMultiplexer*> (mpx ()))->externalTask (), new MpxTcp4TaskQueryEvent (mhostname, mport, mname, encdeclib),
		true);
}

/*! retrieve reference of external task using tcp6 protocol
 *
 * Connection string for tcp6 protocol retrieval consists of four parts:
 * - **protocol:tcp6**
 * - **hostname:<hostname>** where <hostname> represents name of machine on the IP network
 * which should be searched to find MPX application which contains external task. This host
 * name can also be valid IP6 address.
 * - **port:<port>** where <port> represents listen port of server on remote machine. This
 * server is nothing but an instance of 'external task' inhabited in some MPX application on
 * remote machine
 * - **name:<external-task-name>** name of some task in remote MPX application.
 *
 * Example of tcp6 connection string is:
 *
 * protocol:tcp6,hostname:mpx-rmt-machine,port:12345,name:ext-task;
 *
 * This connection string means this: on remote machine named mpx-rmt-machine search MPX
 * application listening on tcp6 port 12345 to find task named 'ext-task'
 *
 * @param connString - connection string of the form protocol:tcp6,hostname:<hostname>,port:<port>,name:<name>;
 * @return **0* - retrieval was initiated
 * @return **-1* - retrieval was not initiated
 */
int MpxTaskBase::RetrieveExternalTaskTcp6 (const char* connString, const char* encdeclib)
{
	size_t size;

	if (strncasecmp (connString, g_hostnameField, strlen (g_hostnameField)) != 0)
	{
		cout << "RetrieveExternalTaskTcp6 () failed, cannot find hostname specifier in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString += strlen (g_hostnameField);
	const char* hostname = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
	{
		cout << "RetrieveExternalTaskTcp6 () failed, cannot find hostname delimiterin connection string '" << connString << "'" << endl;
		return -1;
	}

	connString++;
	char* mhostname = reinterpret_cast <char*> (alloca(size = connString - hostname));
	strncpy (mhostname, hostname, size);
	mhostname [size - 1] = 0;

	if (strncasecmp (connString, g_portField, strlen (g_portField)) != 0)
	{
		cout << "RetrieveExternalTaskTcp6 () failed, cannot find port specifier in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString += strlen (g_portField);
	const char* port = connString;
	connString = strchr (connString, ',');
	if (connString == 0)
	{
		cout << "RetrieveExternalTaskTcp6 () failed, cannot find port delimiter in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString++;
	char* mport = reinterpret_cast <char*> (alloca(size = connString - port));
	strncpy (mport, port, size);
	mport [size - 1] = 0;

	if (strncasecmp (connString, g_nameField, strlen (g_nameField)) != 0)
	{
		cout << "RetrieveExternalTaskTcp6 () failed, cannot find task name specifier in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString += strlen (g_nameField);
	const char* name = connString;
	connString = strchr (connString, ';');
	if (connString == 0)
	{
		cout << "RetrieveExternalTaskTcp6 () failed, cannot find task name delimiter in connection string '" << connString << "'" << endl;
		return -1;
	}

	connString++;
	char* mname = reinterpret_cast <char*> (alloca(size = connString - name));
	strncpy (mname, name, size);
	mname [size - 1] = 0;

	return Send ((reinterpret_cast <MpxTaskMultiplexer*> (mpx ()))->externalTask (), new MpxTcp6TaskQueryEvent (mhostname, mport, mname, encdeclib),
		true);
}

} /* namespace mpx */
