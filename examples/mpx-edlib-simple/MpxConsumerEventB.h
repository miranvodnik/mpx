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
// MpxConsumerEventB.h
//
//  Created on: Mar 12, 2018
//      Author: miran
//

#pragma once

#include <MpxConsumerEvent.h>

namespace mpx_edlib
{

//
//
//
class MpxConsumerEventB: public MpxEventBase
{
public:
	MpxConsumerEventB (int alpha, int beta, int gama);
	virtual ~MpxConsumerEventB ();
	virtual const char* Name () { return "B"; }
	virtual MpxEventBase* Copy () { return new MpxConsumerEventB (*this); }
	inline int alpha ()
	{
		return m_alpha;
	}
	inline int beta ()
	{
		return m_beta;
	}
	inline int gama ()
	{
		return m_gama;
	}
public:
	static const unsigned int EventCode = static_cast <unsigned int> (::MpxConsumerEventBCode);
private:
	int m_alpha;
	int m_beta;
	int m_gama;
};

} // namespace mpx_edlib
