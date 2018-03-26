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
// TaskProvider.cpp
//
//  Created on: Mar 9, 2018
//      Author: miran
//

#include <TaskProvider.h>
#include <MpxConsumerEventA.h>
#include <MpxConsumerEventB.h>
using namespace mpx_edlib;

namespace mpx_task_provider
{

EventDescriptor TaskProvider::g_evntab [] =
{
{ AnyState, MpxExternalTaskEvent::EventCode, ExternalTaskEventHandler },
{ AnyState, MpxTimerEvent::EventCode, TimerEventHandler },
{ AnyState, MpxConsumerEventA::EventCode, ConsumerEventAHandler },
{ AnyState, MpxConsumerEventB::EventCode, ConsumerEventBHandler },
{ 0, 0, 0 } };

MpxTaskBase::evnset TaskProvider::g_evnset = MpxTaskBase::CreateEventSet (g_evntab);

TaskProvider::TaskProvider (const char* name) :
	MpxTaskBase (g_evnset, name)
{
}

TaskProvider::~TaskProvider ()
{
}

void TaskProvider::StartTask ()
{
}

void TaskProvider::StopTask ()
{
}

void TaskProvider::ExternalTaskEventHandler (MpxEventBase* event)
{
	MpxExternalTaskEvent* externalTaskEvent = dynamic_cast <MpxExternalTaskEvent*> (event);
	if (externalTaskEvent == 0)
		return;

	if (externalTaskEvent->error () != 0)
	{
		cout << "not connected to external task" << endl;
		return;
	}
	if (externalTaskEvent->flags () != EPOLLIN)
		return;
	MpxTaskBase* proxyTask = reinterpret_cast <MpxTaskBase*> (event->src ());
	timespec t = GetCurrentTime ();
	t.tv_sec += 1;
	m_proxyset [StartTimer (t)] = proxyTask;
	cout << "provider proxy = " << proxyTask << endl;
}

void TaskProvider::TimerEventHandler (MpxEventBase* event)
{
	MpxTimerEvent* timerEvent = dynamic_cast <MpxTimerEvent*> (event);
	if (timerEvent == 0)
		return;

	proxyset::iterator it = m_proxyset.find (timerEvent);
	if (it == m_proxyset.end ())
		return;

	((MpxProxyTaskBase*)it->second)->Disconnect ();
	m_proxyset.erase (it);
}

void TaskProvider::ConsumerEventAHandler (MpxEventBase* event)
{
	MpxConsumerEventA* consumerEventA = dynamic_cast <MpxConsumerEventA*> (event);
	if (consumerEventA == 0)
	{
		cout << "cannot cast to MpxConsumerEventA" << endl;
		return;
	}

	MpxConsumerEventStruct* eventStruct = consumerEventA->eventStruct ();
//	cout << name() << ": " << eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventA.m_string << endl;

	Send (reinterpret_cast <MpxTaskBase*> (event->src ()),
		new MpxConsumerEventA (eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventA.m_string));
}

void TaskProvider::ConsumerEventBHandler (MpxEventBase* event)
{
	MpxConsumerEventB* consumerEventB = dynamic_cast <MpxConsumerEventB*> (event);
	if (consumerEventB == 0)
	{
		cout << "cannot cast to MpxConsumerEventB" << endl;
		return;
	}

	MpxConsumerEventStruct* eventStruct = consumerEventB->eventStruct ();
//	cout << name() << ": " << eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.alpha << endl;
//	cout << eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.beta << endl;
//	cout << eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.gama << endl;

	Send (reinterpret_cast <MpxTaskBase*> (event->src ()),
		new MpxConsumerEventB (eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.alpha,
			eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.beta,
			eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.gama));
}

} // namespace mpx_task_provider
