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
// MpxConsumerEvent.h
//
//  Created on: Mar 12, 2018
//      Author: miran
//

#pragma once

#include <mpx-events.h>
#include <mpx-events/MpxEventBase.h>
using namespace mpx;

namespace mpx_edlib
{

class MpxConsumerEventBase: public MpxEventBase
{
protected:
	MpxConsumerEventBase (MpxConsumerEventBase& e) :
		MpxEventBase (e.code ())
	{
		MpxConsumerEventStruct* eventStruct = e.eventStruct ();
		u_long size = xdr_sizeof ((xdrproc_t) xdr_MpxConsumerEventStruct, (char*) eventStruct);
		char* data = (char*) alloca(size);
		XDR xdr;
		xdrmem_create (&xdr, data, size, XDR_ENCODE);
		xdr_MpxConsumerEventStruct (&xdr, eventStruct);
		m_eventStruct = new MpxConsumerEventStruct;
		xdrmem_create (&xdr, data, size, XDR_DECODE);
		xdr_MpxConsumerEventStruct (&xdr, m_eventStruct);
	}
	MpxConsumerEventBase (MpxConsumerEventStruct* eventStruct) :
		MpxEventBase (eventStruct->m_code)
	{
		m_eventStruct = eventStruct;
	}
public:
	MpxConsumerEventBase (unsigned int eventCode) :
		MpxEventBase (eventCode)
	{
		m_eventStruct = new MpxConsumerEventStruct;
		memset (m_eventStruct, 0, sizeof *m_eventStruct);
		m_eventStruct->m_code = (MpxConsumerEventCode) eventCode;
	}

	virtual ~MpxConsumerEventBase ()
	{
		if (m_eventStruct != 0)
		{
			xdr_free ((xdrproc_t) xdr_MpxConsumerEventStruct, (char*) m_eventStruct);
			delete m_eventStruct;
		}
		m_eventStruct = 0;
	}
	inline MpxConsumerEventStruct* eventStruct ()
	{
		return m_eventStruct;
	}
protected:
	MpxConsumerEventStruct* m_eventStruct;
};

//
//
//
template <typename T> class MpxConsumerEvent: public MpxConsumerEventBase
{
private:
	MpxConsumerEvent (MpxConsumerEventStruct* eventStruct) :
		MpxConsumerEventBase (eventStruct)
	{
	}
public:
	typedef T MpxConsumerEventType;
	MpxConsumerEvent (unsigned int eventCode) :
		MpxConsumerEventBase (eventCode)
	{
	}

	virtual ~MpxConsumerEvent ()
	{
	}

	inline const char* Name ()
	{
		return "Consumer Event";
	}
	inline MpxEventBase* Copy ()
	{
		return new MpxConsumerEventType (*((MpxConsumerEventType*) this));
	}
	static MpxEventBase* CreateConsumerEvent (MpxConsumerEventStruct* eventStruct)
	{
		return new MpxConsumerEvent < MpxConsumerEventType > (eventStruct);
	}
};

} // namespace mpx_edlib
