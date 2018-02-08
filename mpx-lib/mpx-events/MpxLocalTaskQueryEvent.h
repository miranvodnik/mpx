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

class MpxLocalTaskQueryEvent: public mpx::MpxEventBase
{
public:
	MpxLocalTaskQueryEvent (const char* path, const char* name);
	virtual ~MpxLocalTaskQueryEvent ();
	virtual const char* Name ()
	{
		return "local task query event";
	}
	virtual MpxEventBase* Copy ()
	{
		return new MpxLocalTaskQueryEvent (*this);
	}
	virtual int Encode (xdrproc_t& proc, xdrdata_t& data)
	{
		return 0;
	}
	virtual int Decode (MpxEventStruct* eventStruct)
	{
		return 0;
	}
	inline const char* path ()
	{
		return m_path.c_str ();
	}
	inline const char* name ()
	{
		return m_name.c_str ();
	}
private:
	string m_path;
	string m_name;
};

} /* namespace mpx */
