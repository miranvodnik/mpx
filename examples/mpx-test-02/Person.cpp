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
// Person.cpp
//
//  Created on: Feb 23, 2018
//      Author: miran
//

#include <mpx-events/MpxStartEvent.h>
#include <mpx-events/MpxStopEvent.h>
using namespace mpx;

#include <Person.h>

namespace mpx_test_02
{

EventDescriptor Person::g_evntab [] =
{
	{ AnyState, Question::EventCode, QuestionHandler },
	{ AnyState, Answer::EventCode, AnswerHandler },
	{ 0, 0, 0 },
};

MpxTaskBase::evnset Person::g_evnset = MpxTaskBase::CreateEventSet (Person::g_evntab);

void Person::StartTask ()
{
	if (m_initiator)
		Send (m_peer, new Question ("How are you?"));
}

void Person::StopTask ()
{

}

void Person::QuestionHandler (MpxEventBase* event)
{
	Question* questionEvent = dynamic_cast < Question* > (event);
	if (questionEvent == 0)
		return;
	cout << name() << questionEvent->question() << endl;
	if (m_initiator)
	{
		Send (m_peer, new Answer ("I'm OK, too"));
	}
	else
	{
		Send (m_peer, new Answer ("I'm OK"));
		Send (m_peer, new Question ("And how are you?"));
	}
}

void Person::AnswerHandler (MpxEventBase* event)
{
	Answer* answerEvent = dynamic_cast < Answer* > (event);
	if (answerEvent == 0)
		return;
	cout << name() << answerEvent->answer() << endl;
}

} // namespace mpx_test_02
