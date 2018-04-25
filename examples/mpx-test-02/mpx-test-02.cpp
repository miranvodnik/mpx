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
// mpx-test-02.cpp
//
//  Created on: Feb 23, 2018
//      Author: miran
//

#include <mpx-core/MpxEnvironment.h>
#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-event-queues/MpxLocalMQTask.h>
using namespace mpx;

#include <Person.h>
using namespace mpx_test_02;

#include <iostream>
using namespace std;

int main (int n, char* p[])
{
	MpxTaskMultiplexer* mpxA = MpxEnvironment::CreateTaskMultiplexer();
	MpxTaskMultiplexer* mpxB = MpxEnvironment::CreateTaskMultiplexer();
	Person* personA;
	Person* personB;
	mpxA->RegisterTask(personA = new Person (true, "A: "));
	mpxB->RegisterTask(personB = new Person (false, "B: "));
	personA->peer (personB);
	personB->peer (personA);

	MpxEnvironment::Start (new MpxLocalMQTask ());

	while (true)
		sleep (1);

	return	0;
}
