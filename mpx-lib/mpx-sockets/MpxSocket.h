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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <mpx-core/MpxRunningContext.h>
#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-core/MpxXDRProcRegistry.h>
#include <mpx-tasks/MpxTaskBase.h>

#include "mpx-core/MpxUtilities.h"

namespace mpx
{

template <typename T> class MpxSocket
{
public:
	typedef T MpxSocketEvent;
	MpxSocket (MpxTaskBase* task, bool fast = false, long int timeOut = 1000 * 1000 * 1000, bool seqPacket = false) :
		m_task (task), m_fast (fast), m_timeOut (timeOut), m_seqPacket (seqPacket)
	{
		MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) m_task->mpx ();
		if (mpx->getTid () != syscall (SYS_gettid))
			return;

		m_sent = 0;
		m_rcvd = 0;

		m_ctx = mpx->ctx ();

		m_idleTimer = 0;
		StartTimer (m_ctx->realTime ());
		m_IOHandle = 0;

		m_readBuffer = 0;
		m_readBufferUse = 0;
		m_readBufferPtr = 0;
		m_readBufferEnd = 0;

		m_writeBuffer = 0;
		m_writeBufferPtr = 0;
		m_writeBufferEnd = 0;
	}

	virtual ~MpxSocket ()
	{
		Release ();
	}

	inline size_t size ()
	{
		return m_readBufferPtr - m_readBufferUse;
	}

	u_char* FastRead (size_t size)
	{
		if (size > this->size ())
			return 0;
		u_char* ptr = m_readBufferUse;
		m_readBufferUse += size;
		return ptr;
	}

	int Read (u_char* buffer, size_t size)
	{
		if (size > this->size ())
			size = this->size ();
		if (size < 0)
			size = 0;
		MpxUtilities::memcpy (buffer, m_readBufferUse, size);
		m_readBufferUse += size;
		return size;
	}

	int Write (u_char* buffer, size_t size)
	{
		if ((buffer == 0) || (size < 0))
			return -1;

		size_t freeSpace = m_writeBufferEnd - m_writeBufferPtr;
		if (freeSpace < size)
		{
			size_t usedSpace = m_writeBufferPtr - m_writeBuffer;
			size_t needSpace = usedSpace + size;
			needSpace >>= 10;
			needSpace += 1;
			needSpace <<= 10;
			u_char* tmpb = new u_char [needSpace];
			if (tmpb == 0)
				return -1;
			MpxUtilities::memcpy (tmpb, m_writeBuffer, usedSpace);
			if (m_writeBuffer != 0)
				delete [] m_writeBuffer;
			m_writeBuffer = tmpb;
			m_writeBufferPtr = m_writeBuffer + usedSpace;
			m_writeBufferEnd = m_writeBuffer + needSpace;
		}

		MpxUtilities::memcpy (m_writeBufferPtr, buffer, size);
		m_writeBufferPtr += size;

		m_ctx->EnableDescriptor (m_IOHandle, EPOLLOUT);
		return size;
	}

	int PostXdrRequest (xdrproc_t proc, void* data)
	{
		u_int dummy = 0;
		u_int intSize = xdr_sizeof ((xdrproc_t) xdr_int, &dummy);
		u_int msgSize = xdr_sizeof (proc, data);
		u_int freeSpace = m_writeBufferEnd - m_writeBufferPtr;

		if (freeSpace < msgSize + intSize)
		{
			u_int usedSpace = m_writeBufferPtr - m_writeBuffer;
			u_int needSpace = usedSpace + msgSize + intSize;
			needSpace >>= 10;
			needSpace += 1;
			needSpace <<= 10;
			u_char* tmpb = new u_char [needSpace];
			if (tmpb == 0)
				return -1;
			MpxUtilities::memcpy (tmpb, m_writeBuffer, usedSpace);
			if (m_writeBuffer != 0)
				delete [] m_writeBuffer;
			m_writeBuffer = tmpb;
			m_writeBufferPtr = m_writeBuffer + usedSpace;
			m_writeBufferEnd = m_writeBuffer + needSpace;
			freeSpace = needSpace - usedSpace;
		}

		XDR xdr;

		xdrmem_create (&xdr, (char*) m_writeBufferPtr, freeSpace, XDR_ENCODE);
		if (xdr_int (&xdr, (int*) &msgSize) != TRUE)
			return -1;

		xdrmem_create (&xdr, (char*) (m_writeBufferPtr + intSize), freeSpace - intSize, XDR_ENCODE);
		if (proc (&xdr, data) != TRUE)
			return -1;

		u_int pos = xdr_getpos(&xdr);
		m_writeBufferPtr += pos + intSize;

		m_ctx->EnableDescriptor (m_IOHandle, EPOLLOUT);
		return msgSize + intSize;
	}

