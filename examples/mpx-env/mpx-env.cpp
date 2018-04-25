/*
 * mpx-env.cpp
 *
 *  Created on: 6. nov. 2017
 *      Author: miran
 */

#include <execinfo.h>
#include <signal.h>
#include <errno.h>
#include <mpx-core/MpxEnvironment.h>
#include <mpx-tasks/MpxLocalClientProxyTask.h>
#include <mpx-event-queues/MpxLocalMQTask.h>
#include <mpx-event-queues/MpxPosixMQTask.h>
#include <mpx-events/MpxEvents.h>
#include <mpx-jobs/MpxJobGetAddrInfo.h>
#include <mpx-sockets/MpxTcp4EndPoint.h>
#include <mpx-sockets/MpxTcp4Listener.h>
#include <mpx-sockets/MpxTcp6EndPoint.h>
#include <mpx-sockets/MpxTcp6Listener.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-tasks/MpxTaskBase.h>

using namespace mpx;

#include <map>
using namespace std;

static int jcnt = 0;
static int evncnt = 0;
static int endcnt = 0;

static const unsigned int ConsumerEvent = 1;
static const unsigned int ConsumerState = 1;

class MpxConsumerEvent: public MpxEventBase
{
private:
	MpxConsumerEvent () :
		MpxEventBase (ConsumerEvent)
	{
	}
public:
	MpxConsumerEvent (const char* str) :
		MpxEventBase (ConsumerEvent), m_string (str)
	{
	}
	virtual ~MpxConsumerEvent ()
	{
	}
	virtual const char* Name ()
	{
		return "Consumer Event";
	}
	virtual MpxEventBase* Copy ()
	{
		return new MpxConsumerEvent (*this);
	}

	static inline MpxEventBase* ObjAllocator ()
	{
		return new MpxConsumerEvent ();
	}
	static inline void* XdrAllocator ()
	{
		return (void*) new int;
	}

	static const int g_xdrId = ConsumerEvent;
	inline const char* str ()
	{
		return m_string.c_str ();
	}
private:
	string m_string;
};

class TaskCtl: public MpxTaskBase
{
public:
	TaskCtl (int timeOut, const char* name = 0) :
		MpxTaskBase (g_evnset, name), m_timeOut (timeOut)
	{
	}
	~TaskCtl ()
	{
	}
private:
	virtual void StartTask ()
	{
		timespec t = GetCurrentTime ();
		t.tv_sec += m_timeOut;
		StartTimer (t);
	}
	virtual void StopTask ()
	{
	}

	mpx_event_handler (TimerEventHandler, TaskCtl)
private:
	static EventDescriptor g_evntab [];
	static evnset g_evnset;
	int m_timeOut;
};

EventDescriptor TaskCtl::g_evntab [] =
{
{ AnyState, MpxTimerEvent::EventCode, TaskCtl::TimerEventHandler },
{ 0, 0, 0 } };

MpxTaskBase::evnset TaskCtl::g_evnset = MpxTaskBase::CreateEventSet (TaskCtl::g_evntab);

void TaskCtl::TimerEventHandler (MpxEventBase* event)
{
	kill (getpid (), SIGINT);
}

class TaskABC: public MpxTaskBase
{
	typedef set <MpxTcp4EndPoint*> epset;
	typedef set <MpxTcp6EndPoint*> ep6set;
public:
	TaskABC (const char* name = 0) :
		MpxTaskBase (g_evnset, name), m_consumer (0), m_timer (0), m_listener (0), m_6listener (0)
	{
	}
	~TaskABC ()
	{
		Release ();
	}
	virtual void StartTask ();
	virtual void StopTask ();
	void Release ()
	{
		if (m_timer != 0)
			StopTimer (m_timer);
		m_timer = 0;
		if (m_listener != 0)
			delete m_listener;
		m_listener = 0;
		if (m_6listener != 0)
			delete m_6listener;
		m_6listener = 0;
		for (epset::iterator it = m_epset.begin (); it != m_epset.end (); ++it)
			delete *it;
		m_epset.clear ();
		for (ep6set::iterator it = m_ep6set.begin (); it != m_ep6set.end (); ++it)
			delete *it;
		m_ep6set.clear ();
	}
	inline void consumer (MpxTaskBase* consumer)
	{
		m_consumer = consumer;
	}

