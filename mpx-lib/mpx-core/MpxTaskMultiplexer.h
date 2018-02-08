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

#include <mpx-core/MpxRunnable.h>
#include <mpx-core/MpxRunningContext.h>
#include <mpx-core/MpxUtilities.h>
#include <mpx-tasks/MpxTaskBase.h>
#include <mpx-event-queues/MpxMQTaskI.h>
#include <mpx-events/MpxTimerEvent.h>

#include <map>
using namespace std;

namespace mpx
{

class MpxTaskMultiplexer: public mpx::MpxRunnable
{

	typedef set <MpxTaskBase*> taskset;
	typedef map <string, MpxTaskBase*> taskmap;

public:
	MpxTaskMultiplexer (int fd [], const char* connStr = 0, bool isWorker = false);
	virtual ~MpxTaskMultiplexer ();
	inline MpxRunningContext* ctx ()
	{
		return m_ctx;
	}
	int RegisterTask (MpxTaskBase* task);
	MpxTaskBase* RetrieveTask (const char* name);
	void DisposeTask (MpxTaskBase* task, bool release);
	int Send (MpxTaskMultiplexer* dst, MpxEventBase* event);
	inline struct timespec GetCurrentTime ()
	{
		return m_ctx->realTime ();
	}
	void* StartTimer (MpxTaskBase* task, struct timespec timer);
	void StopTimer (void* timer);
	inline void Stop ()
	{
		close (m_fd [1]);
		if (m_isWorker == true)
			pthread_cancel (getHandle ());
	}
	void Broadcast (MpxEventBase* event, bool invoke = false);
	inline bool isWorker ()
	{
		return m_isWorker;
	}
	inline void mqImplTask (MpxMQTaskI* mqImplTask)
	{
		m_mqImplTask = mqImplTask;
	}
	inline MpxTaskBase* externalTask ()
	{
		return m_externalTask;
	}
private:
	virtual int InitInstance (void);
	virtual int ExitInstance (void);
	virtual int Run (void);
	void Release ();

	inline static void CleanupMultiplexer (void* arg)
	{
		((MpxTaskMultiplexer*) arg)->_CleanupMultiplexer ();
	}
	void _CleanupMultiplexer ();

	ctx_starter (StartTaskMultiplexer, MpxTaskMultiplexer)
	;ctx_finisher (StopTaskMultiplexer, MpxTaskMultiplexer)
	;ctx_timehook (TimeHookTaskMultiplexer, MpxTaskMultiplexer)
	;

	fd_handler (ReadEnvPipe, MpxTaskMultiplexer)
	;timer_handler (WaitTaskSet, MpxTaskMultiplexer)
	;

private:
	MpxTaskBase* m_externalTask;
	MpxMQTaskI* m_mqImplTask;
	int m_fd [2];
	string m_connStr;
	bool m_isWorker;
	MpxRunningContext* m_ctx;
	taskset m_taskset;
	taskmap m_taskmap;
	ctx_timer_t m_taskTimer;
	ctx_fddes_t m_pipeHandler;
};

} /* namespace mpx */
