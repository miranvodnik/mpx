//    Event Driven Task Multiplexing Library
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

#pragma once

#include <mpx-core/MpxUtilities.h>
#include <mpx-working-threads/MpxJob.h>
#include <mpx-tasks/MpxTaskBase.h>

namespace mpx
{

class MpxWorkerTask: public mpx::MpxTaskBase
{
public:
	MpxWorkerTask (const char* name = 0);
	virtual ~MpxWorkerTask ();
private:
	mpx_event_handler (StartEventHandler, MpxWorkerTask)
	;mpx_event_handler (StopEventHandler, MpxWorkerTask)
	;mpx_event_handler (TimerEventHandler, MpxWorkerTask)
	;

	inline static void CleanupExecute (void* arg)
	{
		((MpxWorkerTask*) arg)->_CleanupExecute ();
	}
	void _CleanupExecute ();
private:
	void* m_getTimer;
	void* m_sendTimer;
	MpxJob* m_job;
};

} /* namespace mpx */