	mpx_event_handler (TimerEventHandler, TaskABC)
	mpx_event_handler (ConsumerEventHandler, TaskABC)
	mpx_event_handler (Tcp4ListenerEventHandler, TaskABC)
	mpx_event_handler (Tcp4EndPointEventHandler, TaskABC)
	mpx_event_handler (Tcp6ListenerEventHandler, TaskABC)
	mpx_event_handler (Tcp6EndPointEventHandler, TaskABC)
	mpx_event_handler (JobFinishedEventHandler, TaskABC)
	mpx_event_handler (ExternalTaskEventHandler, TaskABC)
	mpx_event_handler (LocalClientEventHandler, TaskABC)

private:
	static EventDescriptor g_evntab [];
	static evnset g_evnset;
	MpxTaskBase* m_consumer;
	void* m_timer;
	MpxTcp4Listener* m_listener;
	MpxTcp6Listener* m_6listener;
	epset m_epset;
	ep6set m_ep6set;
};

EventDescriptor TaskABC::g_evntab [] =
{
{ AnyState, MpxTimerEvent::EventCode, TaskABC::TimerEventHandler },
{ AnyState, MpxTcp4ListenerEvent::EventCode, TaskABC::Tcp4ListenerEventHandler },
{ AnyState, MpxTcp4EndPointEvent::EventCode, TaskABC::Tcp4EndPointEventHandler },
{ AnyState, MpxTcp6ListenerEvent::EventCode, TaskABC::Tcp6ListenerEventHandler },
{ AnyState, MpxTcp6EndPointEvent::EventCode, TaskABC::Tcp6EndPointEventHandler },
{ AnyState, ConsumerEvent, TaskABC::ConsumerEventHandler },
{ ConsumerState, ConsumerEvent, TaskABC::ConsumerEventHandler },
{ AnyState, MpxJobFinishedEvent::EventCode, TaskABC::JobFinishedEventHandler },
{ AnyState, MpxExternalTaskEvent::EventCode, TaskABC::ExternalTaskEventHandler },
{ AnyState, MpxLocalClientEvent::EventCode, TaskABC::LocalClientEventHandler },
{ 0, 0, 0 } };

MpxTaskBase::evnset TaskABC::g_evnset = MpxTaskBase::CreateEventSet (TaskABC::g_evntab);

void TaskABC::StartTask ()
{
	static int ind = 10;

	if (false)
		cout << "START TASK = " << this << ", STATE = " << state () << endl;
	++evncnt;
	if (false)
	{
		if ((rand () % 2) != 0)
		{
			timespec t = GetCurrentTime ();
			t.tv_nsec += (rand () % 1000) * 1000 * 1000;
			t.tv_sec += t.tv_nsec / SEC_TO_NSEC;
			t.tv_nsec %= SEC_TO_NSEC;
			if (m_timer != 0)
				StopTimer (m_timer);
			m_timer = StartTimer (t);
		}
		else
			;
	}
	if (ind-- > 0)
	{
//		RetrieveExternalTask ("protocol:local,path:mpx-ext-link,name:ext-task;");
//		RetrieveExternalTask ("protocol:tcp4,hostname:192.168.1.65,port:22222,name:ext-task;");
//		RetrieveExternalTask ("protocol:tcp6,hostname:0::ffff:c0a8:140,port:22222,name:ext-task;");
//		RetrieveExternalTask ("protocol:tcp6,hostname:fe80::390:8029:2016:8f23/64,port:22222,name:ext-task;");
		RetrieveExternalTask ("protocol:tcp4,hostname:172.30.19.23,port:22222,name:ext-task;");
	}
	MpxWorkingQueue::Put (new MpxJobGetAddrInfo (this, "www.google.com", "", 0));
	Send (m_consumer, new MpxConsumerEvent (""));
	state ((MpxTaskState) ConsumerState);
	m_listener = new MpxTcp4Listener (this, false);
	m_listener->CreateListener (12345);
	m_listener->StartListener ();
	m_6listener = new MpxTcp6Listener (this, false);
	m_6listener->CreateListener (12345);
	m_6listener->StartListener ();
}

