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
#include <string>
using namespace std;

namespace mpx
{

class MpxPosixMQTaskQueryEvent: public mpx::MpxEventBase
{
public:
	MpxPosixMQTaskQueryEvent (const char* path, const char* name);
	virtual ~MpxPosixMQTaskQueryEvent ();
	virtual const char* Name ()
	{
		return "Posix MQ task query event";
	}
	virtual MpxEventBase* Copy ()
	{
		return new MpxPosixMQTaskQueryEvent (*this);
	}
	inline const char* path ()
	{
		return m_path.c_str ();
	}
	inline const char* name ()
	{
		return m_name.c_str ();
	}
public:
	static const unsigned int EventCode = static_cast <unsigned int> (::MpxPosixMQTaskQueryEventCode);
private:
	string m_path;
	string m_name;
};

} /* namespace mpx */
