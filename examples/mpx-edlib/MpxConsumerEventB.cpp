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
// MpxConsumerEventB.cpp
//
//  Created on: Mar 12, 2018
//      Author: miran
//

#include <MpxConsumerEventB.h>

namespace mpx_edlib
{

MpxConsumerEventB::MpxConsumerEventB (MpxConsumerEventStruct* eventStruct) :
		MpxConsumerEvent <MpxConsumerEventB> (eventStruct)
{

}

MpxConsumerEventB::MpxConsumerEventB (int alpha, int beta, int gama) :
	MpxConsumerEvent (MpxConsumerEventB::EventCode)
{
	m_eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.alpha = alpha;
	m_eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.beta = beta;
	m_eventStruct->MpxConsumerEventStruct_u.m_ConsumerEventB.gama = gama;
}

MpxConsumerEventB::~MpxConsumerEventB ()
{
}

} // namespace mpx_edlib