void TaskABC::Tcp4ListenerEventHandler (MpxEventBase* event)
{
	cout << "NUMBER OF CLIENTS = " << ++endcnt << endl;
	MpxTcp4ListenerEvent* tcp4 = (MpxTcp4ListenerEvent*) event;
	m_epset.insert (new MpxTcp4EndPoint (this, tcp4->fd ()));
}

void TaskABC::Tcp4EndPointEventHandler (MpxEventBase* event)
{
	MpxTcp4EndPointEvent* ep4 = (MpxTcp4EndPointEvent*) event;
	MpxTcp4EndPoint* ep = (MpxTcp4EndPoint*) ep4->endPoint ();

	switch (ep4->flags ())
	{
	case 0:
		cout << "TIMEOUT FAILED" << endl;
		break;
	case EPOLLIN:
		if ((ep4->error () == 0) && (ep4->size () != 0))
		{
			size_t size = ep4->size ();
			u_char* data = ep->fast () ? ep->FastRead (size) : ep4->buffer ();
			if (false)
				cout << size << ": " << data << endl;
			if (ep->Write (data, size) < 0)
			{
				cout << "WRITE FAILED" << endl;
				break;
			}
			return;
		}
		cout << "SEND FAILED, error = " << ep4->error () << endl;
		break;
	case EPOLLOUT:
		if ((ep4->error () == 0) && (ep4->size () != 0))
		{
//			cout << "SEND: " << ep4->size() << endl;
			return;
		}
		cout << "RECV FAILED, error = " << ep4->error () << endl;
		break;
	default:
		cout << "DEFAULT FAILED, error = " << ep4->error () << endl;
		break;
	}

	cout << "SENT = " << ep->sent () << ", RCVD = " << ep->rcvd () << endl;
	cout << "READ LOST = " << ep->readlost () << ", WRITE LOST = " << ep->writelost () << endl;
	cout << "NUMBER OF CLIENTS = " << --endcnt << endl;
	epset::iterator it = m_epset.find (ep);
	if (it != m_epset.end ())
		m_epset.erase (it);
	delete ep;
}

void TaskABC::Tcp6ListenerEventHandler (MpxEventBase* event)
{
	cout << "NUMBER OF CLIENTS = " << ++endcnt << endl;
	MpxTcp6ListenerEvent* tcp6 = (MpxTcp6ListenerEvent*) event;
	m_ep6set.insert (new MpxTcp6EndPoint (this, tcp6->fd ()));
}

