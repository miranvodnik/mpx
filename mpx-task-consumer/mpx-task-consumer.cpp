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
// mpx-task-consumer.cpp
//
//  Created on: Mar 9, 2018
//      Author: miran
//

#include <mpx-core/MpxEnvironment.h>
#include <mpx-event-queues/MpxLocalMQTask.h>
using namespace mpx;
#include "TaskConsumer.h"
using namespace mpx_task_consumer;

int main (int n, char* p [])
{
	if (n < 5)
	{
		cout << "usage: %s <protocol> <hostname> <port> <taskname>" << endl;
		return 0;
	}
	MpxTaskMultiplexer* mpx = MpxEnvironment::CreateTaskMultiplexer ();
	TaskConsumer* task = new TaskConsumer (p[1], p[2], p[3], p[4]);
	mpx->RegisterTask(task);

	MpxEnvironment::Start (new MpxLocalMQTask ());
	while (true)
		sleep (1);
	return 0;
}
