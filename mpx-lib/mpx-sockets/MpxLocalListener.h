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
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mpx-core/MpxUtilities.h"
#include <mpx-core/MpxRunningContext.h>
#include <mpx-tasks/MpxTaskBase.h>

#include <string>
using namespace std;

namespace mpx
{

class MpxLocalListener
{
public:
	MpxLocalListener (MpxTaskBase* task, bool seqPacket = false);
	virtual ~MpxLocalListener ();
	int CreateListener (const char* path);
	int StartListener ();
	int StopListener ();
private:
	fd_handler (HandleListener, MpxLocalListener)
	;
private:
	MpxTaskBase* m_task;
	bool m_seqPacket;

	string m_listenPath;
	int m_listenSocket;
	ctx_fddes_t m_handler;
};

} /* namespace mpx */
