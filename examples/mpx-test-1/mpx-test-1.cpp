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
// mpx-test-1.cpp
//
//  Created on: Feb 22, 2018
//      Author: miran
//

#include <mpx-core/MpxEnvironment.h>
#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-event-queues/MpxLocalMQTask.h>
using namespace mpx;

#include <TaskA.h>
#include <TaskB.h>
using namespace mpx_test_1;

#include <iostream>
using namespace std;

int main (int n, char* p[])
{
	MpxTaskMultiplexer* mpxA = MpxEnvironment::CreateTaskMultiplexer();
	MpxTaskMultiplexer* mpxB = MpxEnvironment::CreateTaskMultiplexer();
	TaskA* taskA;
	TaskB* taskB;
	mpxA->RegisterTask(taskA = new TaskA);
	mpxB->RegisterTask(taskB = new TaskB);
	taskA->peer (taskB);
	taskB->peer (taskA);

	MpxEnvironment::Start (new MpxLocalMQTask ());

	while (true)
		sleep (1);

	return	0;
}