	int ReadXdrRequest (xdrproc_t proc, void* data)
	{
		u_int dummy = 0;
		u_int intSize = xdr_sizeof ((xdrproc_t) xdr_int, &dummy);
		u_int usedSpace = m_readBufferPtr - m_readBufferUse;

		if (usedSpace < intSize)
			return -1;
		XDR xdr;
		u_int msgSize;
		xdrmem_create (&xdr, (char*) m_readBufferUse, intSize, XDR_DECODE);
		if (xdr_int (&xdr, (int*) &msgSize) != TRUE)
			return -1;
		if (usedSpace < msgSize + intSize)
			return -1;
		xdrmem_create (&xdr, (char*) (m_readBufferUse + intSize), msgSize, XDR_DECODE);
		if (proc (&xdr, data) != TRUE)
			return -1;
		m_readBufferUse += intSize + msgSize;
		return 0;
	}

	inline MpxTaskBase* task ()
	{
		return m_task;
	}
	inline void task (MpxTaskBase* task)
	{
		m_task = task;
	}
	inline bool fast ()
	{
		return m_fast;
	}

	inline u_int sent ()
	{
		return m_sent;
	}

	inline u_int rcvd ()
	{
		return m_rcvd;
	}

	inline u_int readlost ()
	{
		return m_readBufferPtr - m_readBuffer;
	}

	inline u_int writelost ()
	{
		return m_writeBufferPtr - m_writeBuffer;
	}

protected:
	virtual void Release ()
	{
		if (m_idleTimer != 0)
			m_ctx->DisableTimer (m_idleTimer);
		m_idleTimer = 0;

		if (m_IOHandle != 0)
			m_ctx->RemoveDescriptor (m_IOHandle);
		m_IOHandle = 0;

		if (m_readBuffer != 0)
			delete [] m_readBuffer;
		m_readBuffer = 0;
		m_readBufferUse = 0;
		m_readBufferPtr = 0;
		m_readBufferEnd = 0;

		if (m_writeBuffer != 0)
			delete [] m_writeBuffer;
		m_writeBuffer = 0;
		m_writeBufferPtr = 0;
		m_writeBufferEnd = 0;
	}

	void StartTimer (timespec t)
	{
		m_ctx->DisableTimer (m_idleTimer);

		if (m_timeOut > 0)
		{
			t.tv_nsec += m_timeOut;
			t.tv_sec += t.tv_nsec / SEC_TO_NSEC;
			t.tv_nsec %= SEC_TO_NSEC;
			m_idleTimer = m_ctx->RegisterTimer (t, IdleTimer, this, ctx_info);
		}
		else
			m_idleTimer = 0;
	}

	inline static void IdleTimer (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t, ctx_appdt_t appdata)
	{
		((MpxSocket*) appdata)->IdleTimer (ctx, handler, t);
	}

