//    FTP/SFTP Client Library
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

#include "SftpClient.h"

namespace sftp
{

static void CheckIODirections (LIBSSH2_SESSION* session, bool& in, bool& out)
{
	int dir;

	in = out = false;
	dir = libssh2_session_block_directions (session);

	if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)
		in = true;

	if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
		out = true;
}

int CBInvokeInfo::g_counter = 0;

SftpClient::SftpCallback* SftpClient::g_disposedScenario = 0;
SftpClient* SftpClient::g_disposedScenarioInitializer = new SftpClient (true);

SftpClient::SftpClient (bool initialize) :
			FtpClientInterface (Ssh2Protocol)
{
	if (!initialize)
		return;
	InitDisposedScenario ();
}

void SftpClient::InitDisposedScenario ()
{
	g_disposedScenario = new SftpClient::SftpCallback [SftpEventCount];
	memset (g_disposedScenario, 0, sizeof(SftpClient::SftpCallback [SftpEventCount]));

	g_disposedScenario [SCtrlBusyTimerExpiredEvent] = FinalTimerExpiredEventHandler;
	g_disposedScenario [SCtrlIdleTimerExpiredEvent] = FinalTimerExpiredEventHandler;
}

void SftpClient::FinalTimerExpiredEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state, void* args [])
{
	if (g_debug)
		cout << "INVOKE SFTP FINALIZER - client = " << clientId () << endl;
	Dispose ("finalizer");
}

SftpClient::SftpClient (evnset& e) :
	FtpClientInterface (e, Ssh2Protocol)
{
	m_ctx = 0;
	memset (m_ftpEvents, 0, sizeof(m_ftpEvents));
	m_scenarioEvents = 0;
	m_appData = 0;

	m_session = 0;
	m_sftp = 0;
	m_dirHandle = 0;

	m_status = sftp_success;
	m_state = SStartup;
	m_requestHandler = 0;
	m_replyHandler = 0;

	m_connDes = 0;
	m_connTmr = 0;
	m_connTimerEnabled = 0;
	m_connTimerDisabled = 0;
	m_connSocket = -1;

	m_targetPath = 0;
	m_targetPathLen = 0;

	m_currentRemoteDir = ".";
	m_currentLocalDir = ".";

	m_localFileHandle = 0;
	m_remoteFileHandle = 0;
	m_fileChunk = 0;
	m_fileChunkPtr = 0;
	m_fileChunkEnd = 0;
	m_mode = 0;
	m_dirMode = 0;

	m_useTrash = true;
}

SftpClient::~SftpClient ()
{
	Dispose ("destructor");
}

void SftpClient::Dispose (const char* msg)
{
	if (g_debug)
		cout << "SftpClient::Dispose - client = " << clientId () << ", socket = " << m_connSocket << endl;
	if (g_debug)
		cout << "DISPOSE: " << msg << endl;

	if (m_state != SDisposed)
		InvokeFtpCallback (SDisposedEvent, sftp_stop, sftp_success, m_state = SDisposed, NULL);

	if (m_useTrash)
	{
		//	create clean-up object for SSH resources within current running context
		SftpTrash*	trash = new SftpTrash (m_ctx, m_connSocket, m_session, m_sftp, m_dirHandle, m_remoteFileHandle);
		trash->CleanUp();	// initiate clean-up and leave trash object to cease within current running context

		m_connSocket = -1;	// clean-up will be done by SftpTrash
		m_session = 0;	// clean-up will be done by SftpTrash
		m_sftp = 0;	// clean-up will be done by SftpTrash
		m_dirHandle = 0;	// clean-up will be done by SftpTrash
		m_remoteFileHandle = 0;	// clean-up will be done by SftpTrash
	}
	else
	{
		if (m_remoteFileHandle != 0)
			libssh2_sftp_close_handle (m_remoteFileHandle);
		m_remoteFileHandle = 0;

		if (m_dirHandle != 0)
			libssh2_sftp_close_handle (m_dirHandle);
		m_dirHandle = 0;

		if (m_sftp != 0)
			libssh2_sftp_shutdown (m_sftp);
		m_sftp = 0;

		if (m_session != 0)
			libssh2_session_disconnect (m_session, "");
		m_session = 0;

		if (m_connSocket > 0)
			close (m_connSocket);
		m_connSocket = 0;
	}

	//	release target path
	if (m_targetPath != 0)
		free (m_targetPath);
	m_targetPath = 0;
	m_targetPathLen = 0;

	// close local file
	if (m_localFileHandle != 0)
		fclose (m_localFileHandle);
	m_localFileHandle = 0;

	UnregisterAllFtpCallbacks ();
	UnregisterFtpScenario ();
	m_scenarioEvents = 0;
	m_appData = 0;
	if (m_connDes != 0)
		m_ctx->RemoveDescriptor (m_connDes);
	m_connDes = 0;
	if (m_connTmr != 0)
	{
		++m_connTimerDisabled;
		m_ctx->DisableTimer (m_connTmr);
		if (g_debug)
			cout << "DISABLE CONN TIMER - client = " << clientId () << ", enabled = " << m_connTimerEnabled << ", disabled = "
				<< m_connTimerDisabled << endl;
	}
	m_connTmr = 0;

	m_currentRemoteDir = ".";
	m_currentLocalDir = ".";

	if (m_fileChunk != 0)
		free (m_fileChunk);
	m_fileChunk = 0;
}

void SftpClient::ConnSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	if (flags & EPOLLIN)
	{
		if (m_connTmr != 0)
		{
			++m_connTimerDisabled;
			m_ctx->DisableTimer (m_connTmr);
		}
		m_connTmr = 0;

		int count = 0;
		if (ioctl (fd, FIONREAD, &count) < 0)
		{
			Dispose ("ioctl");
			return;
		}
		if (count <= 0)
		{
			Dispose ("count");
			return;
		}
		if (m_requestHandler != 0)
		{
			RequestHandler requestHandler = m_requestHandler;
			m_requestHandler = 0;
			if (g_debug)
				cout << "IN HANDLER (" << ctx->round () << ", " << count << "): ";
			(this->*requestHandler) (0); // should alter m_connTmr
		}
		else if (g_debug)
			cout << "IN NO HANDLER (" << ctx->round () << ", " << count << "): ";

		if (m_connTmr == 0) // don't remove this test
		{
			struct timespec t = m_ctx->realTime ();
			t.tv_sec += 10;
			m_connTmr = ctx->RegisterTimer (t, ConnIdleTimerHandler, this, ctx_info);
			++m_connTimerEnabled;
		}
	}
	else
	{
		m_ctx->DisableDescriptor (m_connDes, EPOLLOUT);
		if (m_replyHandler != 0)
		{
			ReplyHandler replyHandler = m_replyHandler;
			m_replyHandler = 0;
			if (g_debug)
				cout << "OUT HANDLER (" << ctx->round () << ", " << "): ";
			(this->*replyHandler) (0);
		}
		else if (g_debug)
			cout << "OUT NO HANDLER (" << ctx->round () << ", " << "): ";
	}
}

void SftpClient::ConnIdleTimerHandler (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	m_connTmr = 0;
	void* args [] =
		{ (void*) clientId (), (void*) (long) m_connTimerEnabled, (void*) (long) m_connTimerDisabled };
	InvokeFtpCallback (SCtrlIdleTimerExpiredEvent, sftp_ctrl, m_status, m_state, args);
	Dispose ("ConnIdleTimerHandler"); // m_ctrlTmr = NULL;
}

/****************************************************************/
/* Function:    SftpClient::RegisterFtpCallback ()              */
/* In:          index - event id: it is equal to index into     */
/*                    event callback table                      */
/*              cb - callback function which will be called to  */
/*                   handle event given by index                */
/*              appData - application specific data, usually    */
/*                   called context data and it is used         */
/*                   unchanged in invocation of callback        */
/*                   function.                                  */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: registration function for callback function and */
/*              associated context data for given event         */
/****************************************************************/

void SftpClient::RegisterFtpCallback (SftpEventIndex index, SftpCallback cb, void* appData)
{
	delete m_ftpEvents [index];
	m_ftpEvents [index] = new pair < SftpCallback, void* > (cb, appData);
}

/****************************************************************/
/* Function:    SftpClient::UnregisterFtpCallback ()            */
/* In:          index - event id: it is equal to index into     */
/*                    event callback table                      */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: function removes callback function and          */
/*              associated context data for given event         */
/****************************************************************/

void SftpClient::UnregisterFtpCallback (SftpEventIndex index)
{
	delete m_ftpEvents [index];
	m_ftpEvents [index] = NULL;
}

/****************************************************************/
/* Function:    SftpClient::UnregisterAllFtpCallbacks ()        */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: function clears event table for given FTP client*/
/*              object. No event will be reported any more      */
/****************************************************************/

void SftpClient::UnregisterAllFtpCallbacks ()
{
	int i;

	for (i = 0; i < SftpEventCount; ++i)
		delete m_ftpEvents [i];
	memset (m_ftpEvents, 0, sizeof(m_ftpEvents));
	m_scenarioEvents = 0;
}

/****************************************************************/
/* Function:    SftpClient::RegisterFtpScenario ()              */
/* In:          dynamicEvents - table of ftp callback functions */
/*              appData - context data common to all callback   */
/*                    functions                                 */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: registration mechanism for activation of        */
/*              'special ftp client scenario'. Every ftp client */
/*              object should register different scenario, one  */
/*              at a given time. Every scenario should be       */
/*              changed by another one at any time. Callbacks   */
/*              registered this way have precedence over        */
/*              callbacks registered with RegisterFtpCallback ()*/
/****************************************************************/

