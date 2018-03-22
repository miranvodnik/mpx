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

class MpxTcp6TaskQueryEvent: public mpx::MpxEventBase
{
public:
	MpxTcp6TaskQueryEvent (const char* hostname, char* port, char* name, const char* encdeclib);
	virtual ~MpxTcp6TaskQueryEvent ();
	virtual const char* Name ()
	{
		return "Tcp6 task query event";
	}
	virtual MpxEventBase* Copy ()
	{
		return new MpxTcp6TaskQueryEvent (*this);
	}
	inline const char* hostname ()
	{
		return m_hostname.c_str ();
	}
	inline const char* port ()
	{
		return m_port.c_str ();
	}
	inline const char* name ()
	{
		return m_name.c_str ();
	}
	inline const char* encdeclib ()
	{
		return m_encdeclib.c_str ();
	}
public:
	static const unsigned int EventCode = static_cast <unsigned int> (::MpxTcp6TaskQueryEventCode);
private:
	string m_hostname;
	string m_port;
	string m_name;
	string m_encdeclib;
};

} /* namespace mpx */
