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
#include <mpx-sockets/MpxLocalClient.h>

namespace mpx
{

MpxLocalClient::MpxLocalClient (MpxTaskBase* task, bool fast, long int timeOut, bool seqPacket) :
	MpxSocket (task, fast, timeOut, seqPacket)
{
	MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) m_task->mpx ();
	if (mpx->getTid () != syscall (SYS_gettid))
		return;

	m_endPoint = 0;
}

MpxLocalClient::~MpxLocalClient ()
{
	Release ();
}

void MpxLocalClient::Release ()
{
	if (m_endPoint != 0)
		close (m_endPoint);
	m_endPoint = 0;

	MpxSocket::Release ();
}

int MpxLocalClient::Connect (const char* localPath)
{
	if (m_IOHandle != 0)
	{
		cout << "ALREADY CONNECTED" << endl;
		return 0;	// already connected
	}

	m_localPath = localPath;
	m_endPoint = socket (PF_LOCAL, (m_seqPacket != true) ? SOCK_STREAM : SOCK_SEQPACKET, 0);
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

	struct sockaddr_un addr;
	memset (&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_LOCAL;
	strncpy (addr.sun_path, localPath, sizeof addr.sun_path);

	MpxTaskMultiplexer* mpx = (MpxTaskMultiplexer*) m_task->mpx ();
	MpxRunningContext* ctx = mpx->ctx ();

	int status;
	if ((status = connect (m_endPoint, reinterpret_cast <struct sockaddr*> (&addr), sizeof(struct sockaddr_un))) < 0)
	{
		if ((errno != EINPROGRESS) && (errno != EAGAIN))
		{
			cout << "connect(" << localPath << ") failed, errno = " << errno << endl;
			StartTimer (ctx->realTime ());
			return -1;
		}
	}

	m_IOHandle = ctx->RegisterDescriptor (EPOLLIN, m_endPoint, HandleSocketIO, this, ctx_info);
	StartTimer (ctx->realTime ());

	return 0;
}

} /* namespace mpx */
