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

class MpxTcp6ListenerEvent: public mpx::MpxEventBase
{
public:
	MpxTcp6ListenerEvent (int fd) :
		MpxEventBase (MpxTcp6ListenerEvent::EventCode), m_fd (fd)
	{
	}
	virtual ~MpxTcp6ListenerEvent ()
	{
	}
	virtual const char* Name ()
	{
		return "TCP4 Listener Event";
	}
	virtual MpxEventBase* Copy ()
	{
		return new MpxTcp6ListenerEvent (*this);
	}
	inline int fd ()
	{
		return m_fd;
	}
public:
	static const unsigned int EventCode = (unsigned int) ::MpxTcp6ListenerEventCode;
private:
	int m_fd;
};

} /* namespace mpx */
