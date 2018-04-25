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
// mpx-task-provider.cpp
//
//  Created on: Mar 9, 2018
//      Author: miran
//

#include <mpx-core/MpxEnvironment.h>
#include <mpx-event-queues/MpxLocalMQTask.h>
using namespace mpx;
#include "TaskProvider.h"
using namespace mpx_task_provider;

int main (int n, char* p [])
{
	MpxEnvironment::CreateTaskMultiplexer ("protocol:tcp4,address:all,port:22220;"
		"protocol:tcp6,address:all,port:22220;"
		"protocol:local,path:mpx-test-22220;");

	MpxTaskMultiplexer* mpx = MpxEnvironment::CreateTaskMultiplexer ();

	TaskProvider* task;

	task = new TaskProvider ("task-provider-0");
	mpx->RegisterTask (task);

	task = new TaskProvider ("task-provider-1");
	mpx->RegisterTask (task);

	task = new TaskProvider ("task-provider-2");
	mpx->RegisterTask (task);

	task = new TaskProvider ("task-provider-3");
	mpx->RegisterTask (task);

	task = new TaskProvider ("task-provider-4");
	mpx->RegisterTask (task);

	task = new TaskProvider ("task-provider-5");
	mpx->RegisterTask (task);

	task = new TaskProvider ("task-provider-6");
	mpx->RegisterTask (task);

	task = new TaskProvider ("task-provider-7");
	mpx->RegisterTask (task);

	task = new TaskProvider ("task-provider-8");
	mpx->RegisterTask (task);

	task = new TaskProvider ("task-provider-9");
	mpx->RegisterTask (task);

	MpxEnvironment::Start (new MpxLocalMQTask ());
	while (true)
		sleep (1);
	return 0;
}
