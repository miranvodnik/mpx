/*
 * mpx-client.cpp
 *
 *  Created on: 17. nov. 2017
 *      Author: miran
 */

#include "mpx-core/MpxEnvironment.h"
#include "mpx-events/MpxEventBase.h"
#include "mpx-tasks/MpxTaskBase.h"
#include "mpx-events/MpxStartEvent.h"
#include "mpx-sockets/MpxTcp4Client.h"
#include "mpx-events/MpxTcp4ClientEvent.h"
#include "mpx-sockets/MpxTcp6Client.h"
#include "mpx-events/MpxTcp6ClientEvent.h"
#include "mpx-event-queues/MpxLocalMQTask.h"
#include "mpx-event-queues/MpxPosixMQTask.h"
using namespace mpx;

#include <map>
using namespace std;

static pthread_t mainThread = 0;
static int killed = 0;

typedef set <MpxTcp4Client*> clnset;
typedef set <MpxTcp6Client*> cln6set;

class Tcp4ClientTask: public MpxTaskBase
{
public:
	Tcp4ClientTask (const char* name = 0) :
		MpxTaskBase (name)
	{
		m_count = 0;
		RegisterEventHandler (AnyState, MpxTcp4ClientEvent::EventCode, Tcp4ClientEventHandler);
	}
	~Tcp4ClientTask ()
	{
		Release ();
	}
	void Release ()
	{
		cout << "RELEASE TASK" << endl;
		clnset::iterator it;
		for (it = m_clnset.begin (); it != m_clnset.end (); ++it)
			delete *it;
		m_clnset.clear ();
		{
			killed = 1;
			pthread_kill (mainThread, SIGINT);
		}
	}
	void StartTask ();
	void StopTask ();

	;mpx_event_handler (Tcp4ClientEventHandler, Tcp4ClientTask)
	;

private:
	int m_count;
	clnset m_clnset;
};

class Tcp6ClientTask: public MpxTaskBase
{
public:
	Tcp6ClientTask (const char* name = 0) :
		MpxTaskBase (name)
	{
		m_count = 0;
		RegisterEventHandler (AnyState, MpxTcp6ClientEvent::EventCode, Tcp6ClientEventHandler);
	}
	~Tcp6ClientTask ()
	{
		Release ();
	}
	void Release ()
	{
		cout << "RELEASE TASK" << endl;
		cln6set::iterator it;
		for (it = m_clnset.begin (); it != m_clnset.end (); ++it)
			delete *it;
		m_clnset.clear ();
		{
			killed = 2;
			pthread_kill (mainThread, SIGINT);
		}
	}
	void StartTask ();
	void StopTask ();

	;mpx_event_handler (Tcp6ClientEventHandler, Tcp6ClientTask)
	;

private:
	int m_count;
	cln6set m_clnset;
};

void Tcp4ClientTask::StartTask ()
{
	m_count = 0;
	for (int i = 0; i < 100; ++i)
	{
		MpxTcp4Client* client;
		client = new MpxTcp4Client (this, true, 1000L * 1000L * 1000L, false);
		if (client->Connect ("localhost", 12345) < 0)
		{
			this->Dispose (true);
			{
				killed = 3;
				pthread_kill (mainThread, SIGINT);
			}
			return;
		}
		if (client->Write ((u_char*) "TEST MESSAGE", 13) < 0)
		{
			cout << "WRITE FAILED" << endl;
			this->Dispose (true);
			{
				killed = 4;
				pthread_kill (mainThread, SIGINT);
			}
			return;
		}
		m_clnset.insert (client);
		++m_count;
	}
}

