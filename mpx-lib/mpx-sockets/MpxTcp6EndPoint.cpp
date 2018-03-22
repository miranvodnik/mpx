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
#include <mpx-sockets/MpxTcp6EndPoint.h>

namespace mpx
{

MpxTcp6EndPoint::MpxTcp6EndPoint (MpxTaskBase* task, int endPoint, bool fast, long int timeOut, bool seqPacket) :
	MpxSocket (task, fast, timeOut, seqPacket), m_endPoint (endPoint)
{
	MpxTaskMultiplexer* mpx = reinterpret_cast <MpxTaskMultiplexer*> (m_task->mpx ());
	if (mpx->getTid () != syscall (SYS_gettid))
	{
		close (m_endPoint);
		m_endPoint = 0;
		return;
	}

	MpxRunningContext* ctx = mpx->ctx ();
	m_IOHandle = ctx->RegisterDescriptor (EPOLLIN | EPOLLOUT, m_endPoint, HandleSocketIO, this, ctx_info);
	ctx->DisableDescriptor (m_IOHandle, EPOLLOUT);
}

MpxTcp6EndPoint::~MpxTcp6EndPoint ()
{
	Release ();
}

void MpxTcp6EndPoint::Release ()
{
	if (m_endPoint != 0)
		close (m_endPoint);
	m_endPoint = 0;

	MpxSocket::Release ();
}

} // namespace mpx
