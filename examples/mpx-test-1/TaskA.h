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
// TaskA.h
//
//  Created on: Feb 23, 2018
//      Author: miran
//

#pragma once

#include <EventA.h>
#include <EventB.h>
#include <mpx-tasks/MpxTaskBase.h>
#include <mpx-events/MpxStartEvent.h>
#include <mpx-events/MpxStopEvent.h>
using namespace mpx;

namespace mpx_test_1
{

//
//
//
class TaskA: public mpx::MpxTaskBase
{
public:
	TaskA ();
	virtual ~TaskA ();
	virtual void StartTask ();
	virtual void StopTask ();

	void peer (MpxTaskBase* task) { m_peer = task; }
private:
	mpx_event_handler(EventBHandler, TaskA)

private:
	static EventDescriptor g_events [];
	static evnset g_evnset;
	MpxTaskBase* m_peer;
};

} // namespace mpx_test_1
