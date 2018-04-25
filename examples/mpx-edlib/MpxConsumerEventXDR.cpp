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
// MpxConsumerEventXDR.cpp
//
//  Created on: Mar 13, 2018
//      Author: miran
//

#include <MpxConsumerEventXDR.h>
#include "MpxConsumerEvent.h"
#include "MpxConsumerEventA.h"
#include "MpxConsumerEventB.h"

namespace mpx_edlib
{

MpxConsumerEventXDR::MpxConsumerEventXDR ()
{
}

MpxConsumerEventXDR::~MpxConsumerEventXDR ()
{
}

int MpxConsumerEventXDR::Encode (MpxEventBase*& event, char* buffer, size_t size)
{
	MpxConsumerEventBase* consumerEvent = dynamic_cast <MpxConsumerEventBase*> (event);
	if (consumerEvent == 0)
		return -1;
	size_t xdrSize = xdr_sizeof (reinterpret_cast <xdrproc_t> (xdr_MpxConsumerEventStruct), reinterpret_cast <char*> (consumerEvent->eventStruct ()));
	if (xdrSize > size)
		return xdrSize;
	XDR xdr;
	xdrmem_create (&xdr, buffer, xdrSize, XDR_ENCODE);
	xdr_MpxConsumerEventStruct (&xdr, consumerEvent->eventStruct ());
	return 0;
}

int MpxConsumerEventXDR::Decode (MpxEventBase*& event, char* buffer, size_t size)
{
	MpxConsumerEventStruct* eventStruct = new MpxConsumerEventStruct;
	memset (eventStruct, 0, sizeof *eventStruct);
	XDR xdr;
	xdrmem_create (&xdr, buffer, size, XDR_DECODE);
	if (xdr_MpxConsumerEventStruct (&xdr, eventStruct) != TRUE)
	{
		delete eventStruct;
		return -1;
	}
	switch (eventStruct->m_code)
	{
	case MpxConsumerEventACode:
		event = MpxConsumerEventA::CreateConsumerEvent (eventStruct);
		break;
	case MpxConsumerEventBCode:
		event = MpxConsumerEventB::CreateConsumerEvent (eventStruct);
		break;
	default:
		event = 0;
		xdr_free (reinterpret_cast <xdrproc_t> (xdr_MpxConsumerEventStruct), reinterpret_cast <char*> (eventStruct));
		delete eventStruct;
		return -1;
		break;
	}
	return xdr.x_private - xdr.x_base;
}

extern "C" MpxEventXDRItf* CreateMpxEventXDR ()
{
	return new MpxConsumerEventXDR ();
}

} // namespace mpx_edlib