	void IdleTimer (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
	{
		m_idleTimer = 0;
		Release ();
		m_task->Send (m_task, new MpxSocketEvent (this, 0, 0, 0, 0), true);
	}

	inline static void HandleSocketIO (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd,
		ctx_appdt_t appdata)
	{
		((MpxSocket*) appdata)->HandleSocketIO (ctx, flags, handler, fd);
	}

	void HandleSocketIO (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
	{
		ctx->DisableTimer (m_idleTimer);
		m_idleTimer = 0;

		if (flags & EPOLLIN)
		{
			if (m_fast)
			{
				if (m_readBufferUse > m_readBuffer)
				{
					u_char* src;
					u_char* dst;
					for (src = m_readBufferUse, dst = m_readBuffer; src < m_readBufferPtr;)
						*dst++ = *src++;
					m_readBufferPtr -= (m_readBufferUse - m_readBuffer);
					m_readBufferUse = m_readBuffer;
				}

				int needSpace = 0;
				int usedSpace = m_readBufferPtr - m_readBuffer;
				int freeSpace = m_readBufferEnd - m_readBufferPtr;

				if (ioctl (fd, FIONREAD, &needSpace) < 0)
				{
					Release ();
					m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, errno, 0, 0), true);
					return;
				}

				if (needSpace == 0)
				{
					Release ();
					m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, 0, 0, 0), true);
					return;
				}

				if (freeSpace < needSpace)
				{
					needSpace += usedSpace;
					needSpace >>= 10;
					needSpace++;
					needSpace <<= 10;
					u_char* buffer = new u_char [needSpace];
					if (buffer == 0)
					{
						Release ();
						m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, errno, 0, 0), true);
						return;
					}
					if (m_readBuffer != 0)
					{
						MpxUtilities::memcpy (buffer, m_readBuffer, usedSpace);
						delete [] m_readBuffer;
					}
					m_readBuffer = buffer;
					m_readBufferUse = buffer;
					m_readBufferPtr = buffer + usedSpace;
					m_readBufferEnd = buffer + needSpace;
					freeSpace = needSpace - usedSpace;
				}

				int recvSize;
				if ((recvSize = recv (fd, m_readBufferPtr, needSpace, 0)) <= 0)
				{
					if (errno == EWOULDBLOCK)
					{
						StartTimer (ctx->realTime ());
						return;
					}
					Release ();
					m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, errno, 0, 0), true);
					return;
				}

				m_rcvd += recvSize;
				m_readBufferPtr += recvSize;

				m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, 0, recvSize, 0), true);
			}
			else
			{
				int needSpace = 0;

				if (ioctl (fd, FIONREAD, &needSpace) < 0)
				{
					Release ();
					m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, errno, 0, 0), true);
					return;
				}

				if (needSpace == 0)
				{
					Release ();
					m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, 0, 0, 0), true);
					return;
				}

				u_char* buffer = new u_char [needSpace];
				if (buffer == 0)
				{
					Release ();
					m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, errno, 0, 0), true);
					return;
				}

				int recvSize;
				if ((recvSize = recv (fd, buffer, needSpace, 0)) <= 0)
				{
					delete [] buffer;
					if (errno == EWOULDBLOCK)
					{
						StartTimer (ctx->realTime ());
						return;
					}
					Release ();
					m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, errno, 0, 0), true);
					return;
				}

				m_rcvd += recvSize;

				m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN, 0, recvSize, buffer), true);
				delete [] buffer;
			}
		}
		else if (flags & EPOLLOUT)
		{
			size_t size = m_writeBufferPtr - m_writeBuffer;
			ssize_t count = send (fd, m_writeBuffer, size, 0);
			if (count <= 0)
			{
				Release ();
				m_task->Send (m_task, new MpxSocketEvent (this, EPOLLOUT, errno, 0, 0), true);
				return;
			}
			m_sent += count;
			m_task->Send (m_task, new MpxSocketEvent (this, EPOLLOUT, 0, count, 0), true);
			if ((size_t) count < size)
				MpxUtilities::memcpy (m_writeBuffer, m_writeBuffer + count, size - count);
			else
				ctx->DisableDescriptor (handler, EPOLLOUT);
			m_writeBufferPtr -= count;
		}
		else
		{
			Release ();
			m_task->Send (m_task, new MpxSocketEvent (this, EPOLLIN | EPOLLOUT, 0, 0, 0), true);
			return;
		}

		StartTimer (ctx->realTime ());
	}

protected:
	MpxTaskBase* m_task;
	bool m_fast;
	long int m_timeOut;
	bool m_seqPacket;

	u_int m_sent;
	u_int m_rcvd;

	MpxRunningContext* m_ctx;

	ctx_timer_t m_idleTimer;
	ctx_fddes_t m_IOHandle;

	u_char* m_readBuffer;
	u_char* m_readBufferUse;
	u_char* m_readBufferPtr;
	u_char* m_readBufferEnd;

	u_char* m_writeBuffer;
	u_char* m_writeBufferPtr;
	u_char* m_writeBufferEnd;
};

} // namespace mpx
