/*
// mpx-events.x
//  Created on: Mar 12, 2018
//     Author: miran
*/
/*
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
*/

#ifdef RPC_HDR
%#include <mpx-events/mpx-events.h>
#endif

enum MpxConsumerEventCode
{
	MpxConsumerEventACode = 1,
	MpxConsumerEventBCode = 2
};

struct	MpxConsumerEventAStruct
{
	MpxEventBaseStruct base;
	string m_string<>;
};

struct	MpxConsumerEventBStruct
{
	MpxEventBaseStruct base;
	int alpha;
	int beta;
	int gama;
};

union MpxConsumerEventStruct
switch (MpxConsumerEventCode m_code)
{
	case MpxConsumerEventACode:
		MpxConsumerEventAStruct m_ConsumerEventA;
	case MpxConsumerEventBCode:
		MpxConsumerEventBStruct m_ConsumerEventB;
};