void TaskABC::Tcp6EndPointEventHandler (MpxEventBase* event)
{
	MpxTcp6EndPointEvent* ep6 = (MpxTcp6EndPointEvent*) event;
	MpxTcp6EndPoint* ep = (MpxTcp6EndPoint*) ep6->endPoint ();

	switch (ep6->flags ())
	{
	case 0:
		cout << "TIMEOUT FAILED" << endl;
		break;
	case EPOLLIN:
		if ((ep6->error () == 0) && (ep6->size () != 0))
		{
			size_t size = ep6->size ();
			u_char* data = ep->fast () ? ep->FastRead (size) : ep6->buffer ();
			if (false)
				cout << size << ": " << data << endl;
			if (ep->Write (data, size) < 0)
			{
				cout << "WRITE FAILED" << endl;
				break;
			}
			return;
		}
		cout << "SEND FAILED, error = " << ep6->error () << endl;
		break;
	case EPOLLOUT:
		if ((ep6->error () == 0) && (ep6->size () != 0))
		{
//			cout << "SEND: " << ep6->size() << endl;
			return;
		}
		cout << "RECV FAILED, error = " << ep6->error () << endl;
		break;
	default:
		cout << "DEFAULT FAILED, error = " << ep6->error () << endl;
		break;
	}

	cout << "SENT = " << ep->sent () << ", RCVD = " << ep->rcvd () << endl;
	cout << "READ LOST = " << ep->readlost () << ", WRITE LOST = " << ep->writelost () << endl;
	cout << "NUMBER OF CLIENTS = " << --endcnt << endl;
	ep6set::iterator it = m_ep6set.find (ep);
	if (it != m_ep6set.end ())
		m_ep6set.erase (it);
	delete ep;
}

void TaskABC::TimerEventHandler (MpxEventBase* event)
{
	m_timer = 0;
	if (false)
		if (state () == StartState)
		{
			cout << "ILLEGAL EVENT (" << event->code () << ") IN START STATE" << endl;
			return;
		}
	if (false)
		cout << "TIMER TASK = " << this << ", STATE = " << state () << ", EVENT = " << event->code () << endl;
	++evncnt;
	if ((rand () % 2) != 0)
	{
		MpxTimerEvent* timer = (MpxTimerEvent*) event;
		timespec t = timer->timerStamp ();
		t.tv_nsec += (rand () % 1000) * 1000 * 1000;
		t.tv_sec += t.tv_nsec / SEC_TO_NSEC;
		t.tv_nsec %= SEC_TO_NSEC;
//		m_timer = StartTimer (t);
	}
	else
		;
	Send (m_consumer, new MpxConsumerEvent (""));
	state ((MpxTaskState) ConsumerState);
}

void TaskABC::ConsumerEventHandler (MpxEventBase* event)
{
	if (false)
		if (state () == StartState)
		{
			cout << "ILLEGAL EVENT (" << event->code () << ") IN START STATE" << endl;
			return;
		}
	if (false)
		cout << "CONSUMER TASK = " << this << ", STATE = " << state () << ", EVENT = " << event->code () << endl;
	++evncnt;
	Send (m_consumer, new MpxConsumerEvent (""));
//	cout << ((MpxConsumerEvent*)event)->str() << endl;
//	Send ((MpxTaskBase*) event->src (), new MpxConsumerEvent ("OK"));
	state ((MpxTaskState) ConsumerState);
}

void TaskABC::StopTask ()
{
//	cout << "STOP TASK = " << this << ", STATE = " << state() << ", EVENT = " << event->code() << endl;
	Release ();
	++evncnt;
}

void TaskABC::JobFinishedEventHandler (MpxEventBase* event)
{
//	cout << ++jcnt << ": JOB FINISHED TASK = " << this << ", STATE = " << state() << ", EVENT = " << event->code() << endl;
	MpxJobFinishedEvent* jobFinishedEvent = (MpxJobFinishedEvent*) event;
	MpxJobGetAddrInfo* job = dynamic_cast <MpxJobGetAddrInfo*> (jobFinishedEvent->job ());
	delete job;
	MpxWorkingQueue::Put (new MpxJobGetAddrInfo (this, "www.google.com", "", 0));
	++jcnt;
	++evncnt;
}

