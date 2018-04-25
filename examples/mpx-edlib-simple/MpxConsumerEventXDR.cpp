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
	switch (event->code())
	{
	case MpxConsumerEventA::EventCode:
		{
			MpxConsumerEventA* a = dynamic_cast <MpxConsumerEventA*> (event);
			if (a == 0)
				return -1;
			size_t asize = 3 + strlen (a->str());
			if (asize > size)
				return asize;
			buffer[0] = MpxConsumerEventA::EventCode;
			buffer[1] = asize - 3;
			strcpy (buffer + 2, a->str());
		}
		break;
	case MpxConsumerEventB::EventCode:
		{
			MpxConsumerEventB* b = dynamic_cast <MpxConsumerEventB*> (event);
			if (b == 0)
				return -1;
			if (4 > size)
				return 4;
			buffer[0] = MpxConsumerEventB::EventCode;
			buffer[1] = b->alpha();
			buffer[2] = b->beta();
			buffer[3] = b->gama();
		}
		break;
	default:
		return -1;
	}
	return 0;
}

int MpxConsumerEventXDR::Decode (MpxEventBase*& event, char* buffer, size_t size)
{
	if (size < 1)
		return -1;
	switch (buffer[0])
	{
	case MpxConsumerEventACode:
		{
			if (size < 2)
				return -1;
			size_t len = buffer[1];
			if (size < len + 3)
				return -1;
			char* str = (char*) alloca (len + 1);
			strncpy (str, buffer + 2, len + 1);
			str[len] = 0;
			event = new MpxConsumerEventA (str);
			return len + 3;
		}
		break;
	case MpxConsumerEventBCode:
		{
			if (size < 4)
				return -1;
			event = new MpxConsumerEventB (buffer[1], buffer[2], buffer[3]);
			return 4;
		}
		break;
	default:
		break;
	}
	event = 0;
	return -1;
}

extern "C" MpxEventXDRItf* CreateMpxEventXDR ()
{
	return new MpxConsumerEventXDR ();
}

} // namespace mpx_edlib
