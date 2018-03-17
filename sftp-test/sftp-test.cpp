//    Event Driven Task Multiplexer test Program using FTP/SFTP Client Library
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

#include <mpx-events/MpxEvents.h>
#include <mpx-event-queues/MpxLocalMQTask.h>
#include <mpx-core/MpxEnvironment.h>
#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-jobs/MpxJobGetAddrInfo.h>
#include <mpx-tasks/MpxTaskBase.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
using namespace mpx;
#include <ftpwrk/FtpClientWorker.h>
#include <ftpwrk/SftpClientWorker.h>
#include <ftplog/SftpLog.h>
using namespace sftp;
#include <map>
using namespace std;

typedef set <MpxTaskBase*> taskset;

static FtpRequest* CreateFtpRequest (int n, char*p [])
{
	bool status = false;
	bool ssh = false;
	char* hostname = 0;
	char* user = 0;
	char* password = 0;
	char* authentication = 0;
	char* localDir = 0;
	char* localFile = 0;
	char* remoteDir = 0;
	char* remoteFile = 0;
	char* workingDir = 0;
	FtpRequestCode requestCode = FirstRequest;

	FtpRequest* req = new FtpRequest;
	memset (req, 0, sizeof *req);

	for (int i = 1; i < n; ++i)
	{
		if (strcasecmp (p [i], "--ssh") == 0)
		{
			ssh = true;
		}
		else if (strcasecmp (p [i], "--hostname") == 0)
		{
			if (++i < n)
				hostname = strdup (p [i]);
		}
		else if (strcasecmp (p [i], "--user") == 0)
		{
			if (++i < n)
				user = strdup (p [i]);
		}
		else if (strcasecmp (p [i], "--password") == 0)
		{
			if (++i < n)
				password = strdup (p [i]);
		}
		else if (strcasecmp (p [i], "--authentication") == 0)
		{
			if (++i < n)
				authentication = strdup (p [i]);
		}
		else if (strcasecmp (p [i], "--localDir") == 0)
		{
			if (++i < n)
				localDir = strdup (p [i]);
		}
		else if (strcasecmp (p [i], "--localFile") == 0)
		{
			if (++i < n)
				localFile = strdup (p [i]);
		}
		else if (strcasecmp (p [i], "--remoteDir") == 0)
		{
			if (++i < n)
				remoteDir = strdup (p [i]);
		}
		else if (strcasecmp (p [i], "--remoteFile") == 0)
		{
			if (++i < n)
				remoteFile = strdup (p [i]);
		}
		else if (strcasecmp (p [i], "--workingDir") == 0)
		{
			if (++i < n)
				workingDir = strdup (p [i]);
		}
		else if (strcasecmp (p [i], "--checkConnectivity") == 0)
			requestCode = CheckConnectivity;
		else if (strcasecmp (p [i], "--makeDir") == 0)
			requestCode = MakeDir;
		else if (strcasecmp (p [i], "--getDir") == 0)
			requestCode = GetDir;
		else if (strcasecmp (p [i], "--storeFile") == 0)
			requestCode = StoreFile;
		else if (strcasecmp (p [i], "--retrieveFile") == 0)
			requestCode = RetrieveFile;
		else if (strcasecmp (p [i], "--storeAllFiles") == 0)
			requestCode = StoreAllFiles;
		else if (strcasecmp (p [i], "--retrieveAllFiles") == 0)
			requestCode = RetrieveAllFiles;
		else if (strcasecmp (p [i], "--deleteFile") == 0)
			requestCode = DeleteFile;
		else if (strcasecmp (p [i], "--deleteAllFiles") == 0)
			requestCode = DeleteAllFiles;
		else if (strcasecmp (p [i], "--makeWorkingEnv") == 0)
			requestCode = MakeWorkingEnv;
		else if (strcasecmp (p [i], "--cleanDir") == 0)
			requestCode = CleanDir;
	}

	switch (req->request.requestCode = requestCode)
	{
	case CheckConnectivity:
		req->request.FtpRequestUnion_u.checkConnectivity.dummy = 0;
		status = true;
		break;
	case MakeDir:
		if ((req->request.FtpRequestUnion_u.makeDir.remoteDirName = remoteDir) == 0)
			break;
		status = true;
		break;
	case GetDir:
		if ((req->request.FtpRequestUnion_u.makeDir.remoteDirName = remoteDir) == 0)
			break;
		status = true;
		break;
	case StoreFile:
		if (((req->request.FtpRequestUnion_u.storeFile.localDirName = localDir) == 0)
			|| ((req->request.FtpRequestUnion_u.storeFile.localFileName = localFile) == 0)
			|| ((req->request.FtpRequestUnion_u.storeFile.remoteDirName = remoteDir) == 0)
			|| ((req->request.FtpRequestUnion_u.storeFile.remoteFileName = remoteFile) == 0))
			break;
		status = true;
		req->request.FtpRequestUnion_u.storeFile.workingDirName = (workingDir == 0) ? strdup ("") : workingDir;
		break;
	case RetrieveFile:
		if (((req->request.FtpRequestUnion_u.retrieveFile.localDirName = localDir) == 0)
			|| ((req->request.FtpRequestUnion_u.retrieveFile.localFileName = localFile) == 0)
			|| ((req->request.FtpRequestUnion_u.retrieveFile.remoteDirName = remoteDir) == 0)
			|| ((req->request.FtpRequestUnion_u.retrieveFile.remoteFileName = remoteFile) == 0))
			break;
		status = true;
		req->request.FtpRequestUnion_u.retrieveFile.workingDirName = (workingDir == 0) ? strdup ("") : workingDir;
		break;
	case StoreAllFiles:
		if (((req->request.FtpRequestUnion_u.storeAllFiles.localDirName = localDir) == 0)
			|| ((req->request.FtpRequestUnion_u.storeAllFiles.remoteDirName = remoteDir) == 0))
			break;
		status = true;
		req->request.FtpRequestUnion_u.storeAllFiles.workingDirName = (workingDir == 0) ? strdup ("") : workingDir;
		break;
	case RetrieveAllFiles:
		if (((req->request.FtpRequestUnion_u.retrieveAllFiles.localDirName = localDir) == 0)
			|| ((req->request.FtpRequestUnion_u.retrieveAllFiles.remoteDirName = remoteDir) == 0))
			break;
		status = true;
		req->request.FtpRequestUnion_u.retrieveAllFiles.workingDirName = (workingDir == 0) ? strdup ("") : workingDir;
		break;
	case DeleteFile:
		if (((req->request.FtpRequestUnion_u.deleteFile.remoteDirName = remoteDir) == 0)
			|| ((req->request.FtpRequestUnion_u.deleteFile.remoteFileName = remoteFile) == 0))
			break;
		status = true;
		break;
	case DeleteAllFiles:
		if ((req->request.FtpRequestUnion_u.deleteAllFiles.remoteDirName = remoteDir) == 0)
			break;
		status = true;
		req->request.FtpRequestUnion_u.deleteAllFiles.exceptions = 0;
		break;
	case MakeWorkingEnv:
		if ((req->request.FtpRequestUnion_u.makeWorkingEnv.remoteDirName = remoteDir) == 0)
			break;
		status = true;
		break;
	case CleanDir:
		if ((req->request.FtpRequestUnion_u.cleanDir.remoteDirName = remoteDir) == 0)
			break;
		status = true;
		break;
	default:
		break;
	}

	if (((req->connection.hostname = hostname) == 0) || ((req->connection.user = user) == 0)
		|| ((req->connection.password = password) == 0))
		status = false;

	if (status == false)
		return 0;

	req->connection.authentication = (authentication != 0) ? strdup (authentication) : strdup ("");
	if (ssh)
		req->flags |= FTP_SSH_OPERATION;

	return req;
}

