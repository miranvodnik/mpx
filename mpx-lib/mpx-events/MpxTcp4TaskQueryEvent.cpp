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

#include <mpx-events/MpxTcp4TaskQueryEvent.h>

namespace mpx
{

MpxTcp4TaskQueryEvent::MpxTcp4TaskQueryEvent (const char* hostname, char* port, char* name) :
	MpxEventBase (Tcp4TaskQueryEvent), m_hostname (hostname), m_port (port), m_name (name)
{
}

MpxTcp4TaskQueryEvent::~MpxTcp4TaskQueryEvent ()
{
}

} /* namespace mpx */
