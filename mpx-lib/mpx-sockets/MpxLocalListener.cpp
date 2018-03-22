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

#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-events/MpxLocalListenerEvent.h>
#include <mpx-sockets/MpxLocalListener.h>

namespace mpx
{

MpxLocalListener::MpxLocalListener (MpxTaskBase* task, bool seqPacket) :
	m_task (task), m_seqPacket (seqPacket)
{
	m_listenSocket = 0;
	m_handler = 0;
}

MpxLocalListener::~MpxLocalListener ()
{
	if (m_listenSocket != 0)
		close (m_listenSocket);
	m_listenSocket = 0;
	unlink (m_listenPath.c_str ());
	MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (m_task->mpx ());
	if (mpx != 0)
	{
		MpxRunningContext* ctx = mpx->ctx ();
		if (ctx != 0)
		{
			if (m_handler != 0)
				ctx->RemoveDescriptor (m_handler);
			m_handler = 0;
		}
	}
}

int MpxLocalListener::CreateListener (const char* path)
{
	if ((m_listenSocket = socket (PF_LOCAL, (m_seqPacket != true) ? SOCK_STREAM : SOCK_SEQPACKET, 0)) < 0)
		return -1;

	m_listenPath = path;
	unlink (path);

	struct sockaddr_un srvaddr;

	bzero (&srvaddr, sizeof(srvaddr));
	srvaddr.sun_family = AF_LOCAL;
	strncpy (srvaddr.sun_path, path, sizeof(srvaddr.sun_path) - 1);
	if (bind (m_listenSocket, reinterpret_cast <sockaddr*> (&srvaddr), sizeof(srvaddr)) < 0)
	{
		close (m_listenSocket);
		m_listenSocket = -1;
		return -1;
	}

	if (listen (m_listenSocket, SOMAXCONN) < 0)
	{
		close (m_listenSocket);
		m_listenSocket = -1;
		return -1;
	}

	int gflags;

	if ((gflags = fcntl (m_listenSocket, F_GETFL, 0)) < 0)
	{
		close (m_listenSocket);
		m_listenSocket = -1;
		return -1;
	}

	if (fcntl (m_listenSocket, F_SETFL, gflags | O_NONBLOCK) < 0)
	{
		close (m_listenSocket);
		m_listenSocket = -1;
		return -1;
	}

	return 0;
}

int MpxLocalListener::StartListener ()
{
	MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (m_task->mpx ());

	if (mpx->getTid () != syscall (SYS_gettid))
		return -1;

	MpxRunningContext* ctx = mpx->ctx ();
	m_handler = ctx->RegisterDescriptor (EPOLLIN, m_listenSocket, HandleListener, this, ctx_info);
	return 0;
}

int MpxLocalListener::StopListener ()
{
	MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (m_task->mpx ());

	if (mpx->getTid () != syscall (SYS_gettid))
		return -1;

	MpxRunningContext* ctx = mpx->ctx ();
	ctx->RemoveDescriptor (m_handler);
	m_handler = 0;
	return 0;
}

void MpxLocalListener::HandleListener (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	int localClientSocket;
	int gflags;
	struct sockaddr_un addr;
	socklen_t slen = sizeof(struct sockaddr_un);

	memset (&addr, 0, sizeof(struct sockaddr_un));
	if ((localClientSocket = accept (fd, reinterpret_cast <struct sockaddr*> (&addr), &slen)) < 0)
	{
		cout << "ACCEPT LOCAL CLIENT Failed: accept (), errno = " << errno << endl;
		return;
	}

	if ((gflags = fcntl (localClientSocket, F_GETFL, 0)) < 0)
	{
		cout << "ACCEPT LOCAL CLIENT Failed: fcntl (), errno = " << errno << endl;
		close (localClientSocket);
		return;
	}

	if (fcntl (localClientSocket, F_SETFL, gflags | O_NONBLOCK) < 0)
	{
		cout << "ACCEPT LOCAL CLIENT Failed: fcntl (), errno = " << errno << endl;
		close (localClientSocket);
		return;
	}

	m_task->Send (m_task, new MpxLocalListenerEvent (localClientSocket), true);
}

} /* namespace mpx */