class SftpTest: public MpxTaskBase
{
public:
	SftpTest () : MpxTaskBase (g_evnset)
	{
		m_request = 0;
		m_it = m_sftpSet.end ();
		m_timer = 0;
	}
	~SftpTest ()
	{
		if (m_request != 0)
			xdr_free ((xdrproc_t) xdr_FtpRequest, (char*) m_request);
		m_request = 0;

		if (m_timer != 0)
			StopTimer (m_timer);
		m_timer = 0;

		m_ftpSet.clear ();
		m_sftpSet.clear ();
	}
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
		m_request = copyRequest (request);
	}
	FtpRequest* copyRequest (FtpRequest* request)
	{
		XDR xdr;
		u_long size = xdr_sizeof ((xdrproc_t) xdr_FtpRequest, (void*) request);
		char* buffer = (char*) alloca(size);
		xdrmem_create (&xdr, buffer, size, XDR_ENCODE);
		xdr_FtpRequest (&xdr, request);
		request = new FtpRequest;
		memset (request, 0, sizeof(FtpRequest));
		xdrmem_create (&xdr, buffer, size, XDR_DECODE);
		xdr_FtpRequest (&xdr, request);
		return request;
	}
	bool PostFtpRequest ()
	{

		if ((m_request->flags & FTP_SSH_OPERATION) != 0)
		{
			if (m_it != m_sftpSet.end ())
			{
				Send (*m_it, new SftpInviteRequest (copyRequest (m_request)));
				++m_it;
				return true;
			}
			m_it = m_sftpSet.begin ();
		}
		else
		{
			if (m_it != m_ftpSet.end ())
			{
				Send (*m_it, new SftpInviteRequest (copyRequest (m_request)));
				++m_it;
				return true;
			}
			m_it = m_ftpSet.begin ();
		}
		timespec t = GetCurrentTime ();
		t.tv_sec += 1;
		m_timer = StartTimer (t);
		return false;
	}
