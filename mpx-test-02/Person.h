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
// Person.h
//
//  Created on: Feb 23, 2018
//      Author: miran
//

#pragma once

#include <mpx-tasks/MpxTaskBase.h>
using namespace mpx;

#include <Question.h>
#include <Answer.h>

namespace mpx_test_02
{

//
//
//
class Person: public mpx::MpxTaskBase
{
public:
	Person (bool initiator, const char* name = 0) : MpxTaskBase(g_evnset, name)
	{
		m_peer = 0;
		m_initiator = initiator;
	}
	virtual ~Person ()
	{
	}
	virtual void StartTask ();
	virtual void StopTask ();

	mpx_event_handler(QuestionHandler, Person)
	mpx_event_handler(AnswerHandler, Person)

	inline void peer (MpxTaskBase* peer)
	{
		m_peer = peer;
	}
private:
	static EventDescriptor g_evntab [];
	static evnset g_evnset;
	MpxTaskBase* m_peer;
	bool m_initiator;
};

} // namespace mpx_test_02
