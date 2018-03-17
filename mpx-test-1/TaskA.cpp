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
// TaskA.cpp
//
//  Created on: Feb 23, 2018
//      Author: miran
//

#include "TaskA.h"

namespace mpx_test_1
{

EventDescriptor TaskA::g_events [] =
{
	{ AnyState, EventB::EventBCode, EventBHandler },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset TaskA::g_evnset = MpxTaskBase::CreateEventSet (TaskA::g_events);

TaskA::TaskA() : MpxTaskBase (g_evnset)
{
	m_peer = 0;
}

TaskA::~TaskA()
{

}

void TaskA::StartTask ()
{
	Send (m_peer, new EventA);
}

void TaskA::StopTask ()
{
}

void TaskA::EventBHandler (MpxEventBase* event)
{
	Send (m_peer, new EventA);
	cout << "Event B" << endl;
}

} // namespace mpx_test_1
