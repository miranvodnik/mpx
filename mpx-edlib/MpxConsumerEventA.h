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
// MpxConsumerEventA.h
//
//  Created on: Mar 12, 2018
//      Author: miran
//

#pragma once

#include <MpxConsumerEvent.h>
using namespace mpx;

namespace mpx_edlib
{

//
//
//
class MpxConsumerEventA: public MpxConsumerEvent <MpxConsumerEventA>
{
public:
	MpxConsumerEventA (const char* str);
	virtual ~MpxConsumerEventA ();
	inline const char* str ()
	{
		return m_eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventA.m_string;
	}
public:
	static const unsigned int EventCode = (unsigned int) ::MpxConsumerEventACode;
};

} // namespace mpx_edlib
