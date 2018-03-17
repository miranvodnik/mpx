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
// Question.h
//
//  Created on: Feb 23, 2018
//      Author: miran
//

#pragma once

#include <stdlib.h>

#include <mpx-events/MpxEventBase.h>
using namespace mpx;

namespace mpx_test_02
{

//
//
//
class Question: public mpx::MpxEventBase
{
public:
	Question (const char* question) : MpxEventBase (EventCode), m_question (strdup (question)) {}
	virtual ~Question () { free ((void*) m_question); }

	virtual const char* Name () { return "Question"; }
	virtual MpxEventBase* Copy () { return new Question (*this); }

	inline const char* question () { return (const char*) m_question; }
public:
	static const unsigned int EventCode = 2;
private:
	const char* m_question;
};

} // namespace mpx_test_02
