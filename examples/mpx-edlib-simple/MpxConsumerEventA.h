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
class MpxConsumerEventA: public MpxEventBase
{
public:
	MpxConsumerEventA (const char* str);
	virtual ~MpxConsumerEventA ();
	virtual const char* Name () { return "A"; }
	virtual MpxEventBase* Copy () { return new MpxConsumerEventA ((const char*) str()); }
	inline char* str () const
	{
		return m_str;
	}
public:
	static const unsigned int EventCode = static_cast <unsigned int> (::MpxConsumerEventACode);
private:
	char* m_str;
};

} // namespace mpx_edlib