void TaskABC::ExternalTaskEventHandler (MpxEventBase* event)
{
	MpxExternalTaskEvent* externalTaskEvent = dynamic_cast <MpxExternalTaskEvent*> (event);
	if (externalTaskEvent == 0)
		return;

	if (externalTaskEvent->flags () != EPOLLIN)
		return;

	if (false)
	{
		cout << "stack trace:" << endl;

		void* addrlist [64];
		int addrlen = backtrace (addrlist, sizeof(addrlist) / sizeof(void*));

		if (addrlen == 0)
			return;

		char** symbollist = backtrace_symbols (addrlist, addrlen);

		for (int i = 0; i < addrlen; i++)
			cout << symbollist [i] << endl;

		free (symbollist);
	}

	MpxLocalClientProxyTask* proxyTask = dynamic_cast <MpxLocalClientProxyTask*> ((MpxTaskBase*) event->src ());
	if (proxyTask != 0)
		if (false)
			cout << "proxy task id = " << proxyTask << endl;
	Send (proxyTask, new MpxConsumerEvent ("hello"), false);
}

void TaskABC::LocalClientEventHandler (MpxEventBase* event)
{
	MpxLocalClientEvent* localClientEvent = dynamic_cast <MpxLocalClientEvent*> (event);
}

static void sigint (int signo)
{
	signal (SIGINT, SIG_DFL);
}

