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

#pragma once

#include <mpx-tasks/MpxTaskBase.h>
#include <mpx-sockets/MpxPosixMQ.h>

namespace mpx
{

class MpxPosixMQProxyTask: public MpxTaskBase
{
public:
	MpxPosixMQProxyTask (MpxTaskBase* task, MpxPosixMQ* posixMQ);
	virtual ~MpxPosixMQProxyTask ();
	inline MpxTaskBase* task ()
	{
		return m_task;
	}
	inline MpxPosixMQ* posixMQ ()
	{
		return m_posixMQ;
	}
private:
	mpx_event_handler(HandlePosixMQEvent, MpxPosixMQProxyTask)
	;
private:
	static EventDescriptor g_evntab[];
	MpxTaskBase* m_task;
	MpxPosixMQ* m_posixMQ;
};

} // namespace mpx