void SftpClient::RegisterFtpScenario (SftpCallback* scenarioEvents, void*appData)
{
	if (g_debug)
		cout << "REGISTER SFTP SCENARIO - client = " << clientId () << ", scenario = " << (u_long) scenarioEvents << endl;
	m_scenarioEvents = scenarioEvents;
	m_appData = appData;
}

/****************************************************************/
/* Function:    SftpClient::UnregisterFtpScenario ()            */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: function removes current 'ftp scenario'.        */
/****************************************************************/

void SftpClient::UnregisterFtpScenario ()
{
	m_scenarioEvents = NULL;
}

/****************************************************************/
/* Function:    SftpClient::InvokeFtpCallbackTimer ()           */
/* In:          index - id of event having been fired           */
/*              context - ftp client context - see FtpContext   */
/*              status - status of activity which fired event   */
/*              state - state representing activity which fired */
/*                      event                                   */
/*              args - additional data set which is used in     */
/*                     callback invocation                      */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: function invokes callback registered for given  */
/*              event immediately in the next step of main loop */
/****************************************************************/

void SftpClient::InvokeFtpCallbackTimer (SftpEventIndex index, SftpContext context, SftpStatus status, SftpState state, void* args [])
{
	if (g_debug)
		cout << "FTP CALLBACK TIMER  (index = " << index << ", context = " << context << ", status = " << status << ", state = " << state << ")" << endl;
	CBInvokeInfo* info = new CBInvokeInfo (index, context, status, state, args, (void*) this); // !!! this stored in info
	m_ctx->RegisterTimer (m_ctx->realTime (), FtpCallbackInvoker, info, ctx_info); // !!! this 'becomes' info
}

