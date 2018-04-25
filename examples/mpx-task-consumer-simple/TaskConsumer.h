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
// TaskConsumer.h
//
//  Created on: Mar 9, 2018
//      Author: miran
//

#pragma once

#include <mpx-tasks/MpxProxyTask.h>
#include <mpx-events/MpxEvents.h>
using namespace mpx;

#include <iostream>
using namespace std;

namespace mpx_task_consumer
{

//
//
//
class TaskConsumer: public mpx::MpxTaskBase
{
public:
	TaskConsumer (const char* protocol, const char* hostname, const char* port,
		const char* taskName);
	virtual ~TaskConsumer ();
private:
	virtual void StartTask ();
	virtual void StopTask ();
private:
	mpx_event_handler(ExternalTaskEventHandler, TaskConsumer)
	mpx_event_handler(ProxyTaskEventHandler, TaskConsumer)
	mpx_event_handler(ConsumerEventAHandler, TaskConsumer)
	mpx_event_handler(ConsumerEventBHandler, TaskConsumer)
public:
	inline string protocol ()
	{
		return m_protocol;
	}
	inline string hostname ()
	{
		return m_hostname;
	}
	inline int port ()
	{
		return m_port;
	}
	inline string taskName ()
	{
		return m_taskName;
	}
	static inline int acount ()
	{
		return m_acount;
	}
	static inline int bcount ()
	{
		return m_bcount;
	}
private:
	static EventDescriptor g_evntab [];
	static evnset g_evnset;
	static int m_acount;
	static int m_bcount;
	MpxProxyTaskBase* m_provider;
	string m_protocol;
	string m_hostname;
	int m_port;
	string m_taskName;
	bool m_afterProxy;
};

} // namespace mpx_task_consumer
