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

MpxTaskMultiplexer::~MpxTaskMultiplexer ()
{
	Release ();
	if (m_ctx != 0)
		delete m_ctx;
	m_ctx = 0;
}

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

int MpxTaskMultiplexer::InitInstance (void)
{
	return
		((m_ctx = new MpxRunningContext (StartTaskMultiplexer, StopTaskMultiplexer, TimeHookTaskMultiplexer, this)) != 0) ?
			0 : -1;
}

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

int MpxTaskMultiplexer::ExitInstance (void)
{
//	Release ();
//	if (m_ctx != 0)
//		delete m_ctx;
//	m_ctx = 0;
	cout << "TASK MULTIPLEXER " << this << " STOPPED" << endl;
	return 0;
}

void MpxTaskMultiplexer::StartTaskMultiplexer (MpxRunningContext *ctx)
{
	MpxEventQueueRepository::RegisterEventQueue (this);
	m_mqImplTask = m_mqImplTask->Copy (this);
	m_mqImplTask->Send (m_mqImplTask, new MpxStartEvent (), true);
	RegisterTask (m_externalTask = new MpxExternalTask (m_connStr.c_str ()));
	if (true != true)
	{
		delete m_mqImplTask;
		m_mqImplTask = 0;
		delete m_externalTask;
		m_externalTask = 0;
		MpxEventQueueRepository::UnregisterEventQueue (this);
		MpxEnvironment::WaitMultiplexerBarrier ();
		Broadcast (new MpxStopEvent (), true);
		MpxEnvironment::WaitTasksBarrier ();
		return;
	}
	m_pipeHandler = m_ctx->RegisterDescriptor (EPOLLIN, m_fd [0], ReadEnvPipe, this);
	m_externalTask->Send (m_externalTask, new MpxStartEvent (), true);
	MpxEnvironment::WaitMultiplexerBarrier ();
	Broadcast (new MpxStartEvent (), true);
	MpxEnvironment::WaitTasksBarrier ();
}

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

void MpxTaskMultiplexer::WaitTaskSet (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	cout << "TASK MULTIPLEXER " << this << " TIMED OUT" << endl;
	Release ();
}

void MpxTaskMultiplexer::StopTaskMultiplexer (MpxRunningContext *ctx)
{
}

void MpxTaskMultiplexer::TimeHookTaskMultiplexer (MpxRunningContext *ctx, struct timespec oldTime,
	struct timespec newTime, long long timeDiff)
{
}

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

MpxTaskBase* MpxTaskMultiplexer::RetrieveTask (const char* name)
{
	taskmap::iterator it = m_taskmap.find (name);
	return (it != m_taskmap.end ()) ? it->second : 0;
}

void MpxTaskMultiplexer::DisposeTask (MpxTaskBase* task, bool release)
{
	m_taskset.erase (task);
	delete task;
	if (!m_taskset.empty ())
		return;
	if (release)
		Release ();
}

int MpxTaskMultiplexer::Send (MpxTaskMultiplexer* dst, MpxEventBase* event)
{
	if (m_ctx == 0)
		return -1;

	pid_t tid = syscall (SYS_gettid);
	if (tid != m_ctx->tid ())
		return -1;

	return m_mqImplTask->MQSend (dst, event);
}

void* MpxTaskMultiplexer::StartTimer (MpxTaskBase* task, struct timespec timerStamp)
{
	if (m_ctx == 0)
		return 0;

	MpxTimerEvent* timerEvent = new MpxTimerEvent (task, timerStamp);
	timerEvent->timer (m_ctx->RegisterTimer (timerStamp, MpxTimerEvent::HandleTimer, timerEvent));
	return (void*) timerEvent;
}

void MpxTaskMultiplexer::StopTimer (void* timer)
{
	if (m_ctx != 0)
		m_ctx->DisableTimer (((MpxTimerEvent*) timer)->timer ());
	delete ((MpxTimerEvent*) timer);
}

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