void Tcp4ClientTask::Tcp4ClientEventHandler (MpxEventBase* event)
{
	MpxTcp4ClientEvent* ep4 = (MpxTcp4ClientEvent*) event;
	MpxTcp4Client* ep = (MpxTcp4Client*) ep4->endPoint ();

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
			if (m_count > 10 * 100)
			{
				ep->Shutdown ();
				return;
			}
			if (ep->Write ((u_char*) "TEST MESSAGE", 13) < 0)
			{
				cout << "WRITE FAILED" << endl;
				ep->Shutdown ();
				this->Dispose (true);
				return;
			}
			m_count += 1;
//			cout << size << ": " << data << endl;
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

	clnset::iterator it = m_clnset.find (ep);
	if (it != m_clnset.end ())
		m_clnset.erase (it);
	delete ep;
	if (m_clnset.empty ())
	{
		killed = 5;
		pthread_kill (mainThread, SIGINT);
	}
}

void Tcp4ClientTask::StopTask ()
{
	cout << "STOP" << endl;
	Release ();
}

void Tcp6ClientTask::StartTask ()
{
	m_count = 0;
	for (int i = 0; i < 100; ++i)
	{
		MpxTcp6Client* client;
		client = new MpxTcp6Client (this, true, 1000L * 1000L * 1000L, false);
		if (client->Connect ("localhost", 12345) < 0)
		{
			this->Dispose (true);
			{
				killed = 6;
				pthread_kill (mainThread, SIGINT);
			}
			return;
		}
		if (client->Write ((u_char*) "TEST MESSAGE", 13) < 0)
		{
			cout << "WRITE FAILED" << endl;
			this->Dispose (true);
			{
				killed = 7;
				pthread_kill (mainThread, SIGINT);
			}
			return;
		}
		m_clnset.insert (client);
		++m_count;
	}
}

void Tcp6ClientTask::Tcp6ClientEventHandler (MpxEventBase* event)
{
	MpxTcp6ClientEvent* ep6 = (MpxTcp6ClientEvent*) event;
	MpxTcp6Client* ep = (MpxTcp6Client*) ep6->endPoint ();

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
			if (m_count > 10 * 100)
			{
				ep->Shutdown ();
				return;
			}
			if (ep->Write ((u_char*) "TEST MESSAGE", 13) < 0)
			{
				cout << "WRITE FAILED" << endl;
				ep->Shutdown ();
				this->Dispose (true);
				return;
			}
			m_count += 1;
//			cout << size << ": " << data << endl;
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

	cln6set::iterator it = m_clnset.find (ep);
	if (it != m_clnset.end ())
		m_clnset.erase (it);
	delete ep;
	if (m_clnset.empty ())
	{
		killed = 8;
		pthread_kill (mainThread, SIGINT);
	}
}

void Tcp6ClientTask::StopTask ()
{
	cout << "STOP" << endl;
	Release ();
}

void sigint (int)
{

}

int main (int n, char*p [])
{
	mainThread = pthread_self ();
	signal (SIGINT, sigint);

	char type = 'l';

	if (n > 2)
		n = 2;

	switch (n)
	{
	case 2:
		type = *p [1];
	}

	MpxMQTaskI* localMQ;
	switch (type)
	{
	case 'l':
		localMQ = new MpxLocalMQTask ();
		break;
	case 'p':
		localMQ = new MpxPosixMQTask ();
		break;
	default:
		localMQ = new MpxLocalMQTask ();
		break;
	}

	MpxTaskMultiplexer* mpx = MpxEnvironment::CreateTaskMultiplexer ("protocol:local,path:mpx-ext-link;");
	Tcp4ClientTask* cln = new Tcp4ClientTask ("ext-task");
	mpx->RegisterTask (cln);
	// Tcp6ClientTask* cln6 = new Tcp6ClientTask ("ext6-task");
	// mpx->RegisterTask (cln6);

	MpxEnvironment::Start (localMQ);

	// for (int i = 0; i < 0; ++i)
		// sleep (1);
	while (true)
	{
		sleep (1);
		if (killed > 0)
			break;
	}

	MpxEnvironment::Stop ();
	delete localMQ;

	return 0;
}