void SftpClient::FtpCallbackInvoker (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{ // !!! this pointer is invalid at this point, it actually points to CBInvokeInfo
	CBInvokeInfo* info = (CBInvokeInfo*) this; // !!! info 'becomes' this
	((SftpClient*) info->_this ())->RealFtpCallbackInvoker (ctx, handler, t, info); // !!! this retrieved from info
	delete info;
}

void SftpClient::RealFtpCallbackInvoker (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t, CBInvokeInfo* info)
{
	InvokeFtpCallback (info->index (), info->context (), info->status (), info->state (), info->args ());
}

void SftpClient::InvokeFtpCallback (SftpEventIndex index, SftpContext context, SftpStatus status, SftpState state, void* args [])
{
	if (g_debug)
		cout << "INVOKE FTP CALLBACK (index = " << index << ", context = " << context << ", status = " << status << ", state = " << state << ")" << endl;

	void* nullArgs [] =
		{ (void*) index, 0 };
	if (m_scenarioEvents == NULL)
	{
		ftpcbdes* it = m_ftpEvents [index];
		if (it == NULL)
		{
			it = m_ftpEvents [SftpNullEvent];
			args = nullArgs;
		}
		if (it != NULL)
			it->first (this, context, status, state, args, it->second);
	}
	else
	{
		SftpCallback ftpcb = m_scenarioEvents [index];
		if (ftpcb == NULL)
		{
			ftpcb = m_scenarioEvents [SftpNullEvent];
			args = nullArgs;
		}
		if (ftpcb != NULL)
			ftpcb (this, context, status, state, args, m_appData);
	}
}

/****************************************************************/
/* Function:    SftpClient::Start ()                            */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: starting routine for ftp client. This is the    */
/*              prefered way to start FTP transfer              */
/****************************************************************/

void SftpClient::Start ()
{
	if (g_debug)
		cout << "Start ()" << endl;
	{
		void* args [] =
			{ (void*) "started", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_start, sftp_success, m_state, args);
	}
	InvokeFtpCallbackTimer (SStartupEvent, sftp_start, sftp_success, m_state = SStartup, NULL);
}

/****************************************************************/
/* Function:    SftpClient::Stop ()                             */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: finishing routine for FTP transfer - used to    */
/*              free resources                                  */
/****************************************************************/

void SftpClient::Stop ()
{
	if (g_debug)
		cout << "Stop ()" << endl;
	{
		void* args [] =
			{ (void*) "ssh disposed", (void*) m_targetPath, 0 };
		InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
	}
	Dispose ("STOP");
}

/****************************************************************/
/* Function:    SftpClient::Connect ()                          */
/* In:          hostname - hostname of FTP server               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post connection request to FTP server using its */
/*              hostname                                        */
/****************************************************************/

void SftpClient::Connect (string hostname)
{
	InvokeFtpCallback (SConnectPreparedEvent, sftp_ctrl, sftp_success, m_state = SConnectPrepared, 0);
	{
		void* args [] =
			{ (void*) "connect to ", (void*) hostname.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "Connect B (" << hostname << ")" << endl;
	m_state = SStartup;
	m_hostname = hostname;
	m_connSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_connSocket < 0)
	{
		void* args [] =
			{ (void*) "cannot create SFTP control connection socket", (void*) "socket", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("socket");
		return;
	}

	struct hostent* hostEntry = gethostbyname (hostname.c_str ());
	if (hostEntry == NULL)
	{
		string msg = "cannot get host info ";
		msg += hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "gethostbyname", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("gethostbyname");
		return;
	}

	int flags = fcntl (m_connSocket, F_GETFL, 0);
	if (flags < 0)
	{
		void* args [] =
			{ (void*) "cannot acquire SFTP control socket status", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("fcntl");
		return;
	}

	if (fcntl (m_connSocket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		void* args [] =
			{ (void*) "cannot change SFTP control socket to nonblocking mode", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("fcntl");
		return;
	}

	struct sockaddr_in addr;
	memset (&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (IPPORT_SFTP);
	addr.sin_addr.s_addr = *((uint*) (hostEntry->h_addr_list [0]));

	int status;
	if ((status = connect (m_connSocket, (struct sockaddr*) &addr, sizeof(struct sockaddr_in))) < 0)
	{
		if (errno != EINPROGRESS)
		{
			string msg = "cannot connect to ";
			msg += hostname;
			void* args [] =
				{ (void*) msg.c_str (), (void*) "connect", (void*) (long) errno };
			InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
			Dispose ("connect");
			return;
		}
		if (g_debug)
			cout << "CONNECT BUSY" << endl;
	}

	m_session = libssh2_session_init ();
	if (m_session == 0)
	{
		string msg = "cannot create SSH2 session for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_session_init", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("libssh2_session_init");
		return;
	}
	libssh2_session_set_blocking (m_session, 0);

	struct timespec t = m_ctx->realTime ();
	t.tv_sec += 10;
	m_connTmr = m_ctx->RegisterTimer (t, ConnIdleTimerHandler, this, ctx_info);
	m_connDes = m_ctx->RegisterDescriptor (EPOLLIN, m_connSocket, ConnSocketHandler, this, ctx_info);
	m_requestHandler = &SftpClient::HandleConnectReply;
}

/****************************************************************/
/* Function:    SftpClient::Connect ()                          */
/* In:          hostname - hostname of FTP server               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post connection request to FTP server using its */
/*              hostname and address info (ipv4, ipv6, ...)     */
/****************************************************************/

void SftpClient::Connect (string hostname, sockaddr* addr, socklen_t addrlen)
{
	InvokeFtpCallback (SConnectPreparedEvent, sftp_ctrl, sftp_success, m_state = SConnectPrepared, 0);
	{
		void* args [] =
			{ (void*) "connect to ", (void*) hostname.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "Connect A (" << hostname << ")" << endl;
	m_state = SStartup;
	m_hostname = hostname;
	switch (addr->sa_family)
	{
	case AF_INET:
		m_connSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
		break;
	case AF_INET6:
		m_connSocket = socket (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		break;
	}
	if (m_connSocket < 0)
	{
		void* args [] =
			{ (void*) "cannot create SFTP control connection socket", (void*) "socket", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("socket");
		return;
	}

	int flags = fcntl (m_connSocket, F_GETFL, 0);
	if (flags < 0)
	{
		void* args [] =
			{ (void*) "cannot acquire SFTP control socket status", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("fcntl");
		return;
	}

	if (fcntl (m_connSocket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		void* args [] =
			{ (void*) "cannot change SFTP control socket to nonblocking mode", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("fcntl");
		return;
	}

	int status;
	if ((status = connect (m_connSocket, addr, addrlen)) < 0)
	{
		if (errno != EINPROGRESS)
		{
			string msg = "cannot connect to ";
			msg += hostname;
			void* args [] =
				{ (void*) msg.c_str (), (void*) "connect", (void*) (long) errno };
			InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
			Dispose ("connect");
			return;
		}
	}

	m_session = libssh2_session_init ();
	if (m_session == 0)
	{
		string msg = "cannot create SSH2 session for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_session_init", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("libssh2_session_init");
		return;
	}
	libssh2_session_set_blocking (m_session, 0);

	struct timespec t = m_ctx->realTime ();
	t.tv_sec += 10;
	m_connTmr = m_ctx->RegisterTimer (t, ConnIdleTimerHandler, this, ctx_info);
	m_connDes = m_ctx->RegisterDescriptor (EPOLLIN, m_connSocket, ConnSocketHandler, this, ctx_info);
	m_requestHandler = &SftpClient::HandleConnectReply;
}

void SftpClient::Handshake ()
{
	InvokeFtpCallback (SHandshakePreparedEvent, sftp_ctrl, sftp_success, m_state = SHandshakePrepared, 0);
	{
		void* args [] =
			{ (void*) "handshake", 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "Handshake (" << ")" << endl;
	_Handshake ();
}

void SftpClient::_Handshake ()
{
	int status;
	switch (status = libssh2_session_handshake (m_session, m_connSocket))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "HANDSHAKE AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleHandshakeBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "HANDSHAKE FINISHED" << endl;
		m_requestHandler = 0;
		{
			void* args [] =
				{ (void*) "handshake succeeded", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}

		if (false)
		{
			LIBSSH2_AGENT*	agent = libssh2_agent_init (m_session);
			if (agent != 0)
			{
				int	reason = libssh2_agent_connect (agent);
				if (reason == 0)
				{
					reason = libssh2_agent_list_identities (agent);
					if (reason == 0)
					{
						struct libssh2_agent_publickey*	store = 0;
						struct libssh2_agent_publickey*	prev = 0;
						while ((reason = libssh2_agent_get_identity (agent, &store, prev)) == 0)
						{
							prev = store;
							cout << "AGENT: " << store->comment << endl;
						}
						if (reason < 0)
							cout << "libssh2_agent_get_identity failed, reason = " << reason << endl;
						else
							cout << "libssh2_agent_get_identity succeeded, reason = " << reason << endl;
					}
					else
						cout << "libssh2_agent_list_identities failed, reason = " << reason << endl;

				}
				else
					cout << "libssh2_agent_connect failed, reason = " << reason << endl;

			}
			else
				cout << "libssh2_agent_init failed" << endl;
		}

		InvokeFtpCallbackTimer (SHandshakeFinishedEvent, sftp_ctrl, sftp_success, m_state = SHandshakeFinished, 0);
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR HANDSHAKE" << endl;
		string msg = "SSH2 handshake failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_session_handshake", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		Dispose ("libssh2_session_handshake");
		return;
	}
		m_requestHandler = 0;
		break;
	}
}

void SftpClient::CheckHash (int algorithm)
{
	const char* hash;

	InvokeFtpCallback (SHashPreparedEvent, sftp_ctrl, sftp_success, m_state = SHashPrepared, 0);
	{
		void* args [] =
			{ (void*) "hash ", (void*) (
				(algorithm == LIBSSH2_HOSTKEY_HASH_MD5) ? "md5" : (algorithm == LIBSSH2_HOSTKEY_HASH_SHA1) ? "sha1" : "unknown"),
				0
			};
			InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
		}
	if (g_debug)
		cout << "CheckHash (" << ")" << endl;

	switch (algorithm)
	{
	case LIBSSH2_HOSTKEY_HASH_MD5:
		if ((hash = libssh2_hostkey_hash (m_session, LIBSSH2_HOSTKEY_HASH_MD5)) != 0)
		{
			m_state = Md5Hash;
			if (g_debug)
				cout << "HASH MD5" << endl;
			{
				void* args [] =
					{ (void*) "md5 ok", 0 };
				InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
			}
			InvokeFtpCallbackTimer (Md5HashEvent, sftp_ctrl, sftp_success, m_state, (void**) hash);
			return;
		}
		break;
	case LIBSSH2_HOSTKEY_HASH_SHA1:
		if ((hash = libssh2_hostkey_hash (m_session, LIBSSH2_HOSTKEY_HASH_SHA1)) != 0)
		{
			m_state = Sha1Hash;
			if (g_debug)
				cout << "HASH SHA1" << endl;
			{
				void* args [] =
					{ (void*) "sha1 ok", 0 };
				InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
			}
			InvokeFtpCallbackTimer (Sha1HashEvent, sftp_ctrl, sftp_success, m_state, (void**) hash);
			return;
		}
		break;
	default:
		break;
	}

	{
		if (g_debug)
			cout << "ERROR HASH" << endl;
		string msg = "SSH2 host key hash failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_hostkey_hash", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		return;
	}
}

void SftpClient::LoginPublickey (string user, string publickey)
{
	InvokeFtpCallback (SAuthPreparedEvent, sftp_ctrl, sftp_success, m_state = SAuthPrepared, 0);
	{
		void* args [] =
			{ (void*) "login ", (void*) user.c_str (), (void*) ", ******", 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "LoginPublickey (" << user << ", " << publickey << ")" << endl;

	m_user = user;
	m_publickey = publickey;
	LoginPublickey ();
}

void SftpClient::LoginPublickey ()
{
	int status;
	string	pub = m_publickey + ".pub";

	switch (status = libssh2_userauth_publickey_fromfile (m_session, m_user.c_str(), pub.c_str(), m_publickey.c_str(), 0))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "AUTH AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleAuthPublickeyBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "AUTH FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "authenticated", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallbackTimer (SAuthFinishedEvent, sftp_ctrl, sftp_success, m_state = SAuthFinished, 0);
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR LOGIN, status = " << status << endl;
		string msg = "SSH2 user authentication failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_userauth_publickey_fromfile", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		return;
	}
		break;
	}
}

void SftpClient::LoginPassword (string user, string password)
{
	InvokeFtpCallback (SAuthPreparedEvent, sftp_ctrl, sftp_success, m_state = SAuthPrepared, 0);
	{
		void* args [] =
			{ (void*) "login ", (void*) user.c_str (), (void*) ", ******", 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "LoginPassword (" << user << ", " << password << ")" << endl;

	m_user = user;
	m_password = password;
	LoginPassword ();
}

void SftpClient::LoginPassword ()
{
	int status;

	switch (status = libssh2_userauth_password (m_session, m_user.c_str(), m_password.c_str()))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "AUTH AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleAuthPasswordBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "AUTH FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "authenticated", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallbackTimer (SAuthFinishedEvent, sftp_ctrl, sftp_success, m_state = SAuthFinished, 0);
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR LOGIN, status = " << status << endl;
		string msg = "SSH2 user authentication failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_userauth_password", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_userauth_password");
		return;
	}
		break;
	}
}

void SftpClient::CreateFtp ()
{
	InvokeFtpCallback (SSftpPreparedEvent, sftp_ctrl, sftp_success, m_state = SSftpPrepared, 0);
	{
		void* args [] =
			{ (void*) "init sftp", 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	_CreateFtp ();
}

void SftpClient::_CreateFtp ()
{
	if (g_debug)
		cout << "CreateFtp (" << ")" << endl;
	if ((m_sftp = libssh2_sftp_init (m_session)) != 0)
	{
		if (g_debug)
			cout << "SFTP FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "sftp connection established", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		_GetHomeDirectory ();
		return;
	}

	int status;
	switch (status = libssh2_session_last_errno (m_session))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "SFTP AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleSftpBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "SFTP FINISHED" << endl;
		_GetHomeDirectory ();
		break;
	default:
	{
		m_requestHandler = 0;
		if (g_debug)
			cout << "ERROR SFTP" << endl;
		string msg = "SSH2 cannot create sftp for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_init", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_init");
		return;
	}
		break;
	}
}

void SftpClient::_GetHomeDirectory ()
{
	if (g_debug)
		cout << "_GetHomeDirectory (" << ")" << endl;
	m_remoteDirName = ".";
	int len;
	while (true)
	{
		if ((len = libssh2_sftp_symlink_ex (m_sftp, m_remoteDirName.c_str (), m_remoteDirName.size (), m_targetPath, m_targetPathLen,
			LIBSSH2_SFTP_REALPATH)) > 0)
		{
			m_targetPath [len] = 0;
			m_currentRemoteDir = m_homeDir = m_targetPath;
			{
				void* args [] =
					{ (void*) "home = ", (void*) m_targetPath, 0 };
				InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
			}
			InvokeFtpCallbackTimer (SSftpFinishedEvent, sftp_ctrl, sftp_success, m_state = SSftpFinished, 0);

			if (m_connTmr == 0)
			{
				struct timespec t = m_ctx->realTime ();
				t.tv_sec += 10;
				m_connTmr = m_ctx->RegisterTimer (t, ConnIdleTimerHandler, this, ctx_info);
				if (g_debug)
					cout << "ENABLE CONN TIMER - client = " << clientId () << ", sftp" << endl;
				++m_connTimerEnabled;
			}
			return;
		}

		switch (len)
		{
		case LIBSSH2_ERROR_EAGAIN:
			if (g_debug)
				cout << "HOMEDIR AGAIN" << endl;
			m_requestHandler = &SftpClient::HandleHomeBusyReply;
			break;
		case LIBSSH2_ERROR_NONE:
			if (g_debug)
				cout << "HOMEDIR FINISHED" << endl;
			InvokeFtpCallbackTimer (SSftpFinishedEvent, sftp_ctrl, sftp_success, m_state = SSftpFinished, 0);

			if (m_connTmr == 0)
			{
				struct timespec t = m_ctx->realTime ();
				t.tv_sec += 10;
				m_connTmr = m_ctx->RegisterTimer (t, ConnIdleTimerHandler, this, ctx_info);
				if (g_debug)
					cout << "ENABLE CONN TIMER - client = " << clientId () << ", sftp" << endl;
				++m_connTimerEnabled;
			}
			break;
		case LIBSSH2_ERROR_BUFFER_TOO_SMALL:
			if (g_debug)
				cout << "HOMEDIR TOO SMALL" << endl;
			{
				int targetPathLen = m_targetPathLen;
				targetPathLen >>= 10;
				++targetPathLen;
				targetPathLen <<= 10;
				char* targetPath = (char*) malloc (targetPathLen);
				if (targetPath == 0)
				{
					m_requestHandler = 0;
					if (g_debug)
						cout << "ERROR HOMEDIR" << endl;
					string msg = "SSH2 cannot allocate memory for home directory for ";
					msg += m_hostname;
					void* args [] =
						{ (void*) msg.c_str (), (void*) "malloc", (void*) (long) errno };
					InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
					// Dispose ("libssh2_sftp_symlink_ex");
					return;
				}
				if (m_targetPath != 0)
					free (m_targetPath);
				m_targetPath = targetPath;
				m_targetPathLen = targetPathLen;
			}
			continue;
		default:
		{
			m_requestHandler = 0;
			if (g_debug)
				cout << "ERROR HOMEDIR, status = " << len << endl;
			string msg = "SSH2 cannot get home directory for ";
			msg += m_hostname;
			void* args [] =
				{ (void*) msg.c_str (), (void*) "libssh2_sftp_symlink_ex", (void*) (long) len };
			InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
			// Dispose ("libssh2_sftp_symlink_ex");
			return;
		}
			break;
		}
		break;
	}
}

void SftpClient::PrintWorkingDirectory ()
{
	InvokeFtpCallback (SPwdPreparedEvent, sftp_ctrl, sftp_success, m_state = SPwdPrepared, 0);
	{
		void* args [] =
			{ (void*) "pwd", 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	m_remoteDirName = GetRemoteDirectoryPath ("");
	_PrintWorkingDirectory ();
}

void SftpClient::_PrintWorkingDirectory ()
{
	if (g_debug)
		cout << "PrintWorkingDirectory (" << ")" << endl;
	int len;
	while (true)
	{
		if ((len = libssh2_sftp_symlink_ex (m_sftp, m_remoteDirName.c_str (), m_remoteDirName.size (), m_targetPath, m_targetPathLen,
			LIBSSH2_SFTP_REALPATH)) > 0)
		{
			m_targetPath [len] = 0;
			m_currentRemoteDir = m_targetPath;
			{
				void* args [] =
					{ (void*) "pwd = ", (void*) m_targetPath, 0 };
				InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
			}
			InvokeFtpCallbackTimer (SPwdFinishedEvent, sftp_ctrl, sftp_success, m_state = SPwdFinished, (void**) m_targetPath);
			return;
		}

		switch (len)
		{
		case LIBSSH2_ERROR_EAGAIN:
			if (g_debug)
				cout << "PWD AGAIN" << endl;
			m_requestHandler = &SftpClient::HandlePwdBusyReply;
			break;
		case LIBSSH2_ERROR_NONE:
			if (g_debug)
				cout << "PWD FINISHED" << endl;
			InvokeFtpCallbackTimer (SPwdFinishedEvent, sftp_ctrl, sftp_success, m_state = SPwdFinished, (void**) m_targetPath);
			break;
		case LIBSSH2_ERROR_BUFFER_TOO_SMALL:
			if (g_debug)
				cout << "PWD TOO SMALL" << endl;
			{
				int targetPathLen = m_targetPathLen;
				targetPathLen >>= 10;
				++targetPathLen;
				targetPathLen <<= 10;
				char* targetPath = (char*) malloc (targetPathLen);
				if (targetPath == 0)
				{
					m_requestHandler = 0;
					if (g_debug)
						cout << "ERROR PWD" << endl;
					string msg = "SSH2 cannot allocate memory for working directory for ";
					msg += m_hostname;
					void* args [] =
						{ (void*) msg.c_str (), (void*) "malloc", (void*) (long) errno };
					InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
					// Dispose ("libssh2_sftp_symlink_ex");
					return;
				}
				if (m_targetPath != 0)
					free (m_targetPath);
				m_targetPath = targetPath;
				m_targetPathLen = targetPathLen;
			}
			continue;
		default:
		{
			m_requestHandler = 0;
			if (g_debug)
				cout << "ERROR PWD, status = " << len << endl;
			string msg = "SSH2 cannot print working directory for ";
			msg += m_hostname;
			void* args [] =
				{ (void*) msg.c_str (), (void*) "libssh2_sftp_symlink_ex", (void*) (long) len };
			InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
			// Dispose ("libssh2_sftp_symlink_ex");
			return;
		}
			break;
		}
		break;
	}
}

void SftpClient::ChangeWorkingDirectory (string dir)
{
	if ((!dir.empty()) && ((*dir.end()) != '/'))
			dir += '/';
	InvokeFtpCallback (SCwdPreparedEvent, sftp_ctrl, sftp_success, m_state = SCwdPrepared, 0);
	{
		void* args [] =
			{ (void*) "cwd ", (void*) dir.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "ChangeWorkingDirectory (" << dir << ")" << endl;
	m_remoteDirName = GetRemoteDirectoryPath (dir);
	ChangeWorkingDirectory ();
}

void SftpClient::ChangeWorkingDirectory ()
{
	int len;
	while (true)
	{
		if (g_debug)
			cout << "ChangeWorkingDirectory (" << m_remoteDirName << ")" << endl;
		if ((len = libssh2_sftp_symlink_ex (m_sftp, m_remoteDirName.c_str (), m_remoteDirName.size (), m_targetPath, m_targetPathLen,
			LIBSSH2_SFTP_REALPATH)) > 0)
		{
			m_targetPath [len] = 0;
			m_currentRemoteDir = m_targetPath;
			{
				void* args [] =
					{ (void*) "cwd = ", (void*) m_targetPath, 0 };
				InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
			}
			InvokeFtpCallbackTimer (SCwdFinishedEvent, sftp_ctrl, sftp_success, m_state = SCwdFinished, (void**) m_targetPath);
			return;
		}

		switch (len)
		{
		case LIBSSH2_ERROR_EAGAIN:
			if (g_debug)
				cout << "CWD AGAIN" << endl;
			m_requestHandler = &SftpClient::HandleCwdBusyReply;
			break;
		case LIBSSH2_ERROR_NONE:
			if (g_debug)
				cout << "CWD FINISHED" << endl;
			InvokeFtpCallbackTimer (SCwdFinishedEvent, sftp_ctrl, sftp_success, m_state = SCwdFinished, (void**) m_targetPath);
			break;
		case LIBSSH2_ERROR_BUFFER_TOO_SMALL:
			if (g_debug)
				cout << "CWD TOO SMALL" << endl;
			{
				int targetPathLen = m_targetPathLen;
				targetPathLen >>= 10;
				++targetPathLen;
				targetPathLen <<= 10;
				char* targetPath = (char*) malloc (targetPathLen);
				if (targetPath == 0)
				{
					m_requestHandler = 0;
					if (g_debug)
						cout << "ERROR CWD" << endl;
					string msg = "SSH2 cannot allocate memory for change working directory for ";
					msg += m_hostname;
					void* args [] =
						{ (void*) msg.c_str (), (void*) "malloc", (void*) (long) errno };
					InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
					// Dispose ("libssh2_sftp_symlink_ex");
					return;
				}
				if (m_targetPath != 0)
					free (m_targetPath);
				m_targetPath = targetPath;
				m_targetPathLen = targetPathLen;
			}
			continue;
		default:
		{
			m_requestHandler = 0;
			if (g_debug)
				cout << "ERROR CWD" << endl;
			string msg = "SSH2 cannot change working directory for ";
			msg += m_hostname;
			void* args [] =
				{ (void*) msg.c_str (), (void*) "libssh2_sftp_symlink_ex", (void*) (long) len };
			InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
			// Dispose ("libssh2_sftp_symlink_ex");
			return;
		}
			break;
		}
		break;
	}
}

void SftpClient::CheckTargetPath ()
{
	LIBSSH2_SFTP_ATTRIBUTES attrs;

	memset (&attrs, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
	switch (libssh2_sftp_stat (m_sftp, m_targetPath, &attrs))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "STAT AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleCwdBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "STAT FINISHED" << endl;
		InvokeFtpCallbackTimer (SCwdFinishedEvent, sftp_ctrl, sftp_success, m_state = SCwdFinished, (void**) m_targetPath);
		break;
	}
}

void SftpClient::ChangeToParentDirectory ()
{
	InvokeFtpCallback (SCdupPreparedEvent, sftp_ctrl, sftp_success, m_state = SCdupPrepared, 0);
	{
		void* args [] =
			{ (void*) "cdup", 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	m_remoteDirName = GetRemoteDirectoryPath ("..");
	_ChangeToParentDirectory ();
}

void SftpClient::_ChangeToParentDirectory ()
{
	if (g_debug)
		cout << "ChangeToParentDirectory (" << ")" << endl;

	int len;
	while (true)
	{
		if ((len = libssh2_sftp_symlink_ex (m_sftp, m_remoteDirName.c_str (), m_remoteDirName.size (), m_targetPath, m_targetPathLen,
			LIBSSH2_SFTP_REALPATH)) > 0)
		{
			m_targetPath [len] = 0;
			m_currentRemoteDir = m_targetPath;
			{
				void* args [] =
					{ (void*) "cdup = ", (void*) m_targetPath, 0 };
				InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
			}
			InvokeFtpCallbackTimer (SCdupFinishedEvent, sftp_ctrl, sftp_success, m_state = SCdupFinished, (void**) m_targetPath);
			return;
		}

		switch (len)
		{
		case LIBSSH2_ERROR_EAGAIN:
			if (g_debug)
				cout << "CDUP AGAIN" << endl;
			m_requestHandler = &SftpClient::HandleCdupBusyReply;
			break;
		case LIBSSH2_ERROR_NONE:
			if (g_debug)
				cout << "CDUP FINISHED" << endl;
			InvokeFtpCallbackTimer (SCdupFinishedEvent, sftp_ctrl, sftp_success, m_state = SCdupFinished, (void**) m_targetPath);
			break;
		case LIBSSH2_ERROR_BUFFER_TOO_SMALL:
			if (g_debug)
				cout << "CDUP TOO SMALL" << endl;
			{
				int targetPathLen = m_targetPathLen;
				targetPathLen >>= 10;
				++targetPathLen;
				targetPathLen <<= 10;
				char* targetPath = (char*) malloc (targetPathLen);
				if (targetPath == 0)
				{
					m_requestHandler = 0;
					if (g_debug)
						cout << "ERROR CDUP" << endl;
					string msg = "SSH2 cannot allocate memory for change to parent directory for ";
					msg += m_hostname;
					void* args [] =
						{ (void*) msg.c_str (), (void*) "malloc", (void*) (long) errno };
					InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
					// Dispose ("libssh2_sftp_symlink_ex");
					return;
				}
				if (m_targetPath != 0)
					free (m_targetPath);
				m_targetPath = targetPath;
				m_targetPathLen = targetPathLen;
			}
			continue;
		default:
		{
			m_requestHandler = 0;
			if (g_debug)
				cout << "ERROR CDUP" << endl;
			string msg = "SSH2 cannot change to parent directory for ";
			msg += m_hostname;
			void* args [] =
				{ (void*) msg.c_str (), (void*) "libssh2_sftp_symlink_ex", (void*) (long) len };
			InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
			// Dispose ("libssh2_sftp_symlink_ex");
			return;
		}
			break;
		}
		break;
	}
}

void SftpClient::MakeDirectory (string dir, int mode)
{
	InvokeFtpCallback (SMkdirPreparedEvent, sftp_ctrl, sftp_success, m_state = SMkdirPrepared, 0);
	{
		void* args [] =
			{ (void*) "mkdir ", (void*) dir.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "MakeDirectory (" << dir << ", " << mode << ")" << endl;
	m_remoteDirName = GetRemoteDirectoryPath (dir);
	m_dirMode = mode;
	MakeDirectory ();
}

void SftpClient::MakeDirectory ()
{
	int status;

	switch (status = libssh2_sftp_mkdir_ex (m_sftp, m_remoteDirName.c_str (), m_remoteDirName.size (), m_dirMode))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "MKDIR AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleMkdirBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "MKDIR FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "mkdir = ", (void*) m_remoteDirName.c_str (), 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallbackTimer (SMkdirFinishedEvent, sftp_ctrl, sftp_success, m_state = SMkdirFinished, (void**) m_remoteDirName.c_str ());
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR MKDIR, status = " << status << endl;
		string msg = "SSH2 mkdir failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_mkdir_ex", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_userauth_password");
		return;
	}
		break;
	}
}

void SftpClient::DeleteDirectory (string dir)
{
	InvokeFtpCallback (SRmdirPreparedEvent, sftp_ctrl, sftp_success, m_state = SRmdirPrepared, 0);
	{
		void* args [] =
			{ (void*) "rmdir ", (void*) dir.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "DeleteDirectory (" << dir << ")" << endl;
	m_remoteDirName = GetRemoteDirectoryPath (dir);
	DeleteDirectory ();
}

void SftpClient::DeleteDirectory ()
{
	int status;

	switch (status = libssh2_sftp_rmdir_ex (m_sftp, m_remoteDirName.c_str (), m_remoteDirName.size ()))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "RMDIR AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleRmdirBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "RMDIR FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "rmdir ok", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallbackTimer (SRmdirFinishedEvent, sftp_ctrl, sftp_success, m_state = SRmdirFinished, 0);
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR RMDIR, status = " << status << endl;
		string msg = "SSH2 rmdir failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_rmdir_ex", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_userauth_password");
		return;
	}
		break;
	}
}

void SftpClient::ListDirectory (string dir)
{
	InvokeFtpCallback (SListDirPreparedEvent, sftp_ctrl, sftp_success, m_state = SListDirPrepared, 0);
	if (g_debug)
		cout << "ListDirectory (" << dir << ")" << endl;
	m_remoteDirName = GetRemoteDirectoryPath (dir);
	{
		void* args [] =
			{ (void*) "list dir ", (void*) m_remoteDirName.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	ListDirectory ();
}

void SftpClient::ListDirectory ()
{
	if ((m_dirHandle = libssh2_sftp_opendir (m_sftp, m_remoteDirName.c_str())) != 0)
	{
		if (g_debug)
			cout << "LIST DIR FINISHED" << endl;
		ReadDirectoryEntry ();
		return;
	}

	int status;
	switch (status = libssh2_session_last_errno (m_session))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "LIST DIR AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleListDirBusyReply;
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR LDIR, status = " << status << endl;
		string msg = "SSH2 list directory failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_opendir", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_opendir");
		return;
	}
		break;
	}
}

void SftpClient::ReadDirectoryEntry ()
{
	char buffer [1024];
	char longentry [1024];
	LIBSSH2_SFTP_ATTRIBUTES attrs;
	int len, status;

	while ((len = libssh2_sftp_readdir_ex (m_dirHandle, buffer, 1024, longentry, 1024, &attrs)) >= 0)
	{
		if (len == 0)
		{
			CloseDirHandle ();
			return;
		}
		else
		{
			buffer [len] = 0;
			void* args [] =
				{ (void*) buffer, (void*) longentry, (void*) &attrs };
			{
				void* args [] =
					{ (void*) longentry, 0 };
				InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
			}
			InvokeFtpCallback (SListDirProgressEvent, sftp_ctrl, sftp_success, m_state = SListDirProgress, args);
		}
	}

	switch (status = libssh2_session_last_errno (m_session))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "READ DIR AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleReadDirEntryReply;
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR READ DIR, status = " << status << endl;
		string msg = "SSH2 list directory failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_opendir", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_opendir");
		return;
	}
		break;
	}
}

void SftpClient::CloseDirHandle ()
{
	switch (libssh2_sftp_close_handle (m_dirHandle))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "CLOSE DIR BUSY" << endl;
		m_requestHandler = &SftpClient::HandleListDirCloseReply;
		break;
	case LIBSSH2_ERROR_NONE:
		m_dirHandle = 0;
		if (g_debug)
			cout << "CLOSE DIR FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "list dir ok", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallbackTimer (SListDirFinishedEvent, sftp_ctrl, sftp_success, m_state = SListDirFinished, 0);
		break;
	default:
		break;
	}
}

void SftpClient::GetLocalDirectory (string localDirName)
{
	char* path = realpath (localDirName.c_str (), 0);
	if (path == 0)
		return;
	m_currentLocalDir = path;
	free (path);
}

void SftpClient::PrintLocalWorkingDirectory ()
{
	string localDirName = m_currentLocalDir;
	{
		void* args [] =
			{ (void*) "pwd", 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	GetLocalDirectory (localDirName);
	{
		void* args [] =
			{ (void*) "lpwd = ", (void*) m_currentLocalDir.c_str (), 0 };
		InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
	}
	InvokeFtpCallbackTimer (SLPwdFinishedEvent, sftp_ctrl, sftp_success, m_state = SLPwdPrepared, (void**) m_currentLocalDir.c_str ());
}

void SftpClient::ChangeLocalWorkingDirectory (string dir)
{
	{
		void* args [] =
			{ (void*) "lcwd ", (void*) dir.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	string localDirName;
	if (dir.empty ())
		localDirName = m_currentLocalDir;
	else
	{
		if (dir [0] == '/')
			localDirName = "";
		else
		{
			localDirName = m_currentLocalDir;
			localDirName += '/';
		}
		localDirName += dir;
	}
	GetLocalDirectory (localDirName);
	{
		void* args [] =
			{ (void*) "lcwd = ", (void*) m_currentLocalDir.c_str (), 0 };
		InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
	}
	InvokeFtpCallbackTimer (SLCwdFinishedEvent, sftp_ctrl, sftp_success, m_state = SLCwdPrepared, (void**) m_currentLocalDir.c_str ());
}

void SftpClient::ChangeToLocalParentDirectory ()
{
	{
		void* args [] =
			{ (void*) "lcdup", 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	string localDirName = m_currentLocalDir;
	m_currentLocalDir += "/..";
	GetLocalDirectory (localDirName);
	{
		void* args [] =
			{ (void*) "lcdup = ", (void*) m_currentLocalDir.c_str (), 0 };
		InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
	}
	InvokeFtpCallbackTimer (SLCdupFinishedEvent, sftp_ctrl, sftp_success, m_state = SLCdupPrepared, (void**) m_currentLocalDir.c_str ());
}

string SftpClient::GetLocalFilePath (string filePath)
{
	string localFile;

	if (!filePath.empty ())
	{
		if (filePath [0] == '/')
			localFile = "";
		else
		{
			localFile = m_currentLocalDir;
			localFile += '/';
		}
		localFile += filePath;
	}
	else
		localFile = "";

	return localFile;
}

string SftpClient::GetRemoteDirectoryPath (string remoteDir)
{
	string remoteDirPath;

	if (!remoteDir.empty ())
	{
		if (remoteDir [0] == '/')
			remoteDirPath = "";
		else
		{
			remoteDirPath = m_currentRemoteDir;
			remoteDirPath += '/';
		}
		remoteDirPath += remoteDir;
	}
	else
		remoteDirPath = m_currentRemoteDir;

	return remoteDirPath;
}

string SftpClient::GetRemoteFilePath (string filePath)
{
	string remoteFile;

	if (!filePath.empty ())
	{
		if (filePath [0] == '/')
			remoteFile = "";
		else
		{
			remoteFile = m_currentRemoteDir;
			remoteFile += '/';
		}
		remoteFile += filePath;
	}
	else
		remoteFile = "";

	return remoteFile;
}

void SftpClient::PutFile (string sourceFile, string targetFile, long mode)
{
	InvokeFtpCallback (SPutFilePreparedEvent, sftp_ctrl, sftp_success, m_state = SPutFilePrepared, 0);
	{
		void* args [] =
			{ (void*) "put ", (void*) sourceFile.c_str (), (void*) ", ", (void*) targetFile.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "PutFile (" << sourceFile << ", " << targetFile << ")" << endl;

	m_sourceFile = GetLocalFilePath (sourceFile);
	m_targetFile = GetRemoteFilePath (targetFile);
	m_mode = mode;

	if ((m_localFileHandle = fopen (m_sourceFile.c_str (), "rb")) == 0)
	{
		if (g_debug)
			cout << "ERROR PUT FILE" << endl;
		string msg = "SSH2 put file failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "fopen", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("fopen");
		return;
	}

	PutFile ();
}

void SftpClient::PutFile ()
{
	unsigned long flags = LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC;

	if ((m_remoteFileHandle = libssh2_sftp_open (m_sftp, m_targetFile.c_str(), flags, m_mode)) != 0)
	{
		if (g_debug)
			cout << "PUT FILE OPEN" << endl;
		if (m_fileChunk == 0)
		{
			if (g_debug)
				cout << "READ FILE CHUNK MALLOC" << endl;
			size_t chunkSize = 16 * 1024;
			if ((m_fileChunk = (u_char*) malloc (chunkSize)) == 0)
			{
				if (g_debug)
					cout << "ERROR PUT FILE" << endl;
				string msg = "SSH2 put file failed for ";
				msg += m_hostname;
				void* args [] =
					{ (void*) msg.c_str (), (void*) "malloc", (void*) (long) errno };
				InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
				// Dispose ("libssh2_sftp_open");
				return;
			}
			m_fileChunkPtr = m_fileChunk;
			m_fileChunkEnd = m_fileChunk + chunkSize;
		}

		m_fileChunkPtr = m_fileChunk;
		ReadFileChunk ();
		return;
	}

	int status;
	switch (status = libssh2_session_last_errno (m_session))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "PUT FILE AGAIN" << endl;
		m_requestHandler = &SftpClient::HandlePutFileBusyReply;
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR PUT FILE, status = " << status << endl;
		string msg = "SSH2 put file failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_open", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_open");
		return;
	}
		break;
	}
}

void SftpClient::ReadFileChunk ()
{
	if (g_debug)
		cout << "READ FILE CHUNK" << endl;
	size_t size;
	ssize_t chunkSpace = m_fileChunkEnd - m_fileChunkPtr;
	if ((size = fread (m_fileChunkPtr, 1, chunkSpace, m_localFileHandle)) <= 0)
	{
		if (size == 0)
		{
			if (g_debug)
				cout << "READ FILE CHUNK - NOTHING TO READ" << endl;
			fclose (m_localFileHandle);
			m_localFileHandle = 0;
			PutFileCloseHandle ();
			return;
		}
		if (g_debug)
			cout << "ERROR PUT FILE" << endl;
		string msg = "SSH2 put file failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "fread", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_open");
		return;
	}
	m_fileChunkPtr += size;
	if (g_debug)
		cout << "READ FILE CHUNK - read " << size << ", chunk " << chunkSpace << endl;

	PutFileChunk ();
}

void SftpClient::PutFileChunk ()
{
	ssize_t wsize;
	ssize_t chunkSpace = m_fileChunkPtr - m_fileChunk;
	bool in, out;

	if (g_debug)
		cout << "PUT FILE CHUNK - chunk " << chunkSpace << endl;
	if ((wsize = libssh2_sftp_write (m_remoteFileHandle, (char*) m_fileChunk, chunkSpace)) >= 0)
	{
		{
			void* args [] =
				{ (void*) "put in progress", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
		}
		InvokeFtpCallback (SPutFileProgressEvent, sftp_ctrl, m_status = sftp_success, m_state = SPutFileProgress, (void**) wsize);
		if (g_debug)
			cout << "PUT FILE CHUNK - write " << wsize << endl;
		if ((chunkSpace > wsize) && (wsize > 0))
		{
			memcpy (m_fileChunk, m_fileChunk + wsize, chunkSpace - wsize);
		}
		m_fileChunkPtr -= wsize;
		m_ctx->EnableDescriptor (m_connDes, EPOLLOUT);
		if (wsize == chunkSpace)
			m_replyHandler = &SftpClient::HandleReadFileChunkReply;
		else
			m_replyHandler = &SftpClient::HandlePutFileChunkReply;
		return;
	}

	switch (wsize)
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "PUT FILE CHUNK AGAIN" << endl;
		CheckIODirections (m_session, in, out);
		if (in)
			m_requestHandler = &SftpClient::HandlePutFileChunkReply;
		if (out)
		{
			m_ctx->EnableDescriptor (m_connDes, EPOLLOUT);
			m_replyHandler = &SftpClient::HandlePutFileChunkReply;
		}
		return;
		break;
	default:
		if (g_debug)
			cout << "ERROR PUT FILE, status = " << wsize << endl;
		string msg = "SSH2 put file failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_write", (void*) wsize };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_open");
		return;
		break;
	}
}

void SftpClient::PutFileCloseHandle ()
{
	bool in, out;
	switch (libssh2_sftp_close_handle (m_remoteFileHandle))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "CLOSE FILE BUSY" << endl;
		CheckIODirections (m_session, in, out);
		if (in)
			m_requestHandler = &SftpClient::HandlePutFileCloseReply;
		if (out)
		{
			m_ctx->EnableDescriptor (m_connDes, EPOLLOUT);
			m_replyHandler = &SftpClient::HandlePutFileCloseReply;
		}
		break;
	case LIBSSH2_ERROR_NONE:
		m_remoteFileHandle = 0;
		if (g_debug)
			cout << "CLOSE FILE FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "put ok", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallback (SPutFileFinishedEvent, sftp_ctrl, m_status = sftp_success, m_state = SPutFileFinished, 0);
		break;
	default:
		break;
	}
}

void SftpClient::GetFile (string sourceFile, string targetFile, long mode)
{
	InvokeFtpCallback (SGetFilePreparedEvent, sftp_ctrl, sftp_success, m_state = SGetFilePrepared, 0);
	{
		void* args [] =
			{ (void*) "get ", (void*) sourceFile.c_str (), (void*) ", ", (void*) targetFile.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "GetFile (" << sourceFile << ", " << targetFile << ")" << endl;

	m_sourceFile = GetLocalFilePath (sourceFile);
	m_targetFile = GetRemoteFilePath (targetFile);
	m_mode = mode;

	if ((m_localFileHandle = fopen (sourceFile.c_str (), "wb")) == 0)
	{
		if (g_debug)
			cout << "ERROR GET FILE" << endl;
		string msg = "SSH2 get file failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "fopen", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("fopen");
		return;
	}

	GetFile ();
}

void SftpClient::GetFile ()
{
	unsigned long flags = LIBSSH2_FXF_READ;

	if ((m_remoteFileHandle = libssh2_sftp_open (m_sftp, m_targetFile.c_str(), flags, 0)) != 0)
	{
		if (g_debug)
			cout << "GET FILE OPEN" << endl;
		if (m_fileChunk == 0)
		{
			if (g_debug)
				cout << "WRITE FILE CHUNK MALLOC" << endl;
			size_t chunkSize = 16 * 1024;
			if ((m_fileChunk = (u_char*) malloc (chunkSize)) == 0)
			{
				if (g_debug)
					cout << "ERROR GET FILE" << endl;
				string msg = "SSH2 put file failed for ";
				msg += m_hostname;
				void* args [] =
					{ (void*) msg.c_str (), (void*) "malloc", (void*) (long) errno };
				InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
				// Dispose ("libssh2_sftp_open");
				return;
			}
			m_fileChunkPtr = m_fileChunk;
			m_fileChunkEnd = m_fileChunk + chunkSize;
		}
		m_fileChunkPtr = m_fileChunk;

		GetFileChunk ();

		return;
	}

	int status;
	switch (status = libssh2_session_last_errno (m_session))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "GET FILE AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleGetFileBusyReply;
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR GET FILE, status = " << status << endl;
		string msg = "SSH2 get file failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_open", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_open");
		return;
	}
		break;
	}
}

void SftpClient::GetFileChunk ()
{
	ssize_t wsize;
	ssize_t chunkSpace = m_fileChunkEnd - m_fileChunkPtr;
	bool in, out;

	if (g_debug)
		cout << "GET FILE CHUNK - chunk " << chunkSpace << endl;
	while ((wsize = libssh2_sftp_read (m_remoteFileHandle, (char*) m_fileChunkPtr, chunkSpace)) >= 0)
	{
		if (wsize == 0)
		{
			if (g_debug)
				cout << "GET FILE CHUNK - NOTHING TO READ" << endl;
			fclose (m_localFileHandle);
			m_localFileHandle = 0;
			GetFileCloseHandle ();
			return;
		}
		m_fileChunkPtr += wsize;
		WriteFileChunk ();
		{
			void* args [] =
				{ (void*) "get file in progress", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
		}
		InvokeFtpCallback (SGetFileProgressEvent, sftp_ctrl, m_status = sftp_success, m_state = SGetFileProgress, (void**) wsize);
		if (g_debug)
			cout << "GET FILE CHUNK - read " << wsize << endl;
	}

	switch (wsize)
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "GET FILE CHUNK AGAIN" << endl;
		CheckIODirections (m_session, in, out);
		m_requestHandler = &SftpClient::HandleGetFileChunkReply;
		return;
		if (in)
		{
			if (g_debug)
				cout << "GET FILE CHUNK - IN AGAIN" << endl;
			m_requestHandler = &SftpClient::HandleGetFileChunkReply;
		}
		if (out)
		{
			if (g_debug)
				cout << "GET FILE CHUNK - OUT AGAIN" << endl;
			m_ctx->EnableDescriptor (m_connDes, EPOLLOUT);
			m_replyHandler = &SftpClient::HandleGetFileChunkReply;
		}
		return;
		break;
	default:
		if (g_debug)
			cout << "ERROR GET FILE, status = " << wsize << endl;
		string msg = "SSH2 get file failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_read", (void*) wsize };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_open");
		return;
		break;
	}
}

void SftpClient::WriteFileChunk ()
{
	ssize_t size;
	ssize_t chunkSize = m_fileChunkPtr - m_fileChunk;
	u_char* chunk = m_fileChunk;

	while ((size = fwrite (chunk, 1, chunkSize, m_localFileHandle)) > 0)
	{
		chunkSize -= size;
		chunk += size;
	}

	if (size < 0)
	{
		if (g_debug)
			cout << "ERROR GET FILE" << endl;
		string msg = "SSH2 get file failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "fwrite", (void*) (long) errno };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("fwrite");
	}

	if ((chunk != m_fileChunk) && (chunk != m_fileChunkPtr))
		memcpy (m_fileChunk, m_fileChunkPtr - chunkSize, chunkSize);
	m_fileChunkPtr = m_fileChunk + chunkSize;
}

void SftpClient::GetFileCloseHandle ()
{
	bool in, out;
	switch (libssh2_sftp_close_handle (m_remoteFileHandle))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "CLOSE FILE BUSY" << endl;
		CheckIODirections (m_session, in, out);
		if (in)
			m_requestHandler = &SftpClient::HandleGetFileCloseReply;
		if (out)
		{
			m_ctx->EnableDescriptor (m_connDes, EPOLLOUT);
			m_replyHandler = &SftpClient::HandleGetFileCloseReply;
		}
		break;
	case LIBSSH2_ERROR_NONE:
		m_remoteFileHandle = 0;
		if (g_debug)
			cout << "CLOSE FILE FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "get file ok", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallback (SGetFileFinishedEvent, sftp_ctrl, m_status = sftp_success, m_state = SGetFileFinished, 0);
		break;
	default:
		break;
	}
}

void SftpClient::RenameFile (string sourceFile, string targetFile)
{
	InvokeFtpCallback (SRenamePreparedEvent, sftp_ctrl, sftp_success, m_state = SRenamePrepared, 0);
	{
		void* args [] =
			{ (void*) "rename ", (void*) sourceFile.c_str (), (void*) ", ", (void*) targetFile.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "RenameFile (" << sourceFile << ", " << targetFile << ")" << endl;

	m_sourceFile = GetRemoteFilePath (sourceFile);
	m_targetFile = GetRemoteFilePath (targetFile);
	RenameFile ();
}

void SftpClient::RenameFile ()
{
	int status;
	switch (status = libssh2_sftp_rename (m_sftp, m_sourceFile.c_str(), m_targetFile.c_str()))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "RENAME AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleRenameBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "RENAME FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "rename ok", 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallbackTimer (SRenameFinishedEvent, sftp_ctrl, sftp_success, m_state = SRenameFinished, 0);
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR RENAME, status = " << status << endl;
		string msg = "SSH2 rename failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) (string ("SSH2 rename failed for ") + m_hostname).c_str (), (void*) "libssh2_sftp_rename", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_rename");
		return;
	}
		break;
	}
}

void SftpClient::RemoveFile (string targetFile)
{
	InvokeFtpCallback (SRemovePreparedEvent, sftp_ctrl, sftp_success, m_state = SRemovePrepared, 0);
	{
		void* args [] =
			{ (void*) "rm ", (void*) targetFile.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "RemoveFile (" << targetFile << ")" << endl;

	m_targetFile = GetRemoteFilePath (targetFile);
	RemoveFile ();
}

void SftpClient::RemoveFile ()
{
	int status;
	switch (status = libssh2_sftp_unlink (m_sftp, m_targetFile.c_str()))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "REMOVE AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleRemoveBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "REMOVE FINISHED" << endl;
		{
			void* args [] =
				{ (void*) "removed ", (void*) m_targetFile.c_str (), 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallbackTimer (SRemoveFinishedEvent, sftp_ctrl, sftp_success, m_state = SRemoveFinished, 0);
		break;
	default:
	{
		if (g_debug)
			cout << "ERROR REMOVE, status = " << status << endl;
		string msg = "SSH2 remove failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_unlink", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_unlink");
		return;
	}
		break;
	}
}

void SftpClient::ListFile (string targetFile)
{
	InvokeFtpCallback (SListPreparedEvent, sftp_ctrl, sftp_success, m_state = SListPrepared, 0);
	{
		void* args [] =
			{ (void*) "list ", (void*) targetFile.c_str (), 0 };
		InvokeFtpCallback (SRequestEvent, sftp_request, sftp_success, m_state, args);
	}
	if (g_debug)
		cout << "ListFile (" << targetFile << ")" << endl;

	m_targetFile = GetRemoteFilePath (targetFile);
	ListFile ();
}

void SftpClient::ListFile ()
{
	int status;
	LIBSSH2_SFTP_ATTRIBUTES attrs;

	switch (status = libssh2_sftp_stat (m_sftp, m_targetFile.c_str(), &attrs))
	{
	case LIBSSH2_ERROR_EAGAIN:
		if (g_debug)
			cout << "LIST AGAIN" << endl;
		m_requestHandler = &SftpClient::HandleListBusyReply;
		break;
	case LIBSSH2_ERROR_NONE:
		if (g_debug)
			cout << "LIST FINISHED" << endl;
		{
			void* args [] =
				{ (void*) m_targetFile.c_str (), 0 };
			InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
		}
		InvokeFtpCallback (SListFinishedEvent, sftp_ctrl, sftp_success, m_state = SListFinished, (void**) &attrs);
		break;
	default:
	{
		if (g_debug)
			cout << "LIST REMOVE, status = " << status << endl;
		string msg = "SSH2 list failed for ";
		msg += m_hostname;
		void* args [] =
			{ (void*) msg.c_str (), (void*) "libssh2_sftp_stat", (void*) (long) status };
		InvokeFtpCallback (SErrorEvent, sftp_ctrl, m_status = sftp_error, m_state, args);
		// Dispose ("libssh2_sftp_stat");
		return;
	}
		break;
	}
}

void SftpClient::HandleConnectReply (void* args [])
{
	m_ctx->DisableTimer (m_connTmr);
	m_connTmr = 0;
	{
		void* args [] =
			{ (void*) "connected", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_reply, sftp_success, m_state, args);
	}
	InvokeFtpCallback (SConnectedEvent, sftp_ctrl, sftp_success, m_state = SConnected, 0);
}

void SftpClient::HandleHandshakeBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "handshake in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	_Handshake ();
}

void SftpClient::HandleAuthPasswordBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "authentication in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	LoginPassword ();
}

void SftpClient::HandleAuthPublickeyBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "authentication in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	LoginPublickey ();
}

void SftpClient::HandleSftpBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "sftp initialization in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	_CreateFtp ();
}

void SftpClient::HandleHomeBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "home-dir in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	_GetHomeDirectory ();
}

void SftpClient::HandlePwdBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "pwd in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	_PrintWorkingDirectory ();
}

void SftpClient::HandleCwdBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "cwd in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	ChangeWorkingDirectory ();
}

void SftpClient::HandleCdupBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "cdup in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	_ChangeToParentDirectory ();
}

void SftpClient::HandleMkdirBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "mkdir in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	MakeDirectory ();
}

void SftpClient::HandleRmdirBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "rmdir in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	DeleteDirectory ();
}

void SftpClient::HandleListDirBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "list dir in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	ListDirectory ();
}

void SftpClient::HandleReadDirEntryReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "list dir-read in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	ReadDirectoryEntry ();
}

void SftpClient::HandleListDirCloseReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "list dir-close in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	CloseDirHandle ();
}

void SftpClient::HandlePutFileBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "put in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	PutFile ();
}

void SftpClient::HandlePutFileChunkReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "put-write in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	PutFileChunk ();
}

void SftpClient::HandleReadFileChunkReply (void* args [])
{
	ReadFileChunk ();
}

void SftpClient::HandlePutFileCloseReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "put-close in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	PutFileCloseHandle ();
}

void SftpClient::HandleGetFileBusyReply (void* args [])
{
	GetFile ();
}

void SftpClient::HandleGetFileChunkReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "get-read in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	GetFileChunk ();
}

void SftpClient::HandleGetFileCloseReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "get-close in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	GetFileCloseHandle ();
}

void SftpClient::HandleRenameBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "rename in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	RenameFile ();
}

void SftpClient::HandleRemoveBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "remove in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	RemoveFile ();
}

void SftpClient::HandleListBusyReply (void* args [])
{
	{
		void* args [] =
			{ (void*) "list in progress", 0 };
		InvokeFtpCallback (SReplyEvent, sftp_progress, sftp_success, m_state, args);
	}
	ListFile ();
}

bool	SftpTrash::g_debug = false;
long	SftpTrash::g_trashId = 0;

void printtrailer (bool force)
{
	if (!(SftpTrash::debug() || force))
		return;

	time_t now = time (0);
	struct tm lt = *localtime (&now);

	fprintf (stdout, "%04d-%02d-%02d %02d:%02d:%02d: ", lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec);
}

SftpTrash::SftpTrash
(
	MpxRunningContext* ctx,
		int connSocket,
		LIBSSH2_SESSION* session,
		LIBSSH2_SFTP* sftp,
		LIBSSH2_SFTP_HANDLE* dirHandle,
		LIBSSH2_SFTP_HANDLE* remoteFileHandle
) :
		m_ctx (ctx),
		m_connSocket (connSocket),
		m_session (session),
		m_sftp (sftp),
		m_dirHandle (dirHandle),
		m_remoteFileHandle (remoteFileHandle)
{
	printtrailer (false);
	if (g_debug)
	{
		m_trashId = ++g_trashId;
		cout << "*** SFTP-TRASH " << m_trashId << " CREATED";
		if (session != 0)
			cout << ", LIVE SESSION";
		if (sftp != 0)
			cout << ", LIVE SFTP";
		cout << endl;
	}
	m_stageTimer = 0;
	m_connTimer = 0;
	m_connHandle = 0;
	m_stage = InitialStage;
}

SftpTrash::~SftpTrash ()
{
	printtrailer (false);
	if (g_debug)
		cout << "*** SFTP-TRASH " << m_trashId << " DELETED" << endl;

	if (m_remoteFileHandle != 0)
		libssh2_sftp_close_handle (m_remoteFileHandle);
	m_remoteFileHandle = 0;

	if (m_dirHandle != 0)
		libssh2_sftp_close_handle (m_dirHandle);
	m_dirHandle = 0;

	if (m_sftp != 0)
		libssh2_sftp_shutdown (m_sftp);
	m_sftp = 0;

	if (m_session != 0)
		libssh2_session_disconnect (m_session, "");
	m_session = 0;

	if (m_connSocket > 0)
		close (m_connSocket);
	m_connSocket = 0;

	if (m_stageTimer != 0)
		m_ctx->DisableTimer (m_stageTimer);
	m_stageTimer = 0;

	if (m_connTimer != 0)
		m_ctx->DisableTimer (m_connTimer);
	m_connTimer = 0;

	if (m_connHandle != 0)
		m_ctx->RemoveDescriptor (m_connHandle);
	m_connHandle = 0;
}

void SftpTrash::CleanUp()
{
	InvokeStageOperation ();
}

void SftpTrash::HandleClientConnection (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	InvokeStageOperation ();
}

void SftpTrash::StageTimer (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	m_stageTimer = 0;
	InvokeStageOperation();
}

void SftpTrash::ConnectionTimer (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	m_connTimer = 0;
	printtrailer (false);
	if (g_debug)
		cout << "*** SFTP-TRASH " << m_trashId << " KILLED" << endl;
	if ((m_stage = (Stage) ((int) m_stage + 1)) >= FinalStage)
		delete	this;
	else
	{
		printtrailer (false);
		if (g_debug)
			cout << "*** SFTP-TRASH " << m_trashId << " FORCE " << m_stage << endl;
		if (m_stageTimer != 0)
			m_ctx->DisableTimer(m_stageTimer);
		m_stageTimer = m_ctx->RegisterTimer (m_ctx->realTime(), StageTimer, this, ctx_info);
	}
}

void SftpTrash::InvokeStageOperation()
{
	printtrailer (false);
	if (g_debug)
		cout << "*** SFTP-TRASH " << m_trashId << " STAGE " << m_stage << " INVOKED" << endl;

	int	reason = 0;
	m_ctx->DisableTimer (m_connTimer);
	m_connTimer = 0;
	switch (m_stage)
	{
	case	InitialStage:
		m_connHandle = m_ctx->RegisterDescriptor (EPOLLIN | EPOLLOUT, m_connSocket, HandleClientConnection, this, ctx_info);
		m_ctx->DisableDescriptor (m_connHandle, EPOLLOUT);
		m_stage = ReleaseRemoteFileHandleStage;
		reason = LIBSSH2_ERROR_NONE;
		break;
	case	ReleaseRemoteFileHandleStage:
		reason = ReleaseRemoteFileHandle ();
		break;
	case	ReleaseDirHandleStage:
		reason = ReleaseDirHandle ();
		break;
	case	ReleaseSftpStage:
		reason = ReleaseSftp ();
		break;
	case	ReleaseSessionStage:
		reason = ReleaseSession ();
		break;
	case	ReleaseConnSocketStage:
		reason = ReleaseConnSocket ();
		break;
	case	FinalStage:
		delete	this;
		return;
	}

	switch (reason)
	{
	case	LIBSSH2_ERROR_EAGAIN:
		{
			printtrailer (false);
			if (g_debug)
				cout << "*** SFTP-TRASH " << m_trashId << " EAGAIN STAGE " << m_stage << endl;
			struct	timespec	t = m_ctx->realTime();
			t.tv_sec += 10;
			m_connTimer = m_ctx->RegisterTimer (t, ConnectionTimer, this, ctx_info);
			t = m_ctx->realTime();
			if ((t.tv_nsec += 10 * 1000 * 1000) >= 1000 * 1000 * 1000)
			{
				t.tv_nsec -= 1000 * 1000 * 1000;
				t.tv_sec += 1;
			}
			if (m_stageTimer != 0)
				m_ctx->DisableTimer(m_stageTimer);
			m_stageTimer = m_ctx->RegisterTimer (t, StageTimer, this, ctx_info);
		}
		break;
	default:
		if (reason != LIBSSH2_ERROR_NONE)
		{
			printtrailer (false);
			if (g_debug)
				cout << "*** SFTP-TRASH " << m_trashId << " ERROR " << reason << " STAGE " << m_stage << endl;
		}
		if (m_stageTimer != 0)
			m_ctx->DisableTimer(m_stageTimer);
		m_stageTimer = m_ctx->RegisterTimer (m_ctx->realTime(), StageTimer, this, ctx_info);
		break;
	}

	if (m_session != 0)
	{
		bool	in, out;
		CheckIODirections(m_session, in, out);
		if (out)
			m_ctx->EnableDescriptor (m_connHandle, EPOLLOUT);
		else
			m_ctx->DisableDescriptor (m_connHandle, EPOLLOUT);
	}
	else
		m_ctx->DisableDescriptor (m_connHandle, EPOLLOUT);
}

int	SftpTrash::ReleaseRemoteFileHandle ()
{
	if (m_remoteFileHandle == 0)
	{
		m_stage = ReleaseDirHandleStage;
		return	LIBSSH2_ERROR_NONE;
	}

	int	result;
	switch (result = libssh2_sftp_close_handle (m_remoteFileHandle))
	{
	case LIBSSH2_ERROR_EAGAIN:
		break;
	case LIBSSH2_ERROR_NONE:
		m_remoteFileHandle = 0;
		m_stage = ReleaseDirHandleStage;
		break;
	default:
		m_stage = FinalStage;
		break;
	}

	return	result;
}

int SftpTrash::ReleaseDirHandle ()
{
	if (m_dirHandle == 0)
	{
		m_stage = ReleaseSftpStage;
		return	LIBSSH2_ERROR_NONE;
	}

	int	result;
	switch (result = libssh2_sftp_close_handle (m_dirHandle))
	{
	case LIBSSH2_ERROR_EAGAIN:
		break;
	case LIBSSH2_ERROR_NONE:
		m_dirHandle = 0;
		m_stage = ReleaseSftpStage;
		break;
	default:
		m_stage = FinalStage;
		break;
	}

	return	result;
}

int SftpTrash::ReleaseSftp ()
{
	if (m_sftp == 0)
	{
		m_stage = ReleaseSessionStage;
		return	LIBSSH2_ERROR_NONE;
	}

	int	result;
	switch (result = libssh2_sftp_shutdown (m_sftp))
	{
	case LIBSSH2_ERROR_EAGAIN:
		break;
	case LIBSSH2_ERROR_NONE:
		printtrailer (false);
		if (g_debug)
			cout << "*** SFTP-TRASH " << m_trashId << " SFTP SHUTDOWN" << endl;
		m_sftp = 0;
		m_stage = ReleaseSessionStage;
		break;
	default:
		m_stage = FinalStage;
		break;
	}

	return	result;
}

int SftpTrash::ReleaseSession ()
{
	if (m_session == 0)
	{
		m_stage = ReleaseConnSocketStage;
		return	LIBSSH2_ERROR_NONE;
	}

	int	result;
	switch (result = libssh2_session_disconnect (m_session, ""))
	{
	case LIBSSH2_ERROR_EAGAIN:
		break;
	case LIBSSH2_ERROR_NONE:
		m_session = 0;
		m_stage = ReleaseConnSocketStage;
		break;
	default:
		m_stage = FinalStage;
		break;
	}

	return	result;
}

int SftpTrash::ReleaseConnSocket ()
{
	if (m_connSocket > 0)
		close (m_connSocket);
	m_connSocket = 0;
	if (m_connHandle)
		m_ctx->RemoveDescriptor(m_connHandle);
	m_connHandle = 0;
	m_stage = FinalStage;

	return	LIBSSH2_ERROR_NONE;
}

} /* namespace sftp */
