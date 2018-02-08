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

#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-core/MpxUtilities.h>
#include <mpx-tasks/MpxTaskBase.h>

#include <string>
using namespace std;

namespace mpx
{

enum MpxPosixMQType
{
	PosixMQInvalidType, PosixMQConsumerType, PosixMQProviderType
};

typedef enum MpxPosixMQType MpxPosixMQType;

class MpxPosixMQ
{
public:
	MpxPosixMQ (MpxTaskBase* task, bool fast = false, long int timeOut = 0);
	virtual ~MpxPosixMQ ();
	int Create (const char* mqPath, long int msgSize);
	int Connect (const char* localPath, long int msgSize);
	int Start ();
	int Stop ();

	int Write (u_char* buffer, size_t size);
	int PostXdrRequest (xdrproc_t proc, void* data);

	inline bool fast ()
	{
		return m_fast;
	}

	inline MpxPosixMQType type ()
	{
		return m_type;
	}
private:
	int PutChunk (u_char* chunk);
	u_char* GetChunk ();
	void RestoreChunk (u_char* chunk);
	void Release ();
	void StartTimer (timespec t);

	timer_handler (IdleTimer, MpxPosixMQ)
	;fd_handler (HandleMqConsumerIO, MpxPosixMQ)
	;fd_handler (HandleMqProviderIO, MpxPosixMQ)
	;
private:
	MpxTaskBase* m_task;
	bool m_fast;
	long int m_timeOut;

	string m_mqPath;
	long int m_msgSize;
	u_char* m_msg;

	MpxTaskMultiplexer* m_mpx;
	MpxRunningContext* m_ctx;
	mqd_t m_fd;
	MpxPosixMQType m_type;

	ctx_timer_t m_idleTimer;
	ctx_fddes_t m_IOHandle;

	u_char** m_chunkTable;
	ssize_t m_chunkTableSize;
	ssize_t m_chunkReadIndex;
	ssize_t m_chunkWriteIndex;
};

} /* namespace mpx */
