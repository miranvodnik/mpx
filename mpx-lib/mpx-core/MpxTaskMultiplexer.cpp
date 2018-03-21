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

#include <mpx-core/MpxEnvironment.h>
#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-tasks/MpxExternalTask.h>
#include <mpx-event-queues/MpxEventQueueRepository.h>
#include <mpx-events/MpxStartEvent.h>
#include <mpx-events/MpxStopEvent.h>

namespace mpx
{

/*! initialization of task multiplexer
 *
 * task multiplexer is initialized with some important data for its operation:
 * - first of them is special pipe descriptor, which is like a sort of thread
 * on which it hangs. As long as it's solid, the multiplexer lives. When he
 * gives it up, he has to die. Closing one part of this descriptor serves to
 * stop the operation of the multiplexer.
 * - second represents a record in which any number of connection strings are
 * stored. Each of them represents the point to which other applications can
 * be connected, either from the same or another system.
 * - the third is an indicator that affects the role of the multiplexer.
 * The default value ​​of this indicator is false. In this case, this object
 * acts as a task multiplexer. However, if this indicator is true, then this
 * object is the working thread for the invocation of slow working requirements.
 *
 * @param fd pipe descriptor used to hold multiplexer alive
 * @param connStr set of connection strings
 * @param isWorker worker/multiplexer switch
 */
MpxTaskMultiplexer::MpxTaskMultiplexer (int *fd, const char* connStr, bool isWorker) :
	m_connStr ((connStr != 0) ? connStr : ""), m_isWorker (isWorker)
{
	m_externalTask = 0;
	m_mqImplTask = 0;
	m_fd [0] = fd [0];
	m_fd [1] = fd [1];
	m_ctx = 0;
	m_taskTimer = 0;
	m_pipeHandler = 0;
}

/*! task multiplexer destructor
 *
 * it releases all allocated data together with its running context object
 *
 */
MpxTaskMultiplexer::~MpxTaskMultiplexer ()
{
	Release ();
	if (m_ctx != 0)
		delete m_ctx;
	m_ctx = 0;
}

/*! Release all allocated data
 *
 * the release of allocated data consists of the following procedures:
 * - all registered tasks are destroyed
 * - the message queue for event exchange is destroyed,
 * - the timer, which keeps the multiplexer alive for a while, after the
 * pipe has been closed, is removed
 * - pipe handler for pipe which kills multiplexer after its write end
 * has been closed, is removed
 */
void MpxTaskMultiplexer::Release ()
{
	for (taskset::iterator it = m_taskset.begin (); it != m_taskset.end (); ++it)
		delete *it;
	m_taskset.clear ();
	if (m_mqImplTask != 0)
		delete m_mqImplTask;
	m_mqImplTask = 0;
	if (m_taskTimer != 0)
		m_ctx->DisableTimer (m_taskTimer);
	m_taskTimer = 0;
	if (m_pipeHandler != 0)
		m_ctx->RemoveDescriptor (m_pipeHandler);
	m_pipeHandler = 0;
//	m_ctx->Quit ();
}

/*! Initialization of task multiplexer thread
 *
 * the working thread associated with this object needs only one long-lived object,
 * that will be executed in the main function of this thread. This object is m_ctx.
 * This function creates this object. It gives it three functions that it needs them
 * to start operating [StartTaskMultiplexer](@ref StartTaskMultiplexer), to end the
 * operation [StopTaskMultiplexer](@ref StopTaskMultiplexer), and to handle changes
 * in system time [TimeHookTaskMultiplexer](@ref TimeHookTaskMultiplexer). This object
 * is in fact the heart of the multiplexer, since it implements the operation of the
 * multiplexer. It can be used by all functions that respond to events, either
 * directly (lower-level) or indirectly (high-level), such as, for example, sending
 * events between tasks. See also the description of class MpxRunningContext.
 *
 * @return **0** - m_ctx has been created
 * @return **-1** -  m_ctx cannot be created
 */
int MpxTaskMultiplexer::InitInstance (void)
{
	return
		((m_ctx = new MpxRunningContext (StartTaskMultiplexer, StopTaskMultiplexer, TimeHookTaskMultiplexer, this)) != 0) ?
			0 : -1;
}

/*! Main thread function of task multiplexer
 *
 * Implementation of this function is required by interface MpxRunnable. It does
 * not do nothing special but in fact very important task: It starts m_ctx MainLoop
 * function. This function iterates in an infinite loop and handles input output events
 * of all tasks that are located in the multiplexer.
 * @return return status of MainLoop() function invoked by object which implements
 * I/O multiplexing functionality for task multiplexer
 */
int MpxTaskMultiplexer::Run (void)
{
	int status;

	pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, &status);
	pthread_cleanup_push (CleanupMultiplexer, this);
		status = m_ctx->MainLoop ();
		pthread_cleanup_pop(0);

