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

#include <mpx-sockets/MpxPosixMQ.h>
#include "mpx-events/MpxPosixMQEvent.h"

namespace mpx
{

MpxPosixMQ::MpxPosixMQ (MpxTaskBase* task, bool fast, long int timeOut) :
	m_task (task), m_fast (fast), m_timeOut (timeOut)
{
	m_msgSize = 0;
	m_msg = 0;
	m_mpx = 0;
	m_ctx = 0;
	m_fd = 0;
	m_type = PosixMQInvalidType;

	m_idleTimer = 0;
	m_IOHandle = 0;

	m_chunkTable = 0;
	m_chunkTableSize = 0;
	m_chunkReadIndex = 0;
	m_chunkWriteIndex = 0;

	if (false)
		cout << "POSIX MQ CREATED " << this << endl;
}

MpxPosixMQ::~MpxPosixMQ ()
{
	if (true)
		cout << "POSIX MQ DELETED " << this << endl;
	Release ();
}

void MpxPosixMQ::Release ()
{
	if (m_msg != 0)
		delete [] m_msg;
	m_msg = 0;

	if (m_idleTimer != 0)
		m_ctx->DisableTimer (m_idleTimer);
	m_idleTimer = 0;

	if (m_IOHandle != 0)
		m_ctx->RemoveDescriptor (m_IOHandle);
	m_IOHandle = 0;

	if (m_fd != 0)
		mq_close (m_fd);
	m_fd = 0;

	if (m_type == PosixMQConsumerType)
		mq_unlink (m_mqPath.c_str ());
}

void MpxPosixMQ::StartTimer (timespec t)
{
	if (m_timeOut > 0)
	{
		t.tv_nsec += m_timeOut;
		t.tv_sec += t.tv_nsec / SEC_TO_NSEC;
		t.tv_nsec %= SEC_TO_NSEC;
		m_idleTimer = ((MpxTaskMultiplexer*) m_task->mpx ())->ctx ()->RegisterTimer (t, IdleTimer, this, ctx_info);
	}
	else
		m_idleTimer = 0;
}

int MpxPosixMQ::Create (const char* mqPath, long int msgSize)
{
	if (msgSize <= 0)
		return -1;

	m_mqPath = mqPath;
	m_msgSize = msgSize;
	if ((m_msg = new u_char [m_msgSize]) == 0)
		return -1;

	struct mq_attr attr;
	attr.mq_flags = O_NONBLOCK;
	attr.mq_maxmsg = 32;
	attr.mq_msgsize = m_msgSize;
	attr.mq_curmsgs = 0;

	if ((mq_unlink (mqPath) != 0) && (errno != ENOENT))
		return -1;

	if (m_fd != 0)
		cout << "MQ ALREADY OPENED" << endl;
	if ((m_fd = mq_open (mqPath, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK, 0666, &attr)) < 0)
	{
		cout << "mq_open (create) failed, errno = " << errno << endl;
		return -1;
	}

	if (false)
		cout << "POSIX MQ READER " << mqPath << ", fd = " << m_fd << endl;
	m_type = PosixMQConsumerType;
	return 0;
}

int MpxPosixMQ::Connect (const char* mqPath, long int msgSize)
{
	if (msgSize <= 0)
		return -1;

	m_mqPath = mqPath;
	m_msgSize = msgSize;
	if ((m_msg = new u_char [m_msgSize]) == 0)
		return -1;

	struct mq_attr attr;
	attr.mq_flags = O_NONBLOCK;
	attr.mq_maxmsg = 32;
	attr.mq_msgsize = m_msgSize;
	attr.mq_curmsgs = 0;

	if (m_fd != 0)
		cout << "MQ ALREADY OPENED" << endl;
	if ((m_fd = mq_open (mqPath, O_WRONLY | O_NONBLOCK, 0777, &attr)) < 0)
	{
		cout << "mq_open (connect) failed, errno = " << errno << endl;
		return -1;
	}

	if (false)
		cout << "POSIX MQ WRITER " << mqPath << ", fd = " << m_fd << endl;
	m_type = PosixMQProviderType;
	return 0;
}

int MpxPosixMQ::Start ()
{
	m_mpx = (MpxTaskMultiplexer*) m_task->mpx ();

	if ((m_mpx == 0) || (m_mpx->getTid () != syscall (SYS_gettid)))
		return -1;

	if ((m_ctx = m_mpx->ctx ()) == 0)
		return -1;

	switch (m_type)
	{
	case PosixMQConsumerType:
		m_IOHandle = m_ctx->RegisterDescriptor (EPOLLIN, m_fd, HandleMqConsumerIO, this, ctx_info);
		break;
	case PosixMQProviderType:
		m_IOHandle = m_ctx->RegisterDescriptor (EPOLLOUT, m_fd, HandleMqProviderIO, this, ctx_info);
		m_ctx->DisableDescriptor (m_IOHandle, EPOLLOUT);
		break;
	default:
		return -1;
	}
	StartTimer (m_ctx->realTime ());
	return 0;
}

int MpxPosixMQ::Stop ()
{
	if ((m_mpx == 0) || (m_mpx->getTid () != syscall (SYS_gettid)))
		return -1;

	if (m_IOHandle != 0)
		m_ctx->RemoveDescriptor (m_IOHandle);
	m_IOHandle = 0;
	return 0;
}

void MpxPosixMQ::IdleTimer (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	m_idleTimer = 0;
	Release ();
	m_task->Send (m_task, new MpxPosixMQEvent (this, 0, 0, 0, 0), true);
}

void MpxPosixMQ::HandleMqConsumerIO (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	unsigned int prio = 0;

	if (flags == EPOLLIN)
	{
		while (true)
		{
			int recvSize;
			if ((recvSize = mq_receive (fd, (char*) m_msg, m_msgSize, &prio)) <= 0)
			{
				if (errno == EWOULDBLOCK)
				{
					StartTimer (ctx->realTime ());
					return;
				}
				cout << "mq_receive() failed, errno = " << errno << ", fd = " << fd << endl;
				Release ();
				m_task->Send (m_task, new MpxPosixMQEvent (this, EPOLLIN, errno, 0, 0), true);
				return;
			}

			m_task->Send (m_task, new MpxPosixMQEvent (this, EPOLLIN, 0, m_msgSize, m_msg), true);
		}
	}
	else
	{
		Release ();
		m_task->Send (m_task, new MpxPosixMQEvent (this, EPOLLIN | EPOLLOUT, 0, 0, 0), true);
		return;
	}

	StartTimer (ctx->realTime ());
}

void MpxPosixMQ::HandleMqProviderIO (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	if (flags & EPOLLOUT)
	{
		u_char* msg;
		while ((msg = GetChunk ()) != 0)
		{
			ssize_t count = mq_send (fd, (char*) msg, m_msgSize, 0);
			if (count < 0)
			{
				if (errno == EWOULDBLOCK)
				{
					RestoreChunk ((u_char*) msg);
					StartTimer (ctx->realTime ());
					return;
				}
				delete [] msg;
				cout << "mq_send() failed, errno = " << errno << ", fd = " << fd << endl;
				Release ();
				m_task->Send (m_task, new MpxPosixMQEvent (this, EPOLLOUT, errno, 0, 0), true);
				return;
			}
			delete [] msg;
		}
		ctx->DisableDescriptor (handler, EPOLLOUT);
	}
	else
	{
		Release ();
		m_task->Send (m_task, new MpxPosixMQEvent (this, EPOLLIN | EPOLLOUT, 0, 0, 0), true);
		return;
	}

	StartTimer (ctx->realTime ());
}

int MpxPosixMQ::PostXdrRequest (xdrproc_t proc, void* data)
{
	u_int dummy = 0;
	u_int intSize = xdr_sizeof ((xdrproc_t) xdr_int, &dummy);
	u_int msgSize = xdr_sizeof (proc, data);
	u_int needSpace = intSize + msgSize;

	if (needSpace > m_msgSize)
		return -1;

	char* chunk = new char [needSpace];
	if (chunk == 0)
		return -1;

	XDR xdr;

	xdrmem_create (&xdr, chunk, needSpace, XDR_ENCODE);
	if (xdr_int (&xdr, (int*) &msgSize) != TRUE)
		return -1;

	xdrmem_create (&xdr, chunk + intSize, needSpace - intSize, XDR_ENCODE);
	if (proc (&xdr, data) != TRUE)
		return -1;

	if (PutChunk ((u_char*) chunk) < 0)
	{
		delete [] chunk;
		return -1;
	}

	m_ctx->EnableDescriptor (m_IOHandle, EPOLLOUT);
	return needSpace;
}

int MpxPosixMQ::Write (u_char* buffer, size_t size)
{
	int result = PutChunk (buffer);
	if (result < 0)
		return result;

	MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) m_task->mpx ();
	MpxRunningContext* ctx = mpx->ctx ();
	ctx->EnableDescriptor (m_IOHandle, EPOLLOUT);

