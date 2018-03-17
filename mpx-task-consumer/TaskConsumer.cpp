//    TODO
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

//
// TaskConsumer.cpp
//
//  Created on: Mar 9, 2018
//      Author: miran
//

#include <TaskConsumer.h>
#include <MpxConsumerEventA.h>
#include <MpxConsumerEventB.h>
using namespace mpx_edlib;

namespace mpx_task_consumer
{

int TaskConsumer::m_acount = 0;
int TaskConsumer::m_bcount = 0;

EventDescriptor TaskConsumer::g_evntab [] =
{
	{ AnyState, MpxExternalTaskEvent::EventCode, ExternalTaskEventHandler },
	{ AnyState, MpxConsumerEventA::EventCode, ConsumerEventAHandler },
	{ AnyState, MpxConsumerEventB::EventCode, ConsumerEventBHandler },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset TaskConsumer::g_evnset = MpxTaskBase::CreateEventSet (g_evntab);

TaskConsumer::TaskConsumer () : MpxTaskBase(g_evnset)
{
	m_provider = 0;
}

TaskConsumer::~TaskConsumer ()
{
}

void TaskConsumer::StartTask ()
{
	RetrieveExternalTask ("protocol:tcp4,hostname:192.168.1.64,port:22220,name:task-provider;", "libmpx-edlib.so");
}

void TaskConsumer::StopTask ()
{

}

void TaskConsumer::ExternalTaskEventHandler (MpxEventBase* event)
{
	MpxExternalTaskEvent* externalTaskEvent = dynamic_cast < MpxExternalTaskEvent* > (event);
	if (externalTaskEvent == 0)
		return;

	if (externalTaskEvent->error() != 0)
	{
		cout << "not connected to external task" << endl;
		return;
	}
	if (externalTaskEvent->flags() != EPOLLIN)
		return;
	if ((m_provider = dynamic_cast < MpxProxyTaskBase* > ((MpxTaskBase*) event->src())) == 0)
		return;
	Send (m_provider, new MpxConsumerEventA ("asdfasdfasdf"));
	Send (m_provider, new MpxConsumerEventB (10, 20, 30));
}

void TaskConsumer::ConsumerEventAHandler (MpxEventBase* event)
{
	MpxConsumerEventA* consumerEventA = static_cast <MpxConsumerEventA*> (event);
	if (consumerEventA == 0)
		return;

	MpxConsumerEventStruct* eventStruct = consumerEventA->eventStruct ();
	++m_acount;
	cout << m_acount << " " << eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventA.m_string << endl;

	Send ((MpxTaskBase*) event->src (),
		new MpxConsumerEventA (eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventA.m_string));
}

void TaskConsumer::ConsumerEventBHandler (MpxEventBase* event)
{
	MpxConsumerEventB* consumerEventB = static_cast <MpxConsumerEventB*> (event);
	if (consumerEventB == 0)
		return;

	MpxConsumerEventStruct* eventStruct = consumerEventB->eventStruct ();
	++m_bcount;
	cout << m_bcount << " " << eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.alpha << endl;
	cout << m_bcount << " " << eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.beta << endl;
	cout << m_bcount << " " << eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.gama << endl;

	Send ((MpxTaskBase*) event->src (),
		new MpxConsumerEventB (eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.alpha,
			eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.beta,
			eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.gama));
}

} // namespace mpx_task_consumer
