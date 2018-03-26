//    TODO
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

//
// MpxProxyTaskEvent.h
//
//  Created on: Mar 23, 2018
//      Author: miran
//

#pragma once

#include <mpx-events/MpxEventBase.h>

namespace mpx
{

//
//
//
class MpxProxyTaskEvent: public MpxEventBase
{
public:
	MpxProxyTaskEvent (int reasonForCall) :
		MpxEventBase (MpxProxyTaskEvent::EventCode), m_reasonForCall (reasonForCall)
	{
	}
	virtual ~MpxProxyTaskEvent ()
	{
	}
	virtual const char* Name ()
	{
		return "Proxy Task Event";
	}
	virtual MpxEventBase* Copy ()
	{
		return new MpxProxyTaskEvent (*this);
	}
	inline int reasonForCall ()
	{
		return m_reasonForCall;
	}
public:
	static const unsigned int EventCode = static_cast <unsigned int> (::MpxProxyTaskEventCode);
	static const int ReasonConnectionBroken = 1;
private:
	int m_reasonForCall;
};

} // namespace mpx
