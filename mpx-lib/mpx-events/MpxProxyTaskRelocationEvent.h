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
// MpxProxyTaskRelocationEvent.h
//
//  Created on: Mar 26, 2018
//      Author: miran
//

#pragma once

#include <mpx-events/MpxEventBase.h>
#include <mpx-tasks/MpxProxyTask.h>

namespace mpx
{

//
//
//
class MpxProxyTaskRelocationEvent: public MpxEventBase
{
public:
	MpxProxyTaskRelocationEvent (MpxTaskBase* proxyTask) :
		MpxEventBase (MpxProxyTaskRelocationEvent::EventCode), m_proxyTask (proxyTask)
	{
	}
	virtual ~MpxProxyTaskRelocationEvent ()
	{
	}
	virtual const char* Name ()
	{
		return "Proxy Task Relocation Event";
	}
	virtual MpxEventBase* Copy ()
	{
		return new MpxProxyTaskRelocationEvent (*this);
	}
	inline MpxTaskBase* proxyTask ()
	{
		return m_proxyTask;
	}
public:
	static const unsigned int EventCode = static_cast <unsigned int> (::MpxProxyTaskRelocationEventCode);
private:
	MpxTaskBase* m_proxyTask;
};

} // namespace mpx
