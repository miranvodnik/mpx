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

#include <mpx-events/MpxEventBase.h>

namespace mpx
{

class MpxTcp6ClientEvent: public mpx::MpxEventBase
{
public:
	MpxTcp6ClientEvent (void* endPoint, u_int flags, u_int error, u_int size, u_char* buffer);
	virtual ~MpxTcp6ClientEvent ();
	virtual const char* Name ()
	{
		return "TCP 6 Client Event";
	}
	virtual MpxEventBase* Copy ()
	{
		return new MpxTcp6ClientEvent (*this);
	}
	void* endPoint ()
	{
		return m_endPoint;
	}
	inline u_int flags ()
	{
		return m_flags;
	}
	inline u_int error ()
	{
		return m_error;
	}
	inline u_int size ()
	{
		return m_size;
	}
	inline u_char* buffer ()
	{
		return m_buffer;
	}
public:
	static const unsigned int EventCode = static_cast <unsigned int> (::MpxTcp6ClientEventCode);
private:
	void* m_endPoint;
	u_int m_flags;
	u_int m_error;
	u_int m_size;
	u_char* m_buffer;
};

} /* namespace mpx */