	cout << "THREAD " << getTid () << " TERMINATED" << endl;
	return status;
}

/*! Exit Instance handler
 *
 * Function does nothing. It is implemented only to meet the
 * requirements of the interface MpxRunnable that it implements
 * @return **0** - return code is not important
 */
int MpxTaskMultiplexer::ExitInstance (void)
{
//	Release ();
//	if (m_ctx != 0)
//		delete m_ctx;
//	m_ctx = 0;
	cout << "TASK MULTIPLEXER " << this << " STOPPED" << endl;
	return 0;
}

/*! Starting task multiplexer
 *
 * The function does the following:
 * - it first creates task that allows sending events between task contained by multiplexer
 * and start that task.
 * - It then creates task that handles communication with external tasks and starts it.
 * - next it assigns special function to the pipe associated with multiplexer. This function
 * responds to input output activity of this pipe and is used to determine when multiplexer
 * should terminate. It must terminate when pipe is closed. This event is detected by this
 * function.
 * - When all this is done, it waits until all other multiplexers do the same. This is done
 * by reducing multiplexer barrier by one unit and waiting to descend to zero.
 * - After that all tasks are started.
 * - finally it waits for other multiplexers to start their own tasks. This is done by
 * reducing task barrier by one unit and waiting to descend to zero.
 * @param ctx reference to object implementing I/O multimplexing
 */
void MpxTaskMultiplexer::StartTaskMultiplexer (MpxRunningContext *ctx)
{
	MpxEventQueueRepository::RegisterEventQueue (this);
	m_mqImplTask = m_mqImplTask->Copy (this);
	m_mqImplTask->Send (m_mqImplTask, new MpxStartEvent (), true);
	RegisterTask (m_externalTask = new MpxExternalTask (m_connStr.c_str ()));
	m_externalTask->Send (m_externalTask, new MpxStartEvent (), true);
	m_pipeHandler = m_ctx->RegisterDescriptor (EPOLLIN, m_fd [0], ReadEnvPipe, this);
	MpxEnvironment::WaitMultiplexerBarrier ();
	Broadcast (new MpxStartEvent (), true);
	MpxEnvironment::WaitTasksBarrier ();
}

/*! handling input output events on a pipe associated with multiplexer.
 *
 * pipe associated with multiplexer is used to terminate processing of this
 * multiplexer:
 * - if multiplexer contains no tasks it is released immediately
 * - otherwise a low level timer is started for a short time, which will force
 * termination if tasks will not terminate before this timer expired
 * - stop event is sent to every task owned by multiplexer
 * - message queue used to exchange events between tasks is destroyed.
 *
 * @param ctx reference to object implementing I/O multiplexing
 * @param flags flags EPOLLIN or EPOLLOUT
 * @param handler reference to object representing this callback routine
 * @param fd file descriptor of pipe end being affected in I/O
 */
