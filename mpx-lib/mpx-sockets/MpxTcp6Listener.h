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

#include <mpx-core/MpxUtilities.h>
#include <mpx-tasks/MpxTaskBase.h>

namespace mpx
{

class MpxTcp6Listener
{
public:
	MpxTcp6Listener (MpxTaskBase* task, bool seqPacket = false);
	virtual ~MpxTcp6Listener ();
	int CreateListener (in_port_t listenPort);
	int CreateListener (const char* localAddress, in_port_t listenPort);
	int CreateListener (struct sockaddr* localAddress, in_port_t listenPort);
	int StartListener ();
	int StopListener ();
private:
	fd_handler (HandleListener, MpxTcp6Listener);
private:
	MpxTaskBase* m_task;
	bool m_seqPacket;

	in_port_t m_listenPort;
	int m_listenSocket;
	ctx_fddes_t m_handler;
};

} /* namespace mpx */
