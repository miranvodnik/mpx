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

class MpxTaskQueryEvent: public MpxEventBase
{
public:
	MpxTaskQueryEvent (const char* taskName, const char* encdeclib, MpxTaskQueryEventType queryType, void* endPoint) :
		MpxEventBase (MpxTaskQueryEvent::EventCode), m_taskName (taskName), m_encdeclib (encdeclib), m_queryType (
			queryType), m_endPoint (endPoint)
	{
	}
	virtual ~MpxTaskQueryEvent ()
	{
	}
	virtual const char* Name ()
	{
		return "Task Query Event";
	}
	virtual MpxEventBase* Copy ()
	{
		return new MpxTaskQueryEvent (*this);
	}
	inline const char* taskName ()
	{
		return m_taskName.c_str ();
	}
	inline const char* encdeclib ()
	{
		return m_encdeclib.c_str ();
	}
	inline MpxTaskQueryEventType queryType ()
	{
		return m_queryType;
	}
	inline void* endPoint ()
	{
		return m_endPoint;
	}
public:
	static const unsigned int EventCode = static_cast <unsigned int> (::MpxTaskQueryEventCode);
private:
	string m_taskName;
	string m_encdeclib;
	MpxTaskQueryEventType m_queryType;
	void* m_endPoint;
};

} // namespace mpx
