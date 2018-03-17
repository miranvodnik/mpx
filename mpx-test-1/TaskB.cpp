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
// TaskB.cpp
//
//  Created on: Feb 23, 2018
//      Author: miran
//

#include "TaskB.h"

namespace mpx_test_1
{

EventDescriptor TaskB::g_events [] =
{
	{ AnyState, EventA::EventACode, EventAHandler },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset TaskB::g_evnset = MpxTaskBase::CreateEventSet (TaskB::g_events);
TaskB::TaskB() : MpxTaskBase (g_evnset)
{
	m_peer = 0;
}

TaskB::~TaskB()
{

}

void TaskB::StartTask ()
{
	Send (m_peer, new EventB);
}

void TaskB::StopTask ()
{
}

void TaskB::EventAHandler (MpxEventBase* event)
{
	Send (m_peer, new EventB);
	cout << "Event A" << endl;
}

} // namespace mpx_test_1