private:
	mpx_event_handler(HandleTimerEvent, SftpTest)
	mpx_event_handler(HandleInviteReplyEvent, SftpTest)
	mpx_event_handler(HandleClientStartEvent, SftpTest)
	mpx_event_handler(HandleClientStopEvent, SftpTest)
	mpx_event_handler(HandleClientRequestEvent, SftpTest)
	mpx_event_handler(HandleClientReplyEvent, SftpTest)
	mpx_event_handler(HandleJobInfoEvent, SftpTest)

private:
	static EventDescriptor g_events [];
	static evnset g_evnset;
	taskset m_ftpSet;
	taskset m_sftpSet;
	FtpRequest* m_request;
	taskset::iterator m_it;
	void* m_timer;
};

EventDescriptor SftpTest::g_events [] =
{
	{ AnyState, MpxTimerEvent::EventCode, HandleTimerEvent },
	{ AnyState, SftpInviteReply::EventCode, HandleInviteReplyEvent },
	{ AnyState, SftpClientStart::EventCode, HandleClientStartEvent },
	{ AnyState, SftpClientStop::EventCode, HandleClientStopEvent },
	{ AnyState, SftpClientRequest::EventCode, HandleClientRequestEvent },
	{ AnyState, SftpClientReply::EventCode, HandleClientReplyEvent },
	{ AnyState, SftpJobInfo::EventCode, HandleJobInfoEvent },
	{ 0, 0, 0 }
};

MpxTaskBase::evnset SftpTest::g_evnset = MpxTaskBase::CreateEventSet (SftpTest::g_events);

void SftpTest::StartTask ()
{
	if (m_request == 0)
		return;

	m_it = ((m_request->flags & FTP_SSH_OPERATION) != 0) ? m_sftpSet.begin () : m_ftpSet.begin ();
	m_timer = StartTimer (GetCurrentTime ());
}

void SftpTest::StopTask ()
{
	m_ftpSet.clear ();
	m_sftpSet.clear ();
}

void SftpTest::HandleTimerEvent (MpxEventBase* event)
{
	MpxTimerEvent* timerEvent = dynamic_cast <MpxTimerEvent*> (event);
	if (timerEvent == 0)
		return;

	if (timerEvent == m_timer)
	{
		m_timer = 0;
		PostFtpRequest ();
	}
	else
	{
		cf_sc_printf (SC_SFTP, SC_ERR, "Unknown timer expired");
	}
}

