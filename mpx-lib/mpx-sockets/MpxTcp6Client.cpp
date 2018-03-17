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
#include <mpx-sockets/MpxTcp6Client.h>

namespace mpx
{

MpxTcp6Client::MpxTcp6Client (MpxTaskBase* task, bool fast, long int timeOut, bool seqPacket) :
	MpxSocket (task, fast, timeOut, seqPacket)
{
	MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) m_task->mpx ();
	if (mpx->getTid () != syscall (SYS_gettid))
		return;

	m_port = 0;
	m_endPoint = 0;
}

MpxTcp6Client::~MpxTcp6Client ()
{
	Release ();
}

void MpxTcp6Client::Release ()
{
	if (m_endPoint != 0)
		close (m_endPoint);
	m_endPoint = 0;

	MpxSocket::Release ();
}

int MpxTcp6Client::Connect (const char* hostname, uint16_t port)
{
	if (m_IOHandle != 0)
		return 0;

	m_hostname = hostname;
	m_endPoint = socket (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (m_endPoint < 0)
	{
		cout << "socket() failed, errno = " << errno << endl;
		return -1;
	}

	int flags = fcntl (m_endPoint, F_GETFL, 0);
	if (flags < 0)
	{
		cout << "fcntl() failed, errno = " << errno << endl;
		return -1;
	}

	if (fcntl (m_endPoint, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		cout << "fcntl() failed, errno = " << errno << endl;
		return -1;
	}

	addrinfo hint;
	memset (&hint, 0, sizeof(addrinfo));
	hint.ai_flags = AI_ALL;
	hint.ai_family = AF_INET6;
	addrinfo* res = 0;
	getaddrinfo (hostname, 0, &hint, &res);

	addrinfo* addrPtr;
	for (addrPtr = res; (addrPtr != 0) && (addrPtr->ai_protocol != IPPROTO_TCP); addrPtr = addrPtr->ai_next)
		;

	if (addrPtr == 0)
	{
		cout << "addrinfo() failed, no info" << endl;
		return -1;
	}

	struct sockaddr_in6 addr = *((struct sockaddr_in6*) (addrPtr->ai_addr));
	addr.sin6_port = htons (port);

	freeaddrinfo (res);

	MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) m_task->mpx ();
	MpxRunningContext* ctx = mpx->ctx ();

	int status;
	if ((status = connect (m_endPoint, (struct sockaddr*) &addr, sizeof(struct sockaddr_in6))) < 0)
	{
		if ((errno != EINPROGRESS) && (errno != EAGAIN))
		{
			cout << "connect() failed, errno = " << errno << endl;
			StartTimer (ctx->realTime ());
			return -1;
		}
	}

	m_IOHandle = ctx->RegisterDescriptor (EPOLLIN, m_endPoint, HandleSocketIO, this, ctx_info);

	StartTimer (ctx->realTime ());
	return 0;
}

int MpxTcp6Client::Connect (const sockaddr_in6* addr, uint16_t port)
{
	if (m_IOHandle != 0)
		return 0;

	m_endPoint = socket (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (m_endPoint < 0)
	{
		cout << "socket() failed, errno = " << errno << endl;
		return -1;
	}

	int flags = fcntl (m_endPoint, F_GETFL, 0);
	if (flags < 0)
	{
		cout << "fcntl() failed, errno = " << errno << endl;
		return -1;
	}

	if (fcntl (m_endPoint, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		cout << "fcntl() failed, errno = " << errno << endl;
		return -1;
	}

	sockaddr_in6 saddr = *addr;
	saddr.sin6_port = htons (port);

	MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) m_task->mpx ();
	MpxRunningContext* ctx = mpx->ctx ();

	int status;
	if ((status = connect (m_endPoint, (struct sockaddr*) &saddr, sizeof(struct sockaddr_in6))) < 0)
	{
		if ((errno != EINPROGRESS) && (errno != EAGAIN))
		{
			cout << "connect() failed, errno = " << errno << endl;
			StartTimer (ctx->realTime ());
			return -1;
		}
	}

	m_IOHandle = ctx->RegisterDescriptor (EPOLLIN, m_endPoint, HandleSocketIO, this, ctx_info);

	StartTimer (ctx->realTime ());
	return 0;
}

void MpxTcp6Client::Shutdown ()
{
	if (m_endPoint > 0)
		shutdown (m_endPoint, SHUT_WR);
}

} /* namespace mpx */
