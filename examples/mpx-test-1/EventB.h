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
// EventB.h
//
//  Created on: Feb 23, 2018
//      Author: miran
//

#pragma once

#include <mpx-events/MpxEventBase.h>
using namespace mpx;

namespace mpx_test_1
{

//
//
//
class EventB: public mpx::MpxEventBase
{
public:
	EventB () : MpxEventBase (EventBCode) {}
	virtual ~EventB () {}

	virtual const char* Name () { return "Event_B"; }
	virtual MpxEventBase* Copy () { return new EventB (*this); }
public:
	static const int EventBCode = 2;
};

} // namespace mpx_test_1
