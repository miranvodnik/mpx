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
// MpxConsumerEventA.cpp
//
//  Created on: Mar 12, 2018
//      Author: miran
//

#include <MpxConsumerEventA.h>

namespace mpx_edlib
{

MpxConsumerEventA::MpxConsumerEventA (const char* str) :
	MpxConsumerEvent (MpxConsumerEventA::EventCode)
{
	m_eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventA.m_string = strdup ((str != 0) ? str : "");
}

MpxConsumerEventA::~MpxConsumerEventA ()
{
}

} // namespace mpx_edlib
