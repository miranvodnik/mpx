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

#include <mpx-events/MpxLocalClientEvent.h>

namespace mpx
{

MpxLocalClientEvent::MpxLocalClientEvent (void* endPoint, u_int flags, u_int error, u_int size, u_char* buffer) :
	MpxEventBase (MpxLocalClientEvent::EventCode), m_endPoint (endPoint), m_flags (flags), m_error (error), m_size (
		size), m_buffer (buffer)
{
}

MpxLocalClientEvent::MpxLocalClientEvent (MpxLocalClientEvent& cln) :
	MpxEventBase (MpxLocalClientEvent::EventCode)
{
	m_endPoint = cln.endPoint ();
	m_flags = cln.flags ();
	m_error = cln.error ();
	m_size = cln.size ();
	if ((m_buffer = static_cast <u_char*> (malloc (m_size))) != 0)
		memcpy (m_buffer, cln.buffer (), m_size);
	else
		m_buffer = 0;
}

MpxLocalClientEvent::~MpxLocalClientEvent ()
{
}

} /* namespace mpx */
