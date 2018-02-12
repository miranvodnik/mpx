//    Event Driven Task Multiplexing Library
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

#include <mpx-tasks/MpxPosixMQProxyTask.h>

namespace mpx
{

EventDescriptor MpxPosixMQProxyTask::g_evntab[] =
{
	{ AnyState, PosixMQEvent, HandlePosixMQEvent, 0 },
	{ 0, 0, 0, 0 }
};

MpxPosixMQProxyTask::MpxPosixMQProxyTask (MpxTaskBase* task, MpxPosixMQ* posixMQ) :
	MpxTaskBase (), m_task (task), m_posixMQ (posixMQ)
{
	RegisterEventHandlers (g_evntab);
}

MpxPosixMQProxyTask::~MpxPosixMQProxyTask ()
{
}

void MpxPosixMQProxyTask::HandlePosixMQEvent (MpxEventBase *event)
{
	cout << "proxy event: posixmq" << endl;
}

} // namespace mpx
