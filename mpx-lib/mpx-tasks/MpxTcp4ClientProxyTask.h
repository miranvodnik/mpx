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

#include <mpx-tasks/MpxProxyTask.h>
#include <mpx-sockets/MpxTcp4Client.h>
#include <mpx-events/MpxTcp4ClientEvent.h>

namespace mpx
{

class MpxTcp4ClientProxyTask: public MpxProxyTask <MpxTcp4ClientEvent, MpxTcp4ClientProxyTask>
{
public:
	MpxTcp4ClientProxyTask (MpxTaskBase* task, const char* encdeclib, MpxTcp4Client* tcp4Client);
	virtual ~MpxTcp4ClientProxyTask ();
public:
	mpx_event_handler(HandleSocketEvent, MpxTcp4ClientProxyTask)
};

} // namespace mpx