void SftpTest::HandleInviteReplyEvent (MpxEventBase* event)
{
	SftpInviteReply* inviteReply = dynamic_cast <SftpInviteReply*> (event);
	if (inviteReply == 0)
		return;

	if (inviteReply->invite () == false)
		cf_sc_printf (SC_SFTP, SC_ERR, "FTP/SFTP Invite Request refused");
}

void SftpTest::HandleClientStartEvent (MpxEventBase* event)
{
}

void SftpTest::HandleClientStopEvent (MpxEventBase* event)
{
	m_timer = StartTimer (GetCurrentTime ());
}

void SftpTest::HandleClientRequestEvent (MpxEventBase* event)
{
	SftpClientRequest* clientRequest = dynamic_cast <SftpClientRequest*> (event);
	if (clientRequest == 0)
		return;

	char** args = clientRequest->args ();
	char** ptr;
	char* msg;
	int size;

	for (size = 0, ptr = (char**) args; *ptr != 0; size += strlen (*ptr++))
		;
	++size;
	if ((msg = (char*) alloca(size)) == 0)
		return;

	for (size = 0, ptr = (char**) args; *ptr != 0; size += strlen (*ptr++))
		strcpy (msg + size, *ptr);

	cf_sc_printf (SC_SFTP, SC_ERR, "S%06d: --> %s", clientRequest->sessionId () % (1000 * 1000), msg);
}

void SftpTest::HandleClientReplyEvent (MpxEventBase* event)
{
	SftpClientReply* clientReply = dynamic_cast <SftpClientReply*> (event);
	if (clientReply == 0)
		return;

	char** args = clientReply->args ();
	char** ptr;
	char* msg;
	int size;

	for (size = 0, ptr = (char**) args; *ptr != 0; size += strlen (*ptr++))
		;
	++size;
	if ((msg = (char*) alloca(size)) == 0)
		return;

	for (size = 0, ptr = (char**) args; *ptr != 0; size += strlen (*ptr++))
		strcpy (msg + size, *ptr);

	cf_sc_printf (SC_SFTP, SC_ERR, "S%06d: <-- %s", clientReply->sessionId () % (1000 * 1000), msg);
}

void SftpTest::HandleJobInfoEvent (MpxEventBase* event)
{
	SftpJobInfo* jobInfo = dynamic_cast <SftpJobInfo*> (event);
	if (jobInfo == 0)
		return;

	cf_sc_printf (SC_SFTP, SC_ERR, "SSH job %d.%d.%d (S%06d) --- %s", jobInfo->clientId (), jobInfo->jobCount (),
		jobInfo->sessionId (), jobInfo->sessionId () % (1000 * 1000), jobInfo->msg ());
}

int main (int n, char*p [])
{
	int timeOut = 10;

	for (int i = 1; i < n; ++i)
	{
		if (strcasecmp (p [i], "--timeout") == 0)
		{
			if (++i < n)
				timeOut = atoi (p [i]);
		}
	}

	FtpRequest* req = CreateFtpRequest (n, p);
	if (req == 0)
		return 0;

	MpxTaskMultiplexer* tmpx = MpxEnvironment::CreateTaskMultiplexer ();
	MpxTaskMultiplexer* smpx = MpxEnvironment::CreateTaskMultiplexer ();
	MpxTaskMultiplexer* fmpx = MpxEnvironment::CreateTaskMultiplexer ();

	SftpTest* sftp = new SftpTest ();
	sftp->request (req);
	tmpx->RegisterTask (sftp);

	for (int i = 0; i < 10; ++i)
	{
		MpxTaskBase* task = new FtpClientWorker ();
		sftp->AddFtpWorker (task);
		fmpx->RegisterTask (task);
	}

	for (int i = 0; i < 10; ++i)
	{
		MpxTaskBase* task = new SftpClientWorker ();
		sftp->AddSftpWorker (task);
		smpx->RegisterTask (task);
	}

	MpxEnvironment::Start (new MpxLocalMQTask ());
	for (int i = 0; i < timeOut; ++i)
		sleep (1);
	MpxEnvironment::Stop ();

	return 0;
}