	return 0;
}

int MpxPosixMQ::PutChunk (u_char* chunk)
{
	ssize_t usedSpace = m_chunkWriteIndex - m_chunkReadIndex;
	if (usedSpace < 0)
		usedSpace += m_chunkTableSize;

	if (usedSpace + 1 >= m_chunkTableSize)
	{
		ssize_t chunkTableSize = m_chunkTableSize;
		chunkTableSize >>= 5;
		++chunkTableSize;
		chunkTableSize <<= 5;

		u_char** chunkTable = new u_char* [chunkTableSize];
		if (chunkTable == 0)
			return -1;

		for (ssize_t index = 0; index < usedSpace; ++index)
			chunkTable [index] = m_chunkTable [(m_chunkReadIndex + index) % m_chunkTableSize];

		delete [] m_chunkTable;
		m_chunkTable = chunkTable;
		m_chunkTableSize = chunkTableSize;
		m_chunkReadIndex = 0;
		m_chunkWriteIndex = usedSpace;
	}

	ssize_t chunkIndex = m_chunkWriteIndex;
	m_chunkTable [m_chunkWriteIndex++] = chunk;
	m_chunkWriteIndex %= m_chunkTableSize;
	return chunkIndex;
}

u_char* MpxPosixMQ::GetChunk ()
{
	ssize_t usedSpace = m_chunkWriteIndex - m_chunkReadIndex;
	if (usedSpace < 0)
		usedSpace += m_chunkTableSize;
	if (usedSpace == 0)
		return 0;
	u_char* chunk = m_chunkTable [m_chunkReadIndex++];
	m_chunkReadIndex %= m_chunkTableSize;
	return chunk;
}

void MpxPosixMQ::RestoreChunk (u_char* chunk)
{
	m_chunkReadIndex--;
	if (m_chunkReadIndex < 0)
		m_chunkReadIndex += m_chunkTableSize;
	m_chunkTable [m_chunkReadIndex] = chunk;
}

} /* namespace mpx */
