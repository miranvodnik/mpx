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
// TaskProvider.h
//
//  Created on: Mar 9, 2018
//      Author: miran
//

#pragma once

#include <mpx-tasks/MpxProxyTask.h>
#include <mpx-events/MpxEvents.h>
using namespace mpx;

#include <map>
using namespace std;

namespace mpx_task_provider
{

//
//
//
class TaskProvider: public mpx::MpxTaskBase
{
	typedef map < void*, MpxTaskBase* > proxyset;
public:
	TaskProvider (const char* name);
	virtual ~TaskProvider ();
private:
	virtual void StartTask ();
	virtual void StopTask ();
private:
	mpx_event_handler(ExternalTaskEventHandler, TaskProvider)
	mpx_event_handler(TimerEventHandler, TaskProvider)
	mpx_event_handler(ConsumerEventAHandler, TaskProvider)
	mpx_event_handler(ConsumerEventBHandler, TaskProvider)
private:
	static EventDescriptor g_evntab[];
	static evnset g_evnset;
	proxyset m_proxyset;
};

} // namespace mpx_task_provider
