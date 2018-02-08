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
#include <mpx-sockets/MpxTcp6Listener.h>
#include <mpx-events/MpxTcp6ListenerEvent.h>

namespace mpx
{

MpxTcp6Listener::MpxTcp6Listener (MpxTaskBase* task, bool seqPacket) :
	m_task (task), m_seqPacket (seqPacket)
{
	m_listenPort = 0;
	m_listenSocket = 0;
	m_handler = 0;
}

MpxTcp6Listener::~MpxTcp6Listener ()
{
	if (m_listenSocket != 0)
		close (m_listenSocket);
	m_listenSocket = 0;
	if (m_handler != 0)
		((MpxTaskMultiplexer*) m_task->mpx ())->ctx ()->RemoveDescriptor (m_handler);
	m_handler = 0;
}

int MpxTcp6Listener::CreateListener (in_port_t listenPort)
{
	if (false)
		cout << "CREATING TCP LISTENER" << endl;
	if ((m_listenSocket = socket (AF_INET6, (m_seqPacket != true) ? SOCK_STREAM : SOCK_SEQPACKET, IPPROTO_TCP)) < 0)
		return -1;

	int reuse = 1;
	setsockopt (m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const void*) &reuse, sizeof(int));
	int one = 1;
	setsockopt (m_listenSocket, SOL_IPV6, IPV6_V6ONLY, &one, sizeof(one));

	struct sockaddr_in6 srvaddr;

	bzero (&srvaddr, sizeof(srvaddr));
	bzero (&srvaddr, sizeof(srvaddr));
	srvaddr.sin6_family = AF_INET6;
	srvaddr.sin6_port = (listenPort <= 0) ? 0 : htons (listenPort);
	srvaddr.sin6_addr = in6addr_any;
	if (bind (m_listenSocket, (sockaddr*) &srvaddr, sizeof(srvaddr)) < 0)
	{
		close (m_listenSocket);
		m_listenSocket = -1;
		return -1;
	}

	socklen_t slen = sizeof(struct sockaddr_in6);
	memset (&srvaddr, 0, sizeof(struct sockaddr_in6));
	getsockname (m_listenSocket, (struct sockaddr*) &srvaddr, &slen);
	m_listenPort = ntohs (srvaddr.sin6_port);

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

	if (false)
		cout << "CREATED TCP LISTENER" << endl;
	return 0;
}

int MpxTcp6Listener::CreateListener (const char* localAddress, in_port_t listenPort)
{
	if (false)
		cout << "CREATING TCP LISTENER" << endl;

	struct sockaddr_in6 srvaddr;

	bzero (&srvaddr, sizeof(srvaddr));
	if (inet_pton (AF_INET6, localAddress, &srvaddr.sin6_addr) <= 0)
		return -1;

	if ((m_listenSocket = socket (AF_INET6, (m_seqPacket != true) ? SOCK_STREAM : SOCK_SEQPACKET, IPPROTO_TCP)) < 0)
		return -1;

	int reuse = 1;
	setsockopt (m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const void*) &reuse, sizeof(int));
	int one = 1;
	setsockopt (m_listenSocket, SOL_IPV6, IPV6_V6ONLY, &one, sizeof(one));

	srvaddr.sin6_family = AF_INET6;
	srvaddr.sin6_port = (listenPort <= 0) ? 0 : htons (listenPort);
	if (bind (m_listenSocket, (sockaddr*) &srvaddr, sizeof(srvaddr)) < 0)
	{
		close (m_listenSocket);
		m_listenSocket = -1;
		return -1;
	}

	socklen_t slen = sizeof(struct sockaddr_in6);
	memset (&srvaddr, 0, sizeof(struct sockaddr_in6));
	getsockname (m_listenSocket, (struct sockaddr*) &srvaddr, &slen);
	m_listenPort = ntohs (srvaddr.sin6_port);

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

	if (false)
		cout << "CREATED TCP LISTENER" << endl;
	return 0;
}

int MpxTcp6Listener::CreateListener (struct sockaddr* localAddress, in_port_t listenPort)
{
	if (false)
		cout << "CREATING TCP V6 LISTENER" << endl;

	struct sockaddr_in6 srvaddr = *((struct sockaddr_in6*) localAddress);

	if ((m_listenSocket = socket (AF_INET6, (m_seqPacket != true) ? SOCK_STREAM : SOCK_SEQPACKET, IPPROTO_TCP)) < 0)
		return -1;

	int reuse = 1;
	setsockopt (m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const void*) &reuse, sizeof(int));
	int one = 1;
	setsockopt (m_listenSocket, SOL_IPV6, IPV6_V6ONLY, &one, sizeof(one));

	srvaddr.sin6_port = (listenPort <= 0) ? 0 : htons (listenPort);
	if (bind (m_listenSocket, (sockaddr*) &srvaddr, sizeof(srvaddr)) < 0)
	{
		close (m_listenSocket);
		m_listenSocket = -1;
		return -1;
	}

	socklen_t slen = sizeof(struct sockaddr_in6);
	memset (&srvaddr, 0, sizeof(struct sockaddr_in6));
	getsockname (m_listenSocket, (struct sockaddr*) &srvaddr, &slen);
	m_listenPort = ntohs (srvaddr.sin6_port);

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

	if (false)
		cout << "CREATED TCP LISTENER" << endl;
	return 0;
}

int MpxTcp6Listener::StartListener ()
{
	MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) m_task->mpx ();

	if (false)
		cout << "STARTING TCP LISTENER" << endl;
	if (mpx->getTid () != syscall (SYS_gettid))
		return -1;

	if (false)
		cout << "STARTED TCP LISTENER" << endl;
	MpxRunningContext* ctx = mpx->ctx ();
	m_handler = ctx->RegisterDescriptor (EPOLLIN, m_listenSocket, HandleListener, this, ctx_info);
	return 0;
}

int MpxTcp6Listener::StopListener ()
{
	MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) m_task->mpx ();

	if (mpx->getTid () != syscall (SYS_gettid))
		return -1;

	MpxRunningContext* ctx = mpx->ctx ();
	ctx->RemoveDescriptor (m_handler);
	m_handler = 0;
	return 0;
}

void MpxTcp6Listener::HandleListener (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	int localClientSocket;
	int gflags;
	struct sockaddr_in6 addr;
	socklen_t slen = sizeof(struct sockaddr_in6);

	if (false)
		cout << "CREATING TCP END POINT" << endl;
	memset (&addr, 0, sizeof(struct sockaddr_in6));
	if ((localClientSocket = accept (fd, (struct sockaddr*) &addr, &slen)) < 0)
	{
		return;
	}

	if ((gflags = fcntl (localClientSocket, F_GETFL, 0)) < 0)
	{
		close (localClientSocket);
		return;
	}

	if (fcntl (localClientSocket, F_SETFL, gflags | O_NONBLOCK) < 0)
	{
		close (localClientSocket);
		return;
	}

	if (false)
		cout << "CREATED TCP END POINT" << endl;
	m_task->Send (m_task, new MpxTcp6ListenerEvent (localClientSocket));
}

} /* namespace mpx */