int main (int n, char*p [])
{
	signal (SIGINT, sigint);

	srand (time (0));
	int cnt = 0;
	char type = 'l';

	if (n > 3)
		n = 3;

	switch (n)
	{
	case 3:
		type = *p [2];
		// break;
	case 2:
		cnt = atoi (p [1]);
	}

	MpxTaskBase* consumer = 0;
	TaskABC* firstTask = 0;

	MpxMQTaskI* mqTask;

	switch (type)
	{
	case 'l':
		mqTask = new MpxLocalMQTask ();
		break;
	case 'p':
		mqTask = new MpxPosixMQTask ();
		break;
	default:
		mqTask = new MpxLocalMQTask ();
		break;
	}

	if (true)
	{
		MpxTaskMultiplexer* mpx = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-ext-link;");
		TaskABC* t = new TaskABC ("ext-task");
		mpx->RegisterTask (t);
		TaskCtl* ctl = new TaskCtl (cnt);
		mpx->RegisterTask (ctl);

		MpxTaskMultiplexer* mpx1 = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-test-1;"
			"protocol:tcp4,address:all,port:22221;"
			"protocol:tcp6,address:all,port:22221;"
			"protocol:local,path:mpx-test-22221;");
		MpxTaskMultiplexer* mpx2 = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-test-2;"
			"protocol:tcp4,address:all,port:22222;"
			"protocol:tcp6,address:all,port:22222;"
			"protocol:local,path:mpx-test-22222;");
		MpxTaskMultiplexer* mpx3 = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-test-3;"
			"protocol:tcp4,address:all,port:22223;"
			"protocol:tcp6,address:all,port:22223;"
			"protocol:local,path:mpx-test-22223;");
		MpxTaskMultiplexer* mpx4 = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-test-4;"
			"protocol:tcp4,address:all,port:22224;"
			"protocol:tcp6,address:all,port:22224;"
			"protocol:local,path:mpx-test-22224;");
		MpxTaskMultiplexer* mpx5 = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-test-5;"
			"protocol:tcp4,address:all,port:22225;"
			"protocol:tcp6,address:all,port:22225;"
			"protocol:local,path:mpx-test-22225;");
		MpxTaskMultiplexer* mpx6 = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-test-6;"
			"protocol:tcp4,address:all,port:22226;"
			"protocol:tcp6,address:all,port:22226;"
			"protocol:local,path:mpx-test-22226;");
		MpxTaskMultiplexer* mpx7 = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-test-7;"
			"protocol:tcp4,address:all,port:22227;"
			"protocol:tcp6,address:all,port:22227;"
			"protocol:local,path:mpx-test-22227;");
		MpxTaskMultiplexer* mpx8 = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-test-8;"
			"protocol:tcp4,address:all,port:22228;"
			"protocol:tcp6,address:all,port:22228;"
			"protocol:local,path:mpx-test-22228;");
		MpxTaskMultiplexer* mpx9 = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-test-9;"
			"protocol:tcp4,address:all,port:22229;"
			"protocol:tcp6,address:all,port:22229;"
			"protocol:local,path:mpx-test-22229;");

		// MpxTaskMultiplexer* mpx1 = MpxEnvironment::CreateTaskMultiplexer ();
		// MpxTaskMultiplexer* mpx2 = MpxEnvironment::CreateTaskMultiplexer ();
		// MpxTaskMultiplexer* mpx3 = MpxEnvironment::CreateTaskMultiplexer ();
		// MpxTaskMultiplexer* mpx4 = MpxEnvironment::CreateTaskMultiplexer ();
		// MpxTaskMultiplexer* mpx5 = MpxEnvironment::CreateTaskMultiplexer ();
		// MpxTaskMultiplexer* mpx6 = MpxEnvironment::CreateTaskMultiplexer ();
		// MpxTaskMultiplexer* mpx7 = MpxEnvironment::CreateTaskMultiplexer ();
		// MpxTaskMultiplexer* mpx8 = MpxEnvironment::CreateTaskMultiplexer ();
		// MpxTaskMultiplexer* mpx9 = MpxEnvironment::CreateTaskMultiplexer ();

		for (int i = 0; i < 100; ++i)
		{
			char name [256];
			sprintf (name, "ext-task-%3d", 9 * i + 1);
			TaskABC* t1 = new TaskABC (name);
			sprintf (name, "ext-task-%3d", 9 * i + 2);
			TaskABC* t2 = new TaskABC (name);
			sprintf (name, "ext-task-%3d", 9 * i + 3);
			TaskABC* t3 = new TaskABC (name);
			sprintf (name, "ext-task-%3d", 9 * i + 4);
			TaskABC* t4 = new TaskABC (name);
			sprintf (name, "ext-task-%3d", 9 * i + 5);
			TaskABC* t5 = new TaskABC (name);
			sprintf (name, "ext-task-%3d", 9 * i + 6);
			TaskABC* t6 = new TaskABC (name);
			sprintf (name, "ext-task-%3d", 9 * i + 7);
			TaskABC* t7 = new TaskABC (name);
			sprintf (name, "ext-task-%3d", 9 * i + 8);
			TaskABC* t8 = new TaskABC (name);
			sprintf (name, "ext-task-%3d", 9 * i + 9);
			TaskABC* t9 = new TaskABC (name);

			if (firstTask == 0)
				firstTask = t1;

			mpx1->RegisterTask (t1);
			mpx2->RegisterTask (t2);
			mpx3->RegisterTask (t3);
			mpx4->RegisterTask (t4);
			mpx5->RegisterTask (t5);
			mpx6->RegisterTask (t6);
			mpx7->RegisterTask (t7);
			mpx8->RegisterTask (t8);
			mpx9->RegisterTask (t9);

			t1->consumer (consumer);
			t2->consumer (t1);
			t3->consumer (t2);
			t4->consumer (t3);
			t5->consumer (t4);
			t6->consumer (t5);
			t7->consumer (t6);
			t8->consumer (t7);
			t9->consumer (t8);

			consumer = t9;
		}
	}
	else
	{
		MpxTaskMultiplexer* mpx = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-ext-link;");
		TaskABC* t = new TaskABC ("ext-task");
		firstTask = t;
		mpx->RegisterTask (t);
		t->consumer (consumer);
		consumer = t;
		TaskCtl* ctl = new TaskCtl (cnt);
		mpx->RegisterTask (ctl);
	}

	if (true)
		firstTask->consumer (consumer);

	MpxEnvironment::Start (mqTask);
	pause ();
	cout << "STOP" << endl;
	MpxEnvironment::Stop ();

	delete mqTask;

	cout << "number of events = " << evncnt << ", " << jcnt << ", " << MpxTaskBase::sentCount () << endl;
	return 0;
}