void MpxTaskMultiplexer::ReadEnvPipe (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	close (fd);
	m_ctx->RemoveDescriptor (m_pipeHandler);
	m_pipeHandler = 0;

	if (m_taskset.empty ())
		Release ();
	else
	{
		timespec t = m_ctx->realTime ();
		size_t size = m_taskset.size ();
		if (size < 1000)
			size = 1000;
		t.tv_nsec += size * (SEC_TO_NSEC / 1000);
		t.tv_sec += t.tv_nsec / SEC_TO_NSEC;
		t.tv_nsec %= SEC_TO_NSEC;
		m_taskTimer = m_ctx->RegisterTimer (t, WaitTaskSet, this, 0);

		delete m_mqImplTask;
		m_mqImplTask = 0;

		Broadcast (new MpxStopEvent (), true);
//		m_mqImplTask->Send (m_mqImplTask, new MpxStopEvent (), false);
	}
}

/*! time-out handler for timer started in function ReadEnvPipe
 *
 * if timer expires, multiplexer is destroyed regardless if there is some
 * unterminated task.
 *
 * @param ctx reference to object implementing multiplexer
 * @param handler reference to object representing this timer callback
 * @param t time stamp of timer expiration
 */
void MpxTaskMultiplexer::WaitTaskSet (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	cout << "TASK MULTIPLEXER " << this << " TIMED OUT" << endl;
	Release ();
}

/*! Function called at the end of multiplexer activity
 *
 * it does nothing. It is required by class implementing I/O multiplexing
 * MpxRunningContext, but it should also be passed as null pointer.
 *
 * @param ctx reference to object implementing I/O multiplexing
 */
void MpxTaskMultiplexer::StopTaskMultiplexer (MpxRunningContext *ctx)
{
}

/*! Function called when time changes significantly
 *
 * This function is too, like StartMultiplexer and StopMultiplexer, required by
 * class implementing I/O multiplexing. Time changes are not important for this
 * library, that is why this function does nothing and it should also be
 * represented with null pointer
 *
 * @param ctx reference to object implementing I/O multiplexing
 * @param oldTime time stamp befor time changed
 * @param newTime time stamp just after time changed
 * @param timeDiff difference between time stamps expressed in nanoseconds
 */
void MpxTaskMultiplexer::TimeHookTaskMultiplexer (MpxRunningContext *ctx, struct timespec oldTime,
	struct timespec newTime, long long timeDiff)
{
}

/*! Register task in task multiplexer
 *
 * Function does the following:
 * - if task is already register within some multiplexer, then it cannot be
 * registered again
 * - otherwise it is inserted in the set m_taskset
 * - if task has a name it is also registered within the set of named tasks.
 * This set is used in task retrieval proces when an external entity wants
 * to communicate with that task.
 * @param task reference to object representing task which should be registered
 * @return **0** - task has been registered
 * @return **-1** - task has already been register within this or another task multiplexer
 */
int MpxTaskMultiplexer::RegisterTask (MpxTaskBase* task)
{
	if (task->mpx () != 0)
		return -1;
	if (m_taskset.find (task) != m_taskset.end ())
		return -1;
	task->mpx (this);
	m_taskset.insert (task);
	const char* name = task->name ();
	if (*name != 0)
		m_taskmap [name] = task;
	return 0;
}

/*! Retrieve task
 *
 * This function retrieves task using its name
 *
 * @param name task name
 * @return **0** - task with that name does not exist in set of registered
 * tasks of this multiplexer
 * @return **other** - named task is registered within this task multiplexer
 * function returns its reference
 */
MpxTaskBase* MpxTaskMultiplexer::RetrieveTask (const char* name)
{
	taskmap::iterator it = m_taskmap.find (name);
	return (it != m_taskmap.end ()) ? it->second : 0;
}

/*! Remove task from the set of registered task
 *
 * Task is removed from the set of registered tasks ans deleted. If required
 * task multiplexer is released too if the set of registered tasks becomes
 * empty
 *
 * @param task erference of task being removed from the set of registered tasks
 * @param release if true, multiplexer should be deleted when set of registered
 * tasks becomes empty
 */
