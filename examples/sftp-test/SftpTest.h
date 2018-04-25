//    TODO
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

//
// SftpTest.h
//
//  Created on: Mar 29, 2018
//      Author: miran
//

#pragma once

#include <mpx-events/MpxEvents.h>
#include <mpx-jobs/MpxJobGetAddrInfo.h>
#include <mpx-tasks/MpxTaskBase.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
using namespace mpx;

#include <ftpmsg/ftpmsg.h>

#include <map>
using namespace std;

#include <mpx-tasks/MpxTaskBase.h>

namespace sftp_test
{

//
//
//
class SftpTest: public mpx::MpxTaskBase
{
	typedef set <MpxTaskBase*> taskset;
public:
	SftpTest ();
	~SftpTest ();
	virtual void StartTask ();
	virtual void StopTask ();
	inline void AddFtpWorker (MpxTaskBase* ftp)
	{
		m_ftpSet.insert (ftp);
	}
	inline void AddSftpWorker (MpxTaskBase* sftp)
	{
		m_sftpSet.insert (sftp);
	}
	inline void request (FtpRequest* request)
	{
		m_request = CopyFtpRequest (request);
	}
	static FtpRequest* CreateFtpRequest (int n, char*p []);
	static FtpRequest* CopyFtpRequest (FtpRequest* request);
	static void DeleteFtpRequest (FtpRequest* request);
	bool PostFtpRequest (timespec t);
private:
	mpx_event_handler(HandleTimerEvent, SftpTest)
	mpx_event_handler(HandleInviteReplyEvent, SftpTest)
	mpx_event_handler(HandleClientStartEvent, SftpTest)
	mpx_event_handler(HandleClientStopEvent, SftpTest)
	mpx_event_handler(HandleClientRequestEvent, SftpTest)
	mpx_event_handler(HandleClientReplyEvent, SftpTest)
	mpx_event_handler(HandleJobInfoEvent, SftpTest)
	mpx_event_handler(HandleSftpCallbackEvent, SftpTest)

private:
	static EventDescriptor g_events [];
	static evnset g_evnset;
	taskset m_ftpSet;
	taskset m_sftpSet;
	FtpRequest* m_request;
	taskset::iterator m_it;
	void* m_timer;
};

} // namespace sftp_test
