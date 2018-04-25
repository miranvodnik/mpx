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
// Task003.cpp
//
//  Created on: Mar 5, 2018
//      Author: miran
//

#include <Task003.h>

namespace mpx_test_03
{

EventDescriptor Task003::g_evntab [] =
{
	{ AnyState, MpxExternalTaskEvent::EventCode, ExternalTaskEventHandler }
};

MpxTaskBase::evnset Task003::g_evnset = MpxTaskBase::CreateEventSet (g_evntab);

Task003::Task003 () :
	MpxTaskBase (g_evnset)
{
}

Task003::~Task003 ()
{
}

void Task003::StartTask ()
{
	RetrieveExternalTask ("protocol:local,path:mpx-ext-link,name:ext-task;");
}

void Task003::StopTask ()
{

}

void Task003::ExternalTaskEventHandler (MpxEventBase* event)
{
	MpxExternalTaskEvent* externalTaskEvent = dynamic_cast < MpxExternalTaskEvent* > (event);
	if (externalTaskEvent == 0)
		return;

	if (externalTaskEvent->error() == 0)
		cout << "connected to external task" << endl;
	else
		cout << "not connected to external task" << endl;
}

} // namespace mpx_test_03
