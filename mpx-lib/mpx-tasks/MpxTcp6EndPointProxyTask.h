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
#include <mpx-sockets/MpxTcp6EndPoint.h>
#include <mpx-events/MpxTcp6EndPointEvent.h>

namespace mpx
{

class MpxTcp6EndPointProxyTask: public MpxProxyTask <MpxTcp6EndPointEvent, MpxTcp6EndPointProxyTask>
{
public:
	MpxTcp6EndPointProxyTask (MpxTaskBase* task, const char* encdeclib, MpxTcp6EndPoint* tcp6EndPoint);
	virtual ~MpxTcp6EndPointProxyTask ();
public:
	mpx_event_handler(HandleSocketEvent, MpxTcp6EndPointProxyTask)
};

} // namespace mpx
