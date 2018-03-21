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

/*! Task Multiplexer
 *
 * This class represents a kind of environment in which there can be an
 * arbitrary number of tasks that work by exchanging messages or events
 * between them. The exchange of these events can take place within this
 * environment, and may also occur between tasks that are located in
 * different environments and even between tasks in different processes
 * or in different machines. The main goal of this environment is to enable
 * the smooth flow of these messages between different tasks regardless of
 * where they are located.
 *
 * This functionality is implemented with methods exposed by this class
 * and with data structures it contains.
 *
 * The following data structures are defined in the class:
 * - the most important among them is the set of tasks that are represented
 * by instances of the class MpxTaskBase and from the subclasses resulting
 * from it.
 * - every multiplexer contains an implementation of message queue which
 * exchange events between different tasks.
 * - every multiplexer contains an 'external task', implementation of
 * MpxExternalTask class, which is responsible for event exchange between
 * tasks that are located in different processes on the same or other
 * system in the network.
 * - another important data is an object that implements the multiplexing
 * of the input output units. This facility also includes time controls and
 * functions for processing the input output data
 * - connnection strings are important for linking external tasks with tasks
 * contained in a given multiplexer. They are used by 'external task'.
 * With their help, they build connection points through which external
 * tasks connect to the system.
 * - very important data is also pipe descriptor which is constructed
 * at the beginning together with the multiplexer. It is only used at the
 * end when it is necessary to terminate the operation of the multiplexer.
 * Its operation is terminated in such a way that it simply closes one side
 * of the pipe. When the multiplexer detects it, it responds with activities
 * that cause all the tasks contained in the multiplexer to be completed
 * within the reasonable time. This also terminates the operation of the
 * multiplexer itself.
 */
class MpxTaskMultiplexer: public mpx::MpxRunnable
{

	typedef set <MpxTaskBase*> taskset;
	typedef map <string, MpxTaskBase*> taskmap;

public:
	MpxTaskMultiplexer (int fd [], const char* connStr = 0, bool isWorker = false);
	virtual ~MpxTaskMultiplexer ();
	int RegisterTask (MpxTaskBase* task);
	MpxTaskBase* RetrieveTask (const char* name);
	void DisposeTask (MpxTaskBase* task, bool release);
	int Send (MpxTaskMultiplexer* dst, MpxEventBase* event);
	void* StartTimer (MpxTaskBase* task, struct timespec timer, void* ctx = 0);
	void StopTimer (void* timer);
	void Broadcast (MpxEventBase* event, bool invoke = false);

	/*! Initiate termination procedure
	 *
	 * function closes write end of pipe associated with task multiplexer
	 * causing it to initiate termination activity. Working threads, which
	 * are in fact not real task multiplexers, are delivered cleanup request,
	 * since they can be stuck in performing heavy and long-lasting jobs,
	 * which cannot be terminated in reasonable time unless they are canceled.
	 */
	inline void Stop ()
	{
		close (m_fd [1]);
		if (m_isWorker == true)
			pthread_cancel (getHandle ());
	}

	/*! I/O multiplexer associated with task multiplexer
	 *
	 * @return I/O multiplexer associated with task multiplexer
	 */
	inline MpxRunningContext* ctx ()
	{
		return m_ctx;
	}

	/*! real time stamp
	 *
	 * @return real time stamp of I/O multiplexer
	 */
	inline struct timespec GetCurrentTime ()
	{
		return m_ctx->realTime ();
	}

	/*! worker thread indicator
	 *
	 * @return **true** - task multiplexer is in fact working thread performing heavy
	 * and long lasting jobs
	 * @return **false** - true task multiplexer
	 */
	inline bool isWorker ()
	{
		return m_isWorker;
	}

	/*! set reference to waiting queue object
	 *
	 * @param mqImplTask reference to waiting queue task, which will be used by task
	 * multiplexer to distribute events between tasks
	 */
	inline void mqImplTask (MpxMQTaskI* mqImplTask)
	{
		m_mqImplTask = mqImplTask;
	}

	/*! 'external task' reference
	 *
	 * @return reference to task which implements communication (event delivery)
	 * between external tasks (other processes and machines)
	 */
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
		(reinterpret_cast <MpxTaskMultiplexer*> (arg))->_CleanupMultiplexer ();
	}
	void _CleanupMultiplexer ();

	ctx_starter (StartTaskMultiplexer, MpxTaskMultiplexer)
	ctx_finisher (StopTaskMultiplexer, MpxTaskMultiplexer)
	ctx_timehook (TimeHookTaskMultiplexer, MpxTaskMultiplexer)

	fd_handler (ReadEnvPipe, MpxTaskMultiplexer)
	timer_handler (WaitTaskSet, MpxTaskMultiplexer)

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