void MpxTaskMultiplexer::DisposeTask (MpxTaskBase* task, bool release)
{
	m_taskset.erase (task);
	delete task;
	if (!m_taskset.empty ())
		return;
	if (release)
		Release ();
}

/*! Send event to another (or to self) multiplexer
 *
 * function uses its message queue task to send arbitrary event to arbitrary
 * multiplexer (also to self). It must be sent from the owner thread of multiplexer
 * otherwise it will fail
 *
 * @param dst destination multiplexer reference
 * @param event event object reference
 * @return result of function MQSend() which is used by object implementing waiting queue
 */
int MpxTaskMultiplexer::Send (MpxTaskMultiplexer* dst, MpxEventBase* event)
{
	if (m_ctx == 0)
		return -1;

	pid_t tid = syscall (SYS_gettid);
	if (tid != m_ctx->tid ())
		return -1;

	return m_mqImplTask->MQSend (dst, event);
}

/*! Start high level timer
 *
 * function starts timer using object implementing I/O multiplexing functionality
 * MpxRunningContext. Timer cannot be started from another thread of execution (from
 * another task multiplexer for example). This function creates timer event which will
 * be triggered when timer expires. Reference to this event is also returned by this
 * function. This reference becomes invalid as soon as timer expires. In fact, it is
 * still valid in timer event handler associated with timer expiration. Anytime before
 * that it should be used to prevent expiration of timer (to stop it), when its reference
 * also becomes invalid.
 *
 * @param task task associated with timer and which will receive timer event created
 * by this function
 * @param timerStamp time stamp of expiration time
 * @return **0** - multiplexer is not created but not running yet
 * @return **non-zero** - pointer to timer event object which will be reported to associated
 * task when this timer expires
 */
void* MpxTaskMultiplexer::StartTimer (MpxTaskBase* task, struct timespec timerStamp, void* ctx)
{
	if (m_ctx == 0)
		return 0;

	pid_t tid = syscall (SYS_gettid);
	if (tid != m_ctx->tid ())
		return 0;

	MpxTimerEvent* timerEvent = new MpxTimerEvent (task, timerStamp, ctx);
	timerEvent->timer (m_ctx->RegisterTimer (timerStamp, MpxTimerEvent::HandleTimer, timerEvent));
	return reinterpret_cast <void*> (timerEvent);
}

/*! Stop high level timer
 *
 * function must be used before associated timer expires, otherwise the reference
 * contained in parameter timer addresses invalid object. This reference also becomes
 * invalid after completion of this function, since this function destrojes object
 * addressed by this reference.
 *
 * @param timer reference to timer event created by function StartTimer() which starts
 * timer associated with it
 */
void MpxTaskMultiplexer::StopTimer (void* timer)
{
	if (m_ctx != 0)
		m_ctx->DisableTimer (((MpxTimerEvent*) timer)->timer ());
	delete (reinterpret_cast <MpxTimerEvent*> (timer));
}

/*! Broadcast arbitrary event
 *
 * broadcast event to every task owned by task multiplexer. Every task is delivered
 * copy of this event. Events being broadcasted must carefully implement Copy() function
 * required by interface MpxEventBase, so that the copy of event will be
 * equivalent to original.
 *
 * @param event reference to event which will be broadcasted
 * @param invoke indicator used by function Send. If true, every task will handle event in
 * current iteration of I/O multiplexer. Otherwise, events will be posted and handled in
 * next iteration of I/O multiplexer
 */
void MpxTaskMultiplexer::Broadcast (MpxEventBase* event, bool invoke)
{
	taskset ts = m_taskset;
	for (taskset::iterator it = ts.begin (); it != ts.end (); ++it)
		(*it)->Send (*it, event->Copy (), invoke);
	delete event;
}

void MpxTaskMultiplexer::_CleanupMultiplexer ()
{
	Release ();
}

} /* namespace mpx */
