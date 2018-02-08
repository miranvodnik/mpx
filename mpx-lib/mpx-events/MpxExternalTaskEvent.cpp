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

#include <mpx-events/MpxExternalTaskEvent.h>

namespace mpx
{

int MpxExternalTaskEvent::Encode (xdrproc_t& proc, xdrdata_t& data)
{
	MpxEventStruct* event = new MpxEventStruct ();
	event->m_code = MpxExternalTaskEventCode;
	event->MpxEventStruct_u.m_ExternalTaskEvent.base.m_code = code ();
	event->MpxEventStruct_u.m_ExternalTaskEvent.base.m_src = (u_long) src ();
	event->MpxEventStruct_u.m_ExternalTaskEvent.base.m_dst = (u_long) dst ();
	event->MpxEventStruct_u.m_ExternalTaskEvent.m_flags = m_flags;
	event->MpxEventStruct_u.m_ExternalTaskEvent.m_error = m_error;
	event->MpxEventStruct_u.m_ExternalTaskEvent.m_size = m_size;
	event->MpxEventStruct_u.m_ExternalTaskEvent.m_buffer.m_buffer_len = m_size;
	event->MpxEventStruct_u.m_ExternalTaskEvent.m_buffer.m_buffer_val = (char*) m_buffer;

	proc = (xdrproc_t) xdr_MpxEventStruct;
	data = event;

	return ExternalTaskEvent;
}

int MpxExternalTaskEvent::Decode (MpxEventStruct* eventStruct)
{
	return 0;
}

} // namespace mpx
