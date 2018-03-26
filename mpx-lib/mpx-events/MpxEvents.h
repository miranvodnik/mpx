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

#include <mpx-events/MpxEventXDRItf.h>
#include <mpx-events/MpxExternalTaskEvent.h>
#include <mpx-events/MpxJobFinishedEvent.h>
#include <mpx-events/MpxLocalClientEvent.h>
#include <mpx-events/MpxLocalEndPointEvent.h>
#include <mpx-events/MpxLocalListenerEvent.h>
#include <mpx-events/MpxLocalTaskQueryEvent.h>
#include <mpx-events/MpxPosixMQEvent.h>
#include <mpx-events/MpxPosixMQTaskQueryEvent.h>
#include <mpx-events/MpxStartEvent.h>
#include <mpx-events/MpxStopEvent.h>
#include <mpx-events/MpxTaskQueryEvent.h>
#include <mpx-events/MpxTaskResponseEvent.h>
#include <mpx-events/MpxTcp4ClientEvent.h>
#include <mpx-events/MpxTcp4EndPointEvent.h>
#include <mpx-events/MpxTcp4ListenerEvent.h>
#include <mpx-events/MpxTcp4TaskQueryEvent.h>
#include <mpx-events/MpxTcp6ClientEvent.h>
#include <mpx-events/MpxTcp6EndPointEvent.h>
#include <mpx-events/MpxTcp6ListenerEvent.h>
#include <mpx-events/MpxTcp6TaskQueryEvent.h>
#include <mpx-events/MpxTimerEvent.h>
#include <mpx-events/MpxUdp4TaskQueryEvent.h>
#include <mpx-events/MpxUdp6TaskQueryEvent.h>
#include <mpx-events/MpxProxyTaskEvent.h>
#include <mpx-events/MpxProxyTaskRelocationEvent.h>
