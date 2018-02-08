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

#include "FtpClient.h"

namespace sftp
{

FtpClient::FtpCallback* FtpClient::g_disposedScenario = 0;
FtpClient* FtpClient::g_disposedScenarioInitializer = new FtpClient (true);

FtpClient::FtpClient (bool initialize) :
			FtpClientInterface (FtpProtocol)
{
	if (!initialize)
		return;
	InitDisposedScenario ();
}

void FtpClient::InitDisposedScenario ()
{
	g_disposedScenario = new FtpClient::FtpCallback [FtpEventCount];
	memset (g_disposedScenario, 0, sizeof(FtpClient::FtpCallback [FtpEventCount]));

	g_disposedScenario [CtrlBusyTimerExpiredEvent] = FinalTimerExpiredEventHandler;
	g_disposedScenario [CtrlIdleTimerExpiredEvent] = FinalTimerExpiredEventHandler;
	g_disposedScenario [DataTimerExpiredEvent] = FinalTimerExpiredEventHandler;
	g_disposedScenario [ListenTimerExpiredEvent] = FinalTimerExpiredEventHandler;
}

void FtpClient::FinalTimerExpiredEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state, void* args [])
{
	Dispose ();
}

/****************************************************************/
/* Function:    FtpClient::verifyCallback ()                    */
/* In:          okCode - success of OpenSSL's validation process*/
/*              storeCtx - certificates to be verified          */
/* Out:         0 - verification failed                         */
/*              1 - verification succeeded                      */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: certificate verification callback               */
/****************************************************************/

int FtpClient::verifyCallback (int okCode, X509_STORE_CTX* storeCtx)
{
	if (g_debug)
		cout << "*** VERIFY CALLBACK: error = " << okCode << ", X509_STORE_CTX = " << storeCtx << " ***" << endl;
	X509* certificate = X509_STORE_CTX_get_current_cert (storeCtx);
	int depth = X509_STORE_CTX_get_error_depth (storeCtx);
	int err = X509_STORE_CTX_get_error (storeCtx);
	char data [256];

	if (g_debug)
		cout << "error depth = " << depth << endl;
	X509_NAME_oneline (X509_get_issuer_name (certificate), data, 256);
	if (g_debug)
		cout << "issuer = " << data << endl;
	X509_NAME_oneline (X509_get_subject_name (certificate), data, 256);
	if (g_debug)
		cout << "subject = " << data << endl;
	if (g_debug)
		cout << "error = " << err << ", " << X509_verify_cert_error_string (err) << endl;
	return ((err == 0) || (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT)) ? 1 : 0; // OK if no error or self signed certificate;
}

/****************************************************************/
/* Function:    FtpClient::FtpClient ()                         */
/* In:          ctx - running context object shared by set of   */
/*                    FtpClient instances                       */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initialize new instance of FtpClient class which*/
/*              is associated with single FTP client session    */
/****************************************************************/

FtpClient::FtpClient () :
	FtpClientInterface (FtpProtocol)
{
	m_ctx = 0;

	m_ctrlProtection = false;

	memset (m_ftpEvents, 0, sizeof(m_ftpEvents));
	m_scenarioEvents = NULL;
	m_appData = NULL;

	m_ctrlSocket = -1;
	m_dataSocket = -1;
	m_listenSocket = -1;

	m_sslCtx = 0;

	m_ctrlBioSocket = 0;
	m_dataBioSocket = 0;

	m_ctrlSslSocket = 0;
	m_dataSslSocket = 0;

	m_dataProtection = false;

	m_status = success;
	m_state = Null;

	m_replyHandler = NULL;
	m_ctrlDes = NULL;
	m_dataDes = NULL;
	m_listenDes = NULL;
	m_dataBusy = false;

	m_ctrlTmr = NULL;
	m_dataTmr = NULL;
	m_listenTmr = NULL;

	m_dataCommand = undefined;
	m_passiveEnabled = false;

	m_ftpFile = NULL;
	if ((m_ftpBuffer = m_ftpBufferPtr = (u_char*) malloc (4096)) == 0)
		m_ftpBufferEnd = 0;
	else
		m_ftpBufferEnd = m_ftpBuffer + 4096;
	m_dataProgress = false;
}

/****************************************************************/
/* Function:    FtpClient::~FtpClient ()                        */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: free all resources allocated by this instance   */
/****************************************************************/

FtpClient::~FtpClient ()
{
	UnregisterAllFtpCallbacks ();
	Dispose ();

	if (m_ftpBuffer != 0)
		free (m_ftpBuffer);
	m_ftpBuffer = m_ftpBufferPtr = m_ftpBufferEnd = 0;
}

/****************************************************************/
/* Function:    FtpClient::Dispose ()                           */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: free all resources allocated by this instance   */
/*              except callback handlers                        */
/****************************************************************/

void FtpClient::Dispose ()
{
	if (g_debug)
		cout << "FtpClient::Dispose" << endl;
	DisposeCtrl ();
	DisposeData ();
	DisposePort ();
	if (m_state != Disposed)
		InvokeFtpCallback (DisposedEvent, stop, m_status, m_state, NULL);
	m_state = Disposed;
}

/****************************************************************/
/* Function:    FtpClient::DisposeCtrl ()                       */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: free all resources allocated for FTP control    */
/*              connection: socket, I/O handler request/reply   */
/*              transfer, timer used to limit request/reply     */
/*              exchange time                                   */
/****************************************************************/

void FtpClient::DisposeCtrl ()
{
	if (m_ctrlSslSocket != 0)
		SSL_free (m_ctrlSslSocket);
	else if (m_ctrlBioSocket != 0)
		BIO_free (m_ctrlBioSocket);
	m_ctrlSslSocket = 0;
	m_ctrlBioSocket = 0;
	m_ctrlProtection = false;

	if (m_ctrlSocket > 0)
	{
		if (g_debug)
			cout << "*** DISPOSE CTRL CONNECTION socket = " << m_ctrlSocket << " ***" << endl;
		close (m_ctrlSocket);
	}
	m_ctrlSocket = -1;

	if (m_ctrlDes != NULL)
		m_ctx->RemoveDescriptor (m_ctrlDes);
	m_ctrlDes = NULL;

	if (m_ctrlTmr != NULL)
		m_ctx->DisableTimer (m_ctrlTmr);
	m_ctrlTmr = NULL;
}

/****************************************************************/
/* Function:    FtpClient::DisposeData ()                       */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: free all resources allocated for FTP data       */
/*              connection: socket, I/O handler for sent or     */
/*              received data, timer used to limit data exchange*/
/*              time, file descriptor used to store or retrieve */
/*              data                                            */
/****************************************************************/

void FtpClient::DisposeData ()
{
	if (m_dataSslSocket != 0)
		SSL_free (m_dataSslSocket);
	else if (m_dataBioSocket != 0)
		BIO_free (m_dataBioSocket);
	m_dataSslSocket = 0;
	m_dataBioSocket = 0;

	if (m_dataSocket > 0)
	{
		if (g_debug)
			cout << "*** DISPOSE DATA CONNECTION ***" << endl;
		close (m_dataSocket);
	}
	m_dataSocket = -1;

	if (m_dataDes != NULL)
		m_ctx->RemoveDescriptor (m_dataDes);
	m_dataDes = NULL;

	if (m_dataTmr != NULL)
		m_ctx->DisableTimer (m_dataTmr);
	m_dataTmr = NULL;

	if (m_ftpFile != NULL)
		fclose (m_ftpFile);
	m_ftpFile = NULL;
}

/****************************************************************/
/* Function:    FtpClient::DisposePort ()                       */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: free all resources allocated for listening      */
/*              socket used to establish active FTP data        */
/*              transfer: socket, I/O handler to establish      */
/*              active data connection, timer used to limit     */
/*              connection establishment time                   */
/****************************************************************/

void FtpClient::DisposePort ()
{
	if (m_listenSocket > 0)
	{
		if (g_debug)
			cout << "*** DISPOSE PORT CONNECTION ***" << endl;
		close (m_listenSocket);
	}
	m_listenSocket = -1;

	if (m_listenDes != NULL)
		m_ctx->RemoveDescriptor (m_listenDes);
	m_listenDes = NULL;

	if (m_listenTmr != NULL)
		m_ctx->DisableTimer (m_listenTmr);
	m_listenTmr = NULL;
}

/****************************************************************/
/* Function:    FtpClient::CtrlSocketHandler ()                 */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags used to distinguish I/O   */
/*                    operations                                */
/*              handler - I/O handler address - always equal to */
/*                    m_ctrlDes                                 */
/*              fd - socket descriptor associated with control  */
/*                    connection - always m_ctrlSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: function handles all I/O operations on FTP      */
/*              control connection. Flags indicate type of I/O  */
/*              operation. EPOLLIN indicates input operation:   */
/*              replies are read and handled. EPOLLOUT indicates*/
/*              output operation: requests are posted to FTP    */
/*              server                                          */
/****************************************************************/

void FtpClient::CtrlSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	char buff [1024];
	int n;

	if (flags & EPOLLIN)
	{
		ctx->DisableTimer (m_ctrlTmr);
		m_ctrlTmr = NULL;

		if (m_ctrlProtection)
		{
			while (true)
			{
				n = SSL_read (m_ctrlSslSocket, buff, 1024);
				int error;
				switch (error = SSL_get_error (m_ctrlSslSocket, n))
				{
				case SSL_ERROR_NONE:
					buff [n] = 0;
					HandleRawReply (buff, this);
					if (g_debug)
						cout << "*** READ HANDLER: ERROR NONE ***" << endl;
					break;
				case SSL_ERROR_WANT_READ:
					if (g_debug)
						cout << "*** READ HANDLER: ERROR WANT READ ***" << endl;
					continue;
				case SSL_ERROR_WANT_WRITE:
					if (g_debug)
						cout << "*** READ HANDLER: ERROR WANT WRITE ***" << endl;
					continue;
				default: // SSL_ERROR_ZERO_RETURN and others
					if (g_debug)
					{
						if (error == SSL_ERROR_ZERO_RETURN)
							cout << "*** READ HANDLER: ERROR ZERO RETURN ***" << endl;
						else
							cout << "*** READ HANDLER: ERROR DEFAULT ***" << endl;
					}
					break;
					{
						void* args [] =
							{ (void*) "cannot read from FTP control connection socket", (void*) "SSL_read", (void*) (long) errno };
						InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
						Dispose ();
					}
					return;
				}
				break;
			}
		}
		else
		{
			while ((n = recv (fd, buff, 1024, 0)) > 0)
			{
				buff [n] = 0;
				HandleRawReply (buff, this);
			}
			if ((n == 0) || ((errno != EAGAIN) && (errno != EWOULDBLOCK)))
			{
				void* args [] =
					{ (void*) "cannot read from FTP control connection socket", (void*) "recv", (void*) (long) errno };
				InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
				Dispose ();
				return;
			}
		}
		struct timespec t = ctx->realTime ();
		t.tv_sec += 10;
		m_ctrlTmr = ctx->RegisterTimer (t, CtrlIdleTimerHandler, this, ctx_info);
	}

	if (flags & EPOLLOUT)
	{
		char* ptr = (char*) m_ctrlRequest.c_str ();
		size_t size = m_ctrlRequest.size ();
		if (m_ctrlProtection)
		{
			ctx->DisableDescriptor (handler, EPOLLOUT);
			int reason = SSL_write (m_ctrlSslSocket, ptr, size);
			int error;
			switch (error = SSL_get_error (m_ctrlSslSocket, reason))
			{
			case SSL_ERROR_NONE:
				if (g_debug)
					cout << "*** WRITE HANDLER: ERROR NONE ***" << endl;
				break;
			case SSL_ERROR_WANT_READ:
				return;
			case SSL_ERROR_WANT_WRITE:
				ctx->EnableDescriptor (handler, EPOLLOUT);
				return;
			default:
				if (g_debug)
				{
					if (error == SSL_ERROR_ZERO_RETURN)
						cout << "*** WRITE HANDLER: ERROR ZERO RETURN ***" << endl;
					else
						cout << "*** WRITE HANDLER: ERROR DEFAULT ***" << endl;
				}
				{
					void* args [] =
						{ (void*) "cannot write to FTP control connection socket", (void*) "SSL_write", (void*) (long) errno };
					InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
					Dispose ();
				}
				return;
			}
		}
		else
		{
			if ((n = send (fd, ptr, size, 0)) <= 0)
			{
				void* args [] =
					{ (void*) "cannot write to FTP control connection socket", (void*) "send", (void*) (long) errno };
				InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
				Dispose ();
				return;
			}
			ctx->DisableDescriptor (handler, EPOLLOUT);
		}
	}
}

/****************************************************************/
/* Function:    FtpClient::CtrlSslSocketConnectHandler ()       */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags used to distinguish I/O   */
/*                    operations                                */
/*              handler - I/O handler address - always equal to */
/*                    m_ctrlDes                                 */
/*              fd - socket descriptor associated with control  */
/*                    connection - always m_ctrlSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: support function for asynchronous OpenSSL       */
/*              control connection establishment                */
/****************************************************************/

void FtpClient::CtrlSslSocketConnectHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	static int wantRead = 0, wantWrite = 0;
	ctx->DisableDescriptor (handler, EPOLLOUT);
	ctx->DisableDescriptor (handler, EPOLLIN);

	int reason = SSL_connect (m_ctrlSslSocket);
	int error;
	switch (error = SSL_get_error (m_ctrlSslSocket, reason))
	{
	case SSL_ERROR_NONE:
		if (g_debug)
			cout << "*** CTRL CONNECT HANDLER: ERROR NONE, read = " << wantRead << ", write = " << wantWrite << " ***" << endl;
		m_ctrlProtection = true;
		m_ctx->ChangeDescriptorHandler (m_ctrlDes, CtrlSocketHandler);
		ctx->EnableDescriptor (m_ctrlDes, EPOLLIN);
		InvokeFtpCallback (AuthFinishedEvent, ctrl, m_status = success, m_state, 0);
		break;
	case SSL_ERROR_WANT_READ:
		ctx->EnableDescriptor (handler, EPOLLIN);
		++wantRead;
		return;
	case SSL_ERROR_WANT_WRITE:
		ctx->EnableDescriptor (handler, EPOLLOUT);
		++wantWrite;
		return;
	default:
		if (g_debug)
		{
			if (error == SSL_ERROR_ZERO_RETURN)
				cout << "*** CTRL CONNECT HANDLER: ERROR ZERO RETURN, read = " << wantRead << ", write = " << wantWrite << " ***" << endl;
			else
				cout << "*** CTRL CONNECT HANDLER: ERROR DEFAULT, read = " << wantRead << ", write = " << wantWrite << " ***" << endl;
		}
		{
			void* args [] =
				{ (void*) "cannot establish SSL", (void*) "SSL_connect", (void*) (long) errno };
			InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
			Dispose ();
		}
		break;
	}
	wantRead = wantWrite = 0;
}

/****************************************************************/
/* Function:    FtpClient::DataSslSocketConnectHandler ()       */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLOUT)     */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: support function for asynchronous OpenSSL       */
/*              data connection establishment                   */
/****************************************************************/

void FtpClient::DataSslSocketConnectHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	static int wantRead = 0, wantWrite = 0;
	ctx->DisableDescriptor (handler, EPOLLOUT);
	ctx->DisableDescriptor (handler, EPOLLIN);

	int reason = SSL_connect (m_dataSslSocket);
	int error;
	switch (error = SSL_get_error (m_dataSslSocket, reason))
	{
	case SSL_ERROR_NONE:
		if (g_debug)
			cout << "*** DATA CONNECT HANDLER: ERROR NONE, read = " << wantRead << ", write = " << wantWrite << " ***" << endl;
		PrepareSslDataHandler ();
		break;
	case SSL_ERROR_WANT_READ:
		if (g_debug)
			cout << "*** DATA CONNECT HANDLER: ERROR WANT READ, read = " << wantRead << ", write = " << wantWrite << " ***" << endl;
		ctx->EnableDescriptor (handler, EPOLLIN);
		++wantRead;

		return;
	case SSL_ERROR_WANT_WRITE:
		if (g_debug)
			cout << "*** DATA CONNECT HANDLER: ERROR WANT WRITE, read = " << wantRead << ", write = " << wantWrite << " ***" << endl;
		ctx->EnableDescriptor (handler, EPOLLOUT);
		++wantWrite;
		return;
	default:
		if (g_debug)
		{
			if (error == SSL_ERROR_ZERO_RETURN)
				cout << "*** DATA CONNECT HANDLER: ERROR ZERO RETURN, read = " << wantRead << ", write = " << wantWrite << " ***" << endl;
			else
				cout << "*** DATA CONNECT HANDLER: ERROR DEFAULT, error = " << error << ",read = " << wantRead << ", write = " << wantWrite
					<< " ***" << endl;
		}
		{
			void* args [] =
				{ (void*) "cannot establish SSL", (void*) "SSL_connect", (void*) (long) errno };
			InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
			Dispose ();
		}
		break;
	}
	wantRead = wantWrite = 0;
}

/****************************************************************/
/* Function:    FtpClient::DataSocketHandler ()                 */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLOUT)     */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: handler for passive FTP data connection         */
/*              establishment. It prepares specialized data     */
/*              connection handlers and control requests which  */
/*              will initiate and handle data transfer commands */
/*              posted just before: different retrieve, store or*/
/*              list requests                                   */
/****************************************************************/

void FtpClient::DataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	ctx->RemoveDescriptor (m_dataDes);
	ctx->DisableTimer (m_dataTmr);
	m_dataDes = NULL;
	m_dataTmr = NULL;

	PrepareDataCommand ();
}

/****************************************************************/
/* Function:    FtpClient::RetrieveDataSocketHandler ()         */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLIN)      */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to store retrieved file contents           */
/****************************************************************/

void FtpClient::RetrieveDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n, m, size;

	while ((n = recv (fd, m_ftpBufferPtr, m_ftpBufferEnd - m_ftpBufferPtr, 0)) > 0)
	{
		void* args [] =
			{ (void*) m_ftpBufferPtr, (void*) (long) n };
		m_ftpBufferPtr [n] = 0;
		size = n + (m_ftpBufferPtr - m_ftpBuffer);
		if ((m = fwrite (m_ftpBuffer, 1, size, m_ftpFile)) < 0)
		{
			DisposeData ();
			if (m_dataBusy)
			{
				void* args [] =
					{ (char*) m_busyLine.c_str () };
				InvokeFtpCallback (RetrFinishedEvent, ctrl, m_status = success, m_state, args);
			}
			return;
		}
		if (m < size)
		{
			memcpy (m_ftpBuffer, m_ftpBuffer + m, size - m);
			m_ftpBufferPtr = m_ftpBuffer + (size - m);
		}
		else
			m_ftpBufferPtr = m_ftpBuffer;
		InvokeFtpCallback (RetrProgressEvent, data, m_status, m_state, args);
	}
	if ((n < 0) && ((errno == EWOULDBLOCK) || (errno == EAGAIN)))
	{
		m_dataProgress = true;
		return;
	}
	DisposeData ();
	if (m_dataBusy)
	{
		void* args [] =
			{ (char*) m_busyLine.c_str () };
		InvokeFtpCallback (RetrFinishedEvent, ctrl, m_status = success, m_state, args);
	}
}

/****************************************************************/
/* Function:    FtpClient::StoreDataSocketHandler ()            */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLOUT)     */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to send file contents which will be stored */
/*              on FTP server                                   */
/****************************************************************/

void FtpClient::StoreDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n = 0, m, size;

	if (m_ftpBufferPtr != m_ftpBufferEnd)
		if ((n = fread (m_ftpBufferPtr, 1, m_ftpBufferEnd - m_ftpBufferPtr, m_ftpFile)) <= 0)
		{
			DisposeData ();
			if (m_dataBusy)
			{
				void* args [] =
					{ (char*) m_busyLine.c_str () };
				InvokeFtpCallback (StorFinishedEvent, ctrl, m_status = success, m_state, args);
			}
			return;
		}
	size = n + (m_ftpBufferPtr - m_ftpBuffer);
	if ((m = send (fd, m_ftpBuffer, size, 0)) <= 0)
	{
		DisposeData ();
		if (m_dataBusy)
		{
			void* args [] =
				{ (char*) m_busyLine.c_str () };
			InvokeFtpCallback (StorFinishedEvent, ctrl, m_status = success, m_state, args);
		}
		return;
	}
	m_dataProgress = true;

	void* args [] =
		{ (void*) (m_ftpBufferPtr), (void*) (long) m };
	InvokeFtpCallback (StorProgressEvent, data, m_status, m_state, args);

	if (m < size)
	{
		memcpy (m_ftpBuffer, m_ftpBuffer + m, size - m);
		m_ftpBufferPtr = m_ftpBuffer + (size - m);
	}
	else
		m_ftpBufferPtr = m_ftpBuffer;
}

/****************************************************************/
/* Function:    FtpClient::StoreUniqueDataSocketHandler ()      */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLOUT)     */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to send file contents which will be stored */
/*              on FTP server in a uniquely named file          */
/****************************************************************/

void FtpClient::StoreUniqueDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n = 0, m, size;

	if (m_ftpBufferPtr != m_ftpBufferEnd)
		if ((n = fread (m_ftpBufferPtr, 1, m_ftpBufferEnd - m_ftpBufferPtr, m_ftpFile)) <= 0)
		{
			DisposeData ();
			if (m_dataBusy)
			{
				void* args [] =
					{ (char*) m_busyLine.c_str () };
				InvokeFtpCallback (StouFinishedEvent, ctrl, m_status = success, m_state, args);
			}
			return;
		}
	size = n + (m_ftpBufferPtr - m_ftpBuffer);
	if ((m = send (fd, m_ftpBuffer, size, 0)) <= 0)
	{
		DisposeData ();
		if (m_dataBusy)
		{
			void* args [] =
				{ (char*) m_busyLine.c_str () };
			InvokeFtpCallback (StouFinishedEvent, ctrl, m_status = success, m_state, args);
		}
		return;
	}
	m_dataProgress = true;

	void* args [] =
		{ (void*) (m_ftpBufferPtr), (void*) (long) m };
	InvokeFtpCallback (StouProgressEvent, data, m_status, m_state, args);

	if (m < size)
	{
		memcpy (m_ftpBuffer, m_ftpBuffer + m, size - m);
		m_ftpBufferPtr = m_ftpBuffer + (size - m);
	}
	else
		m_ftpBufferPtr = m_ftpBuffer;
}

/****************************************************************/
/* Function:    FtpClient::AppendDataSocketHandler ()           */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLOUT)     */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to send file contents which will be        */
/*              appended to selected file on FTP server         */
/****************************************************************/

void FtpClient::AppendDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n;
	while ((n = recv (fd, m_ftpBuffer, 4096, 0)) > 0)
	{
		void* args [] =
			{ (void*) m_ftpBuffer, (void*) (long) n };
		m_ftpBuffer [n] = 0;
		InvokeFtpCallback (AppeProgressEvent, data, m_status, m_state, args);
		return;
	}
	if ((n < 0) && ((errno == EWOULDBLOCK) || (errno == EAGAIN)))
	{
		m_dataProgress = true;
		return;
	}
	DisposeData ();
	if (m_dataBusy)
	{
		void* args [] =
			{ (char*) m_busyLine.c_str () };
		InvokeFtpCallback (AppeFinishedEvent, ctrl, m_status = success, m_state, args);
	}
}

/****************************************************************/
/* Function:    FtpClient::ListDataSocketHandler ()             */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLIN)      */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to retrieve directory listing for selected */
/*              directory on FTP server                         */
/****************************************************************/

void FtpClient::ListDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n;
	if ((n = recv (fd, m_ftpBuffer, 4096, 0)) > 0)
	{
		void* args [] =
			{ (void*) m_ftpBuffer, (void*) (long) n };
		m_ftpBuffer [n] = 0;
		InvokeFtpCallback (ListProgressEvent, data, m_status, m_state, args);
		return;
	}
	if ((n < 0) && ((errno == EWOULDBLOCK) || (errno == EAGAIN)))
	{
		m_dataProgress = true;
		return;
	}
	DisposeData ();
	if (m_dataBusy)
	{
		void* args [] =
			{ (char*) m_busyLine.c_str () };
		InvokeFtpCallback (ListFinishedEvent, ctrl, m_status = success, m_state, args);
	}
}

/****************************************************************/
/* Function:    FtpClient::NameListDataSocketHandler ()         */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLIN)      */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to retrieve list of file names in selected */
/*              directory on FTP server                         */
/****************************************************************/

void FtpClient::NameListDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n;
	while ((n = recv (fd, m_ftpBuffer, 4096, 0)) > 0)
	{
		void* args [] =
			{ (void*) m_ftpBuffer, (void*) (long) n };
		m_ftpBuffer [n] = 0;
		InvokeFtpCallback (NlstProgressEvent, data, m_status, m_state, args);
		return;
	}
	if ((n < 0) && ((errno == EWOULDBLOCK) || (errno == EAGAIN)))
	{
		m_dataProgress = true;
		return;
	}
	DisposeData ();
	if (m_dataBusy)
	{
		void* args [] =
			{ (char*) m_busyLine.c_str () };
		InvokeFtpCallback (NlstFinishedEvent, ctrl, m_status = success, m_state, args);
	}
}

/****************************************************************/
/* Function:    FtpClient::RetrieveSslDataSocketHandler ()      */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLIN)      */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to store retrieved file contents           */
/****************************************************************/

void FtpClient::RetrieveSslDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n = 0, m, size;
	int error;

	n = SSL_read (m_dataSslSocket, m_ftpBufferPtr, m_ftpBufferEnd - m_ftpBufferPtr);
	switch (error = SSL_get_error (m_dataSslSocket, n))
	{
	case SSL_ERROR_NONE:
	{
		void* args [] =
			{ (void*) m_ftpBufferPtr, (void*) (long) n };
		size = n + (m_ftpBufferPtr - m_ftpBuffer);
		if ((m = fwrite (m_ftpBuffer, 1, size, m_ftpFile)) < 0)
		{
			void* args [] =
				{ (void*) "cannot save data to local file", (void*) "fwrite", (void*) (long) errno };
			InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
			Dispose ();
		}
		InvokeFtpCallback (RetrProgressEvent, data, m_status, m_state, args);
		if (m < size)
		{
			memcpy (m_ftpBuffer, m_ftpBuffer + m, size - m);
			m_ftpBufferPtr = m_ftpBuffer + (size - m);
		}
		else
			m_ftpBufferPtr = m_ftpBuffer;
		if (g_debug)
			cout << "*** RETR HANDLER: ERROR NONE ***" << endl;
	}
		return;
	case SSL_ERROR_WANT_READ:
		if (g_debug)
			cout << "*** RETR HANDLER: ERROR WANT READ ***" << endl;
		return;
	case SSL_ERROR_WANT_WRITE:
		if (g_debug)
			cout << "*** RETR HANDLER: ERROR WANT WRITE ***" << endl;
		return;
	default: // SSL_ERROR_ZERO_RETURN and others
		break;
	}

	if ((error == SSL_ERROR_ZERO_RETURN) || (n == 0))
	{
		if (g_debug)
			cout << "*** RETR HANDLER: ERROR ZERO RETURN ***" << endl;
		DisposeData ();
		if (m_dataBusy)
		{
			void* args [] =
				{ (char*) m_busyLine.c_str () };
			InvokeFtpCallback (RetrFinishedEvent, ctrl, m_status = success, m_state, args);
		}
	}
	else
	{
		if (g_debug)
			cout << "*** RETR HANDLER: ERROR DEFAULT, ssl-error = " << error << ", sys-errno = " << errno << ", N = " << n << " ***"
				<< endl;
		void* args [] =
			{ (void*) "cannot read from FTP data socket", (void*) "SSL_read", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
	}
}

/****************************************************************/
/* Function:    FtpClient::StoreSslDataSocketHandler ()         */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLOUT)     */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to send file contents which will be stored */
/*              on FTP server                                   */
/****************************************************************/

void FtpClient::StoreSslDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n = 0, m, size;
	int error;

	if (m_ftpBufferPtr != m_ftpBufferEnd)
		if ((n = fread (m_ftpBufferPtr, 1, m_ftpBufferEnd - m_ftpBufferPtr, m_ftpFile)) <= 0)
		{
			DisposeData ();
			if (m_dataBusy)
			{
				void* args [] =
					{ (char*) m_busyLine.c_str () };
				InvokeFtpCallback (StorFinishedEvent, ctrl, m_status = success, m_state, args);
			}
			return;
		}
	size = n + (m_ftpBufferPtr - m_ftpBuffer);
	m = SSL_write (m_dataSslSocket, m_ftpBuffer, size);
	switch (error = SSL_get_error (m_dataSslSocket, n))
	{
	case SSL_ERROR_NONE:
		m_dataProgress = true;
		{
			void* args [] =
				{ (void*) m_ftpBuffer, (void*) (long) m };
			InvokeFtpCallback (StorProgressEvent, data, m_status, m_state, args);
			if (g_debug)
				cout << "*** STOR HANDLER: ERROR NONE ***" << endl;
		}
		if (m < size)
		{
			memcpy (m_ftpBuffer, m_ftpBuffer + m, size - m);
			m_ftpBufferPtr = m_ftpBuffer + (size - m);
		}
		else
			m_ftpBufferPtr = m_ftpBuffer;
		return;
	case SSL_ERROR_WANT_READ:
		if (g_debug)
			cout << "*** STOR HANDLER: ERROR WANT READ ***" << endl;
		return;
	case SSL_ERROR_WANT_WRITE:
		if (g_debug)
			cout << "*** STOR HANDLER: ERROR WANT WRITE ***" << endl;
		return;
	default: // SSL_ERROR_ZERO_RETURN and others
		break;
	}

	DisposeData ();
	if (m_dataBusy)
	{
		void* args [] =
			{ (char*) m_busyLine.c_str () };
		InvokeFtpCallback (StorFinishedEvent, ctrl, m_status = success, m_state, args);
	}
}

/****************************************************************/
/* Function:    FtpClient::StoreUniqueSslDataSocketHandler ()   */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLOUT)     */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to send file contents which will be stored */
/*              on FTP server in a uniquely named file          */
/****************************************************************/

void FtpClient::StoreUniqueSslDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n = 0, m, size;
	int error;

	if (m_ftpBufferPtr != m_ftpBufferEnd)
		if ((n = fread (m_ftpBufferPtr, 1, m_ftpBufferEnd - m_ftpBufferPtr, m_ftpFile)) <= 0)
		{
			DisposeData ();
			if (m_dataBusy)
			{
				void* args [] =
					{ (char*) m_busyLine.c_str () };
				InvokeFtpCallback (StouFinishedEvent, ctrl, m_status = success, m_state, args);
			}
			return;
		}
	size = n + (m_ftpBufferPtr - m_ftpBuffer);
	m = SSL_write (m_dataSslSocket, m_ftpBuffer, size);
	switch (error = SSL_get_error (m_dataSslSocket, n))
	{
	case SSL_ERROR_NONE:
		m_dataProgress = true;
		{
			void* args [] =
				{ (void*) m_ftpBuffer, (void*) (long) m };
			InvokeFtpCallback (StouProgressEvent, data, m_status, m_state, args);
			if (g_debug)
				cout << "*** STOU HANDLER: ERROR NONE ***" << endl;
		}
		if (m < size)
		{
			memcpy (m_ftpBuffer, m_ftpBuffer + m, size - m);
			m_ftpBufferPtr = m_ftpBuffer + (size - m);
		}
		else
			m_ftpBufferPtr = m_ftpBuffer;
		return;
	case SSL_ERROR_WANT_READ:
		if (g_debug)
			cout << "*** STOU HANDLER: ERROR WANT READ ***" << endl;
		return;
	case SSL_ERROR_WANT_WRITE:
		if (g_debug)
			cout << "*** STOU HANDLER: ERROR WANT WRITE ***" << endl;
		return;
	default: // SSL_ERROR_ZERO_RETURN and others
		break;
	}

	DisposeData ();
	if (m_dataBusy)
	{
		void* args [] =
			{ (char*) m_busyLine.c_str () };
		InvokeFtpCallback (StouFinishedEvent, ctrl, m_status = success, m_state, args);
	}
}

/****************************************************************/
/* Function:    FtpClient::AppendSslDataSocketHandler ()        */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLOUT)     */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to send file contents which will be        */
/*              appended to selected file on FTP server         */
/****************************************************************/

void FtpClient::AppendSslDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n;
	while ((n = recv (fd, m_ftpBuffer, 4096, 0)) > 0)
	{
		void* args [] =
			{ (void*) m_ftpBuffer, (void*) (long) n };
		m_ftpBuffer [n] = 0;
		InvokeFtpCallback (AppeProgressEvent, data, m_status, m_state, args);
		return;
	}
	if ((n < 0) && ((errno == EWOULDBLOCK) || (errno == EAGAIN)))
	{
		m_dataProgress = true;
		return;
	}
	DisposeData ();
	if (m_dataBusy)
	{
		void* args [] =
			{ (char*) m_busyLine.c_str () };
		InvokeFtpCallback (AppeFinishedEvent, ctrl, m_status = success, m_state, args);
	}
}

/****************************************************************/
/* Function:    FtpClient::ListSslDataSocketHandler ()          */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLIN)      */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to retrieve directory listing for selected */
/*              directory on FTP server                         */
/****************************************************************/

void FtpClient::ListSslDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n;
	int error;

	n = SSL_read (m_dataSslSocket, m_ftpBuffer, 4096);
	switch (error = SSL_get_error (m_dataSslSocket, n))
	{
	case SSL_ERROR_NONE:
	{
		void* args [] =
			{ (void*) m_ftpBuffer, (void*) (long) n };
		m_ftpBuffer [n] = 0;
		InvokeFtpCallback (ListProgressEvent, data, m_status, m_state, args);
		if (g_debug)
			cout << "*** LIST HANDLER: ERROR NONE ***" << endl;
	}
		return;
	case SSL_ERROR_WANT_READ:
		if (g_debug)
			cout << "*** LIST HANDLER: ERROR WANT READ ***" << endl;
		return;
	case SSL_ERROR_WANT_WRITE:
		if (g_debug)
			cout << "*** LIST HANDLER: ERROR WANT WRITE ***" << endl;
		return;
	default: // SSL_ERROR_ZERO_RETURN and others
		break;
	}

	if ((error == SSL_ERROR_ZERO_RETURN) || (n == 0))
	{
		if (g_debug)
			cout << "*** LIST HANDLER: ERROR ZERO RETURN ***" << endl;
		DisposeData ();
		if (m_dataBusy)
		{
			void* args [] =
				{ (char*) m_busyLine.c_str () };
			InvokeFtpCallback (ListFinishedEvent, ctrl, m_status = success, m_state, args);
		}
	}
	else
	{
		if (g_debug)
			cout << "*** LIST HANDLER: ERROR DEFAULT, ssl-error = " << error << ", sys-errno = " << errno << ", N = " << n << " ***"
				<< endl;
		void* args [] =
			{ (void*) "cannot read from FTP data socket", (void*) "SSL_read", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
	}
}

/****************************************************************/
/* Function:    FtpClient::NameListSslDataSocketHandler ()      */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLIN)      */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_dataSocket          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized data connection handler which is    */
/*              used to retrieve list of file names in selected */
/*              directory on FTP server                         */
/****************************************************************/

void FtpClient::NameListSslDataSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	int n;
	int error;

	n = SSL_read (m_dataSslSocket, m_ftpBuffer, 4096);
	switch (error = SSL_get_error (m_dataSslSocket, n))
	{
	case SSL_ERROR_NONE:
	{
		void* args [] =
			{ (void*) m_ftpBuffer, (void*) (long) n };
		m_ftpBuffer [n] = 0;
		InvokeFtpCallback (NlstProgressEvent, data, m_status, m_state, args);
		if (g_debug)
			cout << "*** NLST HANDLER: ERROR NONE ***" << endl;
	}
		return;
	case SSL_ERROR_WANT_READ:
		if (g_debug)
			cout << "*** NLST HANDLER: ERROR WANT READ ***" << endl;
		return;
	case SSL_ERROR_WANT_WRITE:
		if (g_debug)
			cout << "*** NLST HANDLER: ERROR WANT WRITE ***" << endl;
		return;
	default: // SSL_ERROR_ZERO_RETURN and others
		break;
	}

	if ((error == SSL_ERROR_ZERO_RETURN) || (n == 0))
	{
		if (g_debug)
			cout << "*** NLST HANDLER: ERROR ZERO RETURN ***" << endl;
		DisposeData ();
		if (m_dataBusy)
		{
			void* args [] =
				{ (char*) m_busyLine.c_str () };
			InvokeFtpCallback (NlstFinishedEvent, ctrl, m_status = success, m_state, args);
		}
	}
	else
	{
		if (g_debug)
			cout << "*** NLST HANDLER: ERROR DEFAULT, ssl-error = " << error << ", sys-errno = " << errno << ", N = " << n << " ***"
				<< endl;
		void* args [] =
			{ (void*) "cannot read from FTP data socket", (void*) "SSL_read", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
	}
}

/****************************************************************/
/* Function:    FtpClient::ListenSocketHandler ()               */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              flags - epoll() flags - not used (EPOLLIN)      */
/*              handler - I/O handler address - always equal to */
/*                    m_dataDes                                 */
/*              fd - socket descriptor associated with data     */
/*                    connection - always m_listenSocket        */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: specialized listen socket handler which is used */
/*              to establish 'active' data connnection initiated*/
/*              with PORT command                               */
/****************************************************************/

void FtpClient::ListenSocketHandler (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int fd)
{
	struct sockaddr_in addr;
	socklen_t slen = sizeof(struct sockaddr_in);
	memset (&addr, 0, sizeof(struct sockaddr_in));
	if ((m_dataSocket = accept (fd, (struct sockaddr*) &addr, &slen)) < 0)
	{
		void* args [] =
			{ (void*) "cannot establish FTP data connection", (void*) "accept", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}
	if ((flags = fcntl (m_dataSocket, F_GETFL, 0)) < 0)
	{
		void* args [] =
			{ (void*) "cannot acquire FTP data connection socket status", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}
	if (fcntl (m_dataSocket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		void* args [] =
			{ (void*) "cannot set nonblocking mode to FTP data connection socket status", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}

	DisposePort ();

	if (m_dataProtection)
		InitDataTlsHandshake ();
	else
		PrepareDataHandler ();
}

/****************************************************************/
/* Function:    FtpClient::CtrlIdleTimerHandler ()              */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              handler - timer handler address - always equal  */
/*                    to m_ctrlTmr                              */
/*              t - invocation time - current time              */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: this timer breaks inactive control connection   */
/*              which is waiting for FTP client request: client */
/*              is inactive too long                            */
/****************************************************************/

void FtpClient::CtrlIdleTimerHandler (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	if (m_dataSocket > 0)
	{
		t.tv_sec += 10;
		m_ctrlTmr = ctx->RegisterTimer (t, CtrlIdleTimerHandler, this, ctx_info);
		return;
	}
	InvokeFtpCallback (CtrlIdleTimerExpiredEvent, ctrl, m_status, m_state, NULL);
	Dispose (); // m_ctrlTmr = NULL;
}

/****************************************************************/
/* Function:    FtpClient::DataTimerHandler ()                  */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              handler - timer handler address - always equal  */
/*                    to m_dataTmr                              */
/*              t - invocation time - current time              */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: this timer breaks inactive data connection in   */
/*              either direction - when sending or receiving    */
/*              file                                            */
/****************************************************************/

void FtpClient::DataTimerHandler (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	if (m_dataProgress)
	{
		struct timespec t = m_ctx->realTime ();
		t.tv_sec += 10;
		m_dataTmr = m_ctx->RegisterTimer (t, DataTimerHandler, this, ctx_info);
		m_dataProgress = false;
		return;
	}
	DisposeData (); // m_dataTmr = NULL;
	InvokeFtpCallback (DataTimerExpiredEvent, ctrl, m_status, m_state, NULL);
}

/****************************************************************/
/* Function:    FtpClient::ListenTimerHandler ()                */
/* In:          ctx - running context shared by many FtpClient  */
/*                    instances                                 */
/*              handler - timer handler address - always equal  */
/*                    to m_listenTmr                            */
/*              t - invocation time - current time              */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: this timer breaks listening socket waiting too  */
/*              long to establish 'active' data connection      */
/****************************************************************/

void FtpClient::ListenTimerHandler (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	DisposeData ();
	DisposePort (); // m_listenTmr = NULL;
	InvokeFtpCallback (ListenTimerExpiredEvent, ctrl, m_status, m_state, NULL);
}

/****************************************************************/
/* Function:    FtpClient::RegisterFtpCallback ()               */
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

void FtpClient::RegisterFtpCallback (FtpEventIndex index, FtpCallback cb, void* appData)
{
	delete m_ftpEvents [index];
	m_ftpEvents [index] = new pair < FtpCallback, void* > (cb, appData);
}

/****************************************************************/
/* Function:    FtpClient::UnregisterFtpCallback ()             */
/* In:          index - event id: it is equal to index into     */
/*                    event callback table                      */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: function removes callback function and          */
/*              associated context data for given event         */
/****************************************************************/

void FtpClient::UnregisterFtpCallback (FtpEventIndex index)
{
	delete m_ftpEvents [index];
	m_ftpEvents [index] = NULL;
}

/****************************************************************/
/* Function:    FtpClient::UnregisterAllFtpCallbacks ()         */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: function clears event table for given FTP client*/
/*              object. No event will be reported any more      */
/****************************************************************/

void FtpClient::UnregisterAllFtpCallbacks ()
{
	int i;

	for (i = 0; i < FtpEventCount; ++i)
		delete m_ftpEvents [i];
	memset (m_ftpEvents, 0, sizeof(m_ftpEvents));
	m_scenarioEvents = 0;
}

/****************************************************************/
/* Function:    FtpClient::RegisterDynamicFtpCallbacks ()       */
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

void FtpClient::RegisterFtpScenario (FtpCallback* scenarioEvents, void*appData)
{
	m_scenarioEvents = scenarioEvents;
	m_appData = appData;
}

/****************************************************************/
/* Function:    FtpClient::UnregisterDynamicFtpCallbacks ()     */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: function removes current 'ftp scenario'.        */
/****************************************************************/

void FtpClient::UnregisterFtpScenario ()
{
	m_scenarioEvents = NULL;
}

/****************************************************************/
/* Function:    FtpClient::InvokeFtpCallback ()                 */
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
/*              event. Callbacks registered within ftp scenarios*/
/*              have precedence over those registered other way */
/****************************************************************/

void FtpClient::InvokeFtpCallback (FtpEventIndex index, FtpContext context, FtpStatus status, FtpState state, void* args [])
{
	void* nullArgs [] =
		{ (void*) index, 0 };
	if (m_scenarioEvents == NULL)
	{
		ftpcbdes* it = m_ftpEvents [index];
		if (it == NULL)
		{
			it = m_ftpEvents [FtpNullEvent];
			args = nullArgs;
		}
		if (it != NULL)
			it->first (this, context, status, state, args, it->second);
	}
	else
	{
		FtpCallback ftpcb = m_scenarioEvents [index];
		if (ftpcb == NULL)
		{
			ftpcb = m_scenarioEvents [FtpNullEvent];
			args = nullArgs;
		}
		if (ftpcb != NULL)
			ftpcb (this, context, status, state, args, m_appData);
	}
}

/****************************************************************/
/* Function:    FtpClient::Start ()                             */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: starting routine for ftp client. This is the    */
/*              prefered way to start FTP transfer              */
/****************************************************************/

void FtpClient::Start ()
{
	InvokeFtpCallback (StartupEvent, start, success, m_state = Startup, NULL);
}

/****************************************************************/
/* Function:    FtpClient::Stop ()                              */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: finishing routine for FTP transfer - used to    */
/*              free resources                                  */
/****************************************************************/

void FtpClient::Stop ()
{
	Dispose ();
}

/****************************************************************/
/* Function:    FtpClient::Connect ()                           */
/* In:          hostname - hostname of FTP server               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post connection request to FTP server using its */
/*              hostname                                        */
/****************************************************************/

void FtpClient::Connect (string hostname)
{
	m_hostname = hostname;
	m_ctrlSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_ctrlSocket < 0)
	{
		void* args [] =
			{ (void*) "cannot create FTP control connection socket", (void*) "socket", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
		return;
	}

	struct hostent* hostEntry = gethostbyname (hostname.c_str ());
	if (hostEntry == NULL)
	{
		void* args [] =
			{ (void*) (string ("cannot get host info ") + hostname).c_str (), (void*) "gethostbyname", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
		return;
	}

	int flags = fcntl (m_ctrlSocket, F_GETFL, 0);
	if (flags < 0)
	{
		void* args [] =
			{ (void*) "cannot acquire FTP control socket status", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
		return;
	}

	if (fcntl (m_ctrlSocket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		void* args [] =
			{ (void*) "cannot change FTP control socket to nonblocking mode", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
		return;
	}

	struct sockaddr_in addr;
	memset (&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (IPPORT_FTP);
	addr.sin_addr.s_addr = *((uint*) (hostEntry->h_addr_list [0]));

	int status;
	if ((status = connect (m_ctrlSocket, (struct sockaddr*) &addr, sizeof(struct sockaddr_in))) < 0)
	{
		if (errno != EINPROGRESS)
		{
			void* args [] =
				{ (void*) (string ("cannot connect to ") + hostname).c_str (), (void*) "connect", (void*) (long) errno };
			InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
			Dispose ();
			return;
		}
	}

	struct timespec t = m_ctx->realTime ();
	t.tv_sec += 10;
	m_ctrlTmr = m_ctx->RegisterTimer (t, CtrlIdleTimerHandler, this, ctx_info);
	m_ctrlDes = m_ctx->RegisterDescriptor (EPOLLIN, m_ctrlSocket, CtrlSocketHandler, this, ctx_info);
	m_replyHandler = &FtpClient::HandleConnectReply;
}

/****************************************************************/
/* Function:    FtpClient::Connect ()                           */
/* In:          hostname - hostname of FTP server               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post connection request to FTP server using its */
/*              hostname and address info (ipv4, ipv6, ...)     */
/****************************************************************/

void FtpClient::Connect (string hostname, sockaddr* addr, socklen_t addrlen)
{
	m_hostname = hostname;
	switch (addr->sa_family)
	{
	case AF_INET:
		m_ctrlSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
		break;
	case AF_INET6:
		m_ctrlSocket = socket (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		break;
	}
	if (m_ctrlSocket < 0)
	{
		void* args [] =
			{ (void*) "cannot create FTP control connection socket", (void*) "socket", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
		return;
	}

	int flags = fcntl (m_ctrlSocket, F_GETFL, 0);
	if (flags < 0)
	{
		void* args [] =
			{ (void*) "cannot acquire FTP control socket status", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
		return;
	}

	if (fcntl (m_ctrlSocket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		void* args [] =
			{ (void*) "cannot change FTP control socket to nonblocking mode", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		Dispose ();
		return;
	}

	int status;
	if ((status = connect (m_ctrlSocket, addr, addrlen)) < 0)
	{
		if (errno != EINPROGRESS)
		{
			void* args [] =
				{ (void*) (string ("cannot connect to ") + hostname).c_str (), (void*) "connect", (void*) (long) errno };
			InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
			Dispose ();
			return;
		}
	}

	struct timespec t = m_ctx->realTime ();
	t.tv_sec += 10;
	m_ctrlTmr = m_ctx->RegisterTimer (t, CtrlIdleTimerHandler, this, ctx_info);
	m_ctrlDes = m_ctx->RegisterDescriptor (EPOLLIN, m_ctrlSocket, CtrlSocketHandler, this, ctx_info);
	m_replyHandler = &FtpClient::HandleConnectReply;
}

/****************************************************************/
/* Function:    FtpClient::Login ()                             */
/* In:          user - FTP user name                            */
/*              password - FTP password                         */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post user request to FTP server                 */
/****************************************************************/

void FtpClient::Login (string user, string password)
{
	m_user = user;
	m_password = password;
	void* args [] =
		{ (void*) user.c_str (), (void*) password.c_str () };

	InvokeFtpCallback (UserPreparedEvent, ctrl, success, m_state = UserPrepared, args);
	m_replyHandler = &FtpClient::HandleUserReply;
	m_ctrlRequest = "user " + user + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::PreparePasswordRequest ()            */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post pass request to FTP server                 */
/****************************************************************/

void FtpClient::PreparePasswordRequest ()
{
	InvokeFtpCallback (PassPreparedEvent, ctrl, success, m_state = PassPrepared, NULL);
	m_replyHandler = &FtpClient::HandlePasswordReply;
	m_ctrlRequest = "pass " + m_password + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::PrepareAccountRequest ()             */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post acct request to FTP server                 */
/****************************************************************/

void FtpClient::PrepareAccountRequest ()
{
	InvokeFtpCallback (AcctPreparedEvent, ctrl, success, m_state = AcctPrepared, NULL);
	m_replyHandler = &FtpClient::HandleAccountReply;
	m_ctrlRequest = "acct " + m_password + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::ChangeWorkingDirectory ()            */
/* In:          pathName - directory path                       */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post cwd request to FTP server                  */
/****************************************************************/

void FtpClient::ChangeWorkingDirectory (string pathName)
{
	void* args [] =
		{ (void*) pathName.c_str (), 0 };

	InvokeFtpCallback (CwdPreparedEvent, ctrl, success, m_state = CwdPrepared, args);
	m_replyHandler = &FtpClient::HandleChangeWorkingDirectoryReply;
	m_ctrlRequest = "cwd " + pathName + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::ChangeToParentDirectory ()           */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post cdup request to FTP server                 */
/****************************************************************/

void FtpClient::ChangeToParentDirectory ()
{
	InvokeFtpCallback (CdupPreparedEvent, ctrl, success, m_state = CdupPrepared, NULL);
	m_replyHandler = &FtpClient::HandleChangeToParentDirectoryReply;
	m_ctrlRequest = "cdup\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::StructureMount ()                    */
/* In:          pathName - filesystem path name                 */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post smnt request to FTP server                 */
/****************************************************************/

void FtpClient::StructureMount (string pathName)
{
	void* args [] =
		{ (void*) pathName.c_str (), 0 };

	InvokeFtpCallback (SmntPreparedEvent, ctrl, success, m_state = SmntPrepared, args);
	m_replyHandler = &FtpClient::HandleStructureMountReply;
	m_ctrlRequest = "smnt " + pathName + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::Reinitialize ()                      */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post rein request to FTP server                 */
/****************************************************************/

void FtpClient::Reinitialize ()
{
	InvokeFtpCallback (ReinPreparedEvent, ctrl, success, m_state = ReinPrepared, NULL);
	m_replyHandler = &FtpClient::HandleReinitializeReply;
	m_ctrlRequest = "rein\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::Logout ()                            */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post quit request to FTP server                 */
/****************************************************************/

void FtpClient::Logout ()
{
	InvokeFtpCallback (QuitPreparedEvent, ctrl, success, m_state = QuitPrepared, NULL);
	m_replyHandler = &FtpClient::HandleLogoutReply;
	m_ctrlRequest = "quit\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::DataPort ()                          */
/* In:          hostAddress - IPV4 host address                 */
/*              port - TCP port number                          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post port request to FTP server to establish    */
/*              active data channel                             */
/****************************************************************/

void FtpClient::DataPort (ulong hostAddress, ulong port)
{
	byte h1, h2, h3, h4, p1, p2;
	h1 = (byte) (hostAddress & 0xff);
	hostAddress >>= 8;
	h2 = (byte) (hostAddress & 0xff);
	hostAddress >>= 8;
	h3 = (byte) (hostAddress & 0xff);
	hostAddress >>= 8;
	h4 = (byte) (hostAddress & 0xff);
	hostAddress >>= 8;
	p1 = (byte) (port & 0xff);
	port >>= 8;
	p2 = (byte) (port & 0xff);

	void* args [] =
		{ (void*) hostAddress, (void*) port };
	InvokeFtpCallback (PortPreparedEvent, ctrl, success, m_state = PortPrepared, args);
	m_replyHandler = &FtpClient::HandleDataPortReply;
	char request [128];
	sprintf (request, "port %d,%d,%d,%d,%d,%d\r\n", h1, h2, h3, h4, p1, p2);
	m_ctrlRequest = request;
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::Passive ()                           */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post pasv request to FTP server to establish    */
/*              passive data channel                            */
/****************************************************************/

void FtpClient::Passive ()
{
	InvokeFtpCallback (PasvPreparedEvent, ctrl, success, m_state = PasvPrepared, NULL);
	m_replyHandler = &FtpClient::HandlePassiveReply;
	m_ctrlRequest = "pasv\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::PreparePasvDataCommand ()            */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: establish passive data connection after it has  */
/*              been accepted by FTP server                     */
/****************************************************************/

void FtpClient::PreparePasvDataCommand ()
{
	size_t lpos = m_ctrlReply.find ('(');

	if (lpos == string::npos)
	{
		void* args [] =
			{ (void*) "PASV reply is ill structured: (A1,A2,A3,A4,P1,P2) expected", (void*) "", (void*) 0 };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		return;
	}

	int field [6];
	size_t pos;
	int i;
	for (i = 0; (i < 6) && ((pos = m_ctrlReply.find_first_of (",)", lpos + 1)) != string::npos); ++i, lpos = pos)
	{
		string part = m_ctrlReply.substr (lpos + 1, pos - lpos - 1);
		field [i] = atoi (part.c_str ());
	}
	if (i < 6)
	{
		void* args [] =
			{ (void*) "PASV reply is ill structured: (A1,A2,A3,A4,P1,P2) expected", (void*) "", (void*) 0 };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		return;
	}

	m_dataSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_dataSocket < 0)
	{
		void* args [] =
			{ (void*) "cannot create FTP data connection socket", (void*) "socket", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		return;
	}

	int flags = fcntl (m_dataSocket, F_GETFL, 0);
	if (flags < 0)
	{
		void* args [] =
			{ (void*) "cannot acquire FTP data socket status", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		return;
	}

	if (fcntl (m_dataSocket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		void* args [] =
			{ (void*) "cannot change FTP data socket to nonblocking mode", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
		return;
	}

	ulong laddr = 0;
	short port = 0;
	for (int i = 0; i < 4; ++i)
	{
		laddr <<= 8;
		laddr += field [i];
	}
	for (int i = 4; i < 6; ++i)
	{
		port <<= 8;
		port += field [i];
	}

	struct sockaddr_in addr;
	memset (&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	addr.sin_addr.s_addr = htonl (laddr);

	int status;
	if ((status = connect (m_dataSocket, (struct sockaddr*) &addr, sizeof(struct sockaddr_in))) < 0)
	{
		if (errno != EINPROGRESS)
		{
			void* args [] =
				{ (void*) (string ("cannot establish data connection to ") + m_hostname).c_str (), (void*) "connect", (void*) (long) errno };
			InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
			return;
		}
	}

	struct timespec t = m_ctx->realTime ();
	t.tv_sec += 10;
	m_dataTmr = m_ctx->RegisterTimer (t, DataTimerHandler, this, ctx_info);
	m_dataDes = m_ctx->RegisterDescriptor (EPOLLOUT, m_dataSocket, DataSocketHandler, this, ctx_info);
	m_dataProgress = false;
}

/****************************************************************/
/* Function:    FtpClient::PrepareDataCommand()                 */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post corresponding data transfer command after  */
/*              data channel has been established               */
/****************************************************************/

void FtpClient::PrepareDataCommand ()
{
	switch (m_dataCommand)
	{
	case retrieve:
		PrepareRetrRequest (m_dataArgs [0], m_dataArgs [1]);
		break;
	case store:
		PrepareStorRequest (m_dataArgs [0], m_dataArgs [1]);
		break;
	case storeUnique:
		PrepareStouRequest (m_dataArgs [0]);
		break;
	case append:
		PrepareAppeRequest (m_dataArgs [0], m_dataArgs [1]);
		break;
	case list:
		PrepareListRequest (m_dataArgs [0]);
		break;
	case nameList:
		PrepareNlstRequest (m_dataArgs [0]);
		break;
	default:
		void* args [] =
			{ (void*) "Invalid command id in PrepareDataCommand(), id = ", (void*) "", (void*) m_dataCommand };
		InvokeFtpCallback (ErrorEvent, ctrl, failure, m_state, args);
		break;
	}
}

/****************************************************************/
/* Function:    FtpClient::PrepareDataHandler ()                */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: prepare corresponding data transfer handler     */
/*              after data channel has been established         */
/****************************************************************/

void FtpClient::PrepareDataHandler ()
{
	m_dataBusy = false;
	m_ctx->RemoveDescriptor (m_dataDes);
	switch (m_dataCommand)
	{
	case retrieve:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLIN, m_dataSocket, RetrieveDataSocketHandler, this, ctx_info);
		break;
	case store:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLOUT, m_dataSocket, StoreDataSocketHandler, this, ctx_info);
		break;
	case storeUnique:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLOUT, m_dataSocket, StoreUniqueDataSocketHandler, this, ctx_info);
		break;
	case append:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLOUT, m_dataSocket, AppendDataSocketHandler, this, ctx_info);
		break;
	case list:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLIN, m_dataSocket, ListDataSocketHandler, this, ctx_info);
		break;
	case nameList:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLIN, m_dataSocket, NameListDataSocketHandler, this, ctx_info);
		break;
	default:
		m_dataDes = NULL;
		return;
	}

	struct timespec t = m_ctx->realTime ();
	t.tv_sec += 10;
	m_dataTmr = m_ctx->RegisterTimer (t, DataTimerHandler, this, ctx_info);
	m_dataProgress = false;
}

/****************************************************************/
/* Function:    FtpClient::PrepareSslDataHandler ()             */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post corresponding data transfer command after  */
/*              SSL data channel has been established           */
/****************************************************************/

void FtpClient::PrepareSslDataHandler ()
{
	m_ctx->RemoveDescriptor (m_dataDes);
	switch (m_dataCommand)
	{
	case retrieve:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLIN, m_dataSocket, RetrieveSslDataSocketHandler, this, ctx_info);
		break;
	case store:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLOUT, m_dataSocket, StoreSslDataSocketHandler, this, ctx_info);
		break;
	case storeUnique:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLOUT, m_dataSocket, StoreUniqueSslDataSocketHandler, this, ctx_info);
		break;
	case append:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLOUT, m_dataSocket, AppendSslDataSocketHandler, this, ctx_info);
		break;
	case list:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLIN, m_dataSocket, ListSslDataSocketHandler, this, ctx_info);
		break;
	case nameList:
		m_dataDes = m_ctx->RegisterDescriptor (EPOLLIN, m_dataSocket, NameListSslDataSocketHandler, this, ctx_info);
		break;
	default:
		m_dataDes = NULL;
		return;
	}

	struct timespec t = m_ctx->realTime ();
	t.tv_sec += 10;
	m_dataTmr = m_ctx->RegisterTimer (t, DataTimerHandler, this, ctx_info);
	m_dataProgress = false;
}

/****************************************************************/
/* Function:    FtpClient::PrepareRetrRequest ()                */
/* In:          localPathName - path name of file which will be */
/*                   used to store retrieved file               */
/*              remotePathName - name of remote file which      */
/*                   should be retrieved                        */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post retr request to FTP server                 */
/****************************************************************/

void FtpClient::PrepareRetrRequest (string localPathName, string remotePathName)
{
	void* args [] =
		{ (void*) localPathName.c_str (), (void*) remotePathName.c_str () };
	InvokeFtpCallback (RetrPreparedEvent, ctrl, success, m_state = RetrPrepared, args);
	if ((m_ftpFile = fopen (localPathName.c_str (), "w")) == NULL)
	{
		void* args [] =
			{ (void*) (string ("cannot open file for writing: ") + localPathName).c_str (), (void*) "fopen", (void*) (long) errno };
		DisposeData ();
		InvokeFtpCallback (ErrorEvent, ctrl, error, m_state, args);
		return;
	}
	m_replyHandler = &FtpClient::HandleRetrieveReply;
	m_ctrlRequest = "retr " + remotePathName + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::PrepareStorRequest ()                */
/* In:          localPathName - path name of file which should  */
/*                   be stored                                  */
/*              remotePathName - name of remote stored file     */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post stor request to FTP server                 */
/****************************************************************/

void FtpClient::PrepareStorRequest (string localPathName, string remotePathName)
{
	void* args [] =
		{ (void*) localPathName.c_str (), (void*) remotePathName.c_str () };
	InvokeFtpCallback (StorPreparedEvent, ctrl, success, m_state = StorPrepared, args);
	struct stat sbuf;
	if (stat (localPathName.c_str (), &sbuf) < 0)
	{
		void* args [] =
			{ (void*) (string ("cannot stat file: ") + localPathName).c_str (), (void*) "stat", (void*) (long) errno };
		DisposeData ();
		InvokeFtpCallback (ErrorEvent, ctrl, error, m_state, args);
		return;
	}
	if ((m_ftpFile = fopen (localPathName.c_str (), "r")) == NULL)
	{
		void* args [] =
			{ (void*) (string ("cannot open file for reading: ") + localPathName).c_str (), (void*) "fopen", (void*) (long) errno };
		DisposeData ();
		InvokeFtpCallback (ErrorEvent, ctrl, error, m_state, args);
		return;
	}

	m_replyHandler = &FtpClient::HandleStoreReply;
	m_ctrlRequest = "stor " + remotePathName + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::PrepareStouRequest ()                */
/* In:          localPathName - path name of file which should  */
/*                   be stored in unique, newly created remote  */
/*                   file                                       */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post stou request to FTP server                 */
/****************************************************************/

void FtpClient::PrepareStouRequest (string localPathName)
{
	void* args [] =
		{ (void*) localPathName.c_str () };

	InvokeFtpCallback (StouPreparedEvent, ctrl, success, m_state = StouPrepared, args);
	m_replyHandler = &FtpClient::HandleStoreUniqueReply;
	m_ctrlRequest = "stou\r\n";
	struct stat sbuf;
	if (stat (localPathName.c_str (), &sbuf) < 0)
	{
		void* args [] =
			{ (void*) (string ("cannot stat file: ") + localPathName).c_str (), (void*) "stat", (void*) (long) errno };
		DisposeData ();
		InvokeFtpCallback (ErrorEvent, ctrl, error, m_state, args);
		return;
	}
	if ((m_ftpFile = fopen (localPathName.c_str (), "r")) == NULL)
	{
		void* args [] =
			{ (void*) (string ("cannot open file for reading: ") + localPathName).c_str (), (void*) "fopen", (void*) (long) errno };
		DisposeData ();
		InvokeFtpCallback (ErrorEvent, ctrl, error, m_state, args);
		return;
	}
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::PrepareAppeRequest ()                */
/* In:          localPathName - path name of file which should  */
/*                   be append remotely                         */
/*              remotePathName - remote file name in which will */
/*                   be appended local file contents            */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post appe request to FTP server                 */
/****************************************************************/

void FtpClient::PrepareAppeRequest (string localPathName, string remotePathName)
{
	void* args [] =
		{ (void*) localPathName.c_str (), (void*) remotePathName.c_str () };
	InvokeFtpCallback (AppePreparedEvent, ctrl, success, m_state = AppePrepared, args);
	m_replyHandler = &FtpClient::HandleAppendReply;
	m_ctrlRequest = "appe " + remotePathName + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::PrepareListRequest ()                */
/* In:          remotePathName - remote file/directory name     */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post list request to FTP server                 */
/****************************************************************/

void FtpClient::PrepareListRequest (string remotePathName)
{
	void* args [] =
		{ remotePathName.empty () ? 0 : (void*) remotePathName.c_str () };
	InvokeFtpCallback (ListPreparedEvent, ctrl, success, m_state = ListPrepared, args);
	m_replyHandler = &FtpClient::HandleListReply;
	if (remotePathName.empty ())
		m_ctrlRequest = "list\r\n";
	else
		m_ctrlRequest = "list " + remotePathName + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::PrepareNlstRequest ()                */
/* In:          remotePathName - remote file/directory name     */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post nlst request to FTP server                 */
/****************************************************************/

void FtpClient::PrepareNlstRequest (string remotePathName)
{
	void* args [] =
		{ remotePathName.empty () ? 0 : (void*) remotePathName.c_str () };
	InvokeFtpCallback (NlstPreparedEvent, ctrl, success, m_state = NlstPrepared, args);
	m_replyHandler = &FtpClient::HandleNameListReply;
	if (remotePathName.empty ())
		m_ctrlRequest = "nlst\r\n";
	else
		m_ctrlRequest = "nlst " + remotePathName + "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::RepresentationType ()                */
/* In:          type - A (ascii), E (EBCDIC), I (image, binary) */
/*                     or L (local)                             */
/*              format - N (nonprint), T (telnet), C (asa)      */
/*              byteSize - size of single byte                  */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post type request to FTP server                 */
/****************************************************************/

void FtpClient::RepresentationType (FtpType type, FtpFormat format, int byteSize)
{
	if (type == undefinedType)
		type = ascii;
	if (format == undefinedFormat)
		format = nonprint;
	if (byteSize < 0)
		byteSize = 8;

	if (type != local)
	{
		byteSize = -1;
		if (type == image)
			format = undefinedFormat;
	}
	else
		format = undefinedFormat;

	void* args [] =
		{ (void*) (long) type, (void*) (long) format, (void*) (long) byteSize };
	InvokeFtpCallback (TypePreparedEvent, ctrl, success, m_state = TypePrepared, args);
	m_replyHandler = &FtpClient::HandleRepresentationTypeReply;
	m_ctrlRequest = "type";
	switch (type)
	{
	case ascii:
		m_ctrlRequest += " A";
		break;
	case ebcdic:
		m_ctrlRequest += " E";
		break;
	case image:
		m_ctrlRequest += " I";
		break;
	case local:
		m_ctrlRequest += " L";
		break;
	default:
		break;
	}
	switch (format)
	{
	case nonprint:
		m_ctrlRequest += " N";
		break;
	case telnet:
		m_ctrlRequest += " T";
		break;
	case asa:
		m_ctrlRequest += " C";
		break;
	default:
		break;
	}
	if (byteSize > 0)
		m_ctrlRequest += " " + byteSize;
	m_ctrlRequest += "\r\n";
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::FileStructure ()                     */
/* In:          str - f (file), r (record), p (page)            */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post stru request to FTP server                 */
/****************************************************************/

void FtpClient::FileStructure (FtpStructure str)
{
	if (str == undefinedStructure)
		str = file;

	void* args [] =
		{ (void*) str };
	InvokeFtpCallback (StruPreparedEvent, ctrl, success, m_state = StruPrepared, args);
	m_replyHandler = &FtpClient::HandleFileStructureReply;
	switch (str)
	{
	case file:
		m_ctrlRequest = "stru f\r\n";
		break;
	case record:
		m_ctrlRequest = "stru r\r\n";
		break;
	case page:
		m_ctrlRequest = "stru p\r\n";
		break;
	default:
		break;
	}
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::TransferMode ()                      */
/* In:          mode - s (stream), b (block), c (compressed)    */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: post mode request to FTP server                 */
/****************************************************************/

void FtpClient::TransferMode (FtpMode mode)
{
	if (mode == undefinedMode)
		mode = stream;

	void* args [] =
		{ (void*) mode };
	InvokeFtpCallback (ModePreparedEvent, ctrl, success, m_state = ModePrepared, args);
	m_replyHandler = &FtpClient::HandleTransferModeReply;
	switch (mode)
	{
	case stream:
		m_ctrlRequest = "mode s\r\n";
		break;
	case block:
		m_ctrlRequest = "mode b\r\n";
		break;
	case compressed:
		m_ctrlRequest = "mode c\r\n";
		break;
	default:
		break;
	}
	PostCtrlRequest ();
}

/****************************************************************/
/* Function:    FtpClient::Retrieve ()                          */
/* In:          localPathName - local file path                 */
/*              remotePathName - remote file name in current    */
/*                    working directory                         */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initiate retrieve request                       */
/****************************************************************/

void FtpClient::Retrieve (string localPathName, string remotePathName)
{
	m_dataCommand = retrieve;
	m_dataArgs [0] = localPathName;
	m_dataArgs [1] = remotePathName;
	if (m_passiveEnabled)
		InitiatePassiveDataTransfer ();
	else
		InitiateActiveDataTransfer ();
}

/****************************************************************/
/* Function:    FtpClient::Store ()                             */
/* In:          localPathName - local file path                 */
/*              remotePathName - remote file name in current    */
/*                    working directory                         */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initiate store request                          */
/****************************************************************/

void FtpClient::Store (string localPathName, string remotePathName)
{
	m_dataCommand = store;
	m_dataArgs [0] = localPathName;
	m_dataArgs [1] = remotePathName;
	if (m_passiveEnabled)
		InitiatePassiveDataTransfer ();
	else
		InitiateActiveDataTransfer ();
}

/****************************************************************/
/* Function:    FtpClient::StoreUnique ()                       */
/* In:          localPathName - local file path                 */
/*              /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initiate store unique request                   */
/****************************************************************/

void FtpClient::StoreUnique (string localPathName)
{
	m_dataCommand = storeUnique;
	m_dataArgs [0] = localPathName;
	if (m_passiveEnabled)
		InitiatePassiveDataTransfer ();
	else
		InitiateActiveDataTransfer ();
}

/****************************************************************/
/* Function:    FtpClient::Append ()                            */
/* In:          localPathName - local file path                 */
/*              remotePathName - remote file name in current    */
/*                    working directory                         */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initiate append request                         */
/****************************************************************/

void FtpClient::Append (string localPathName, string remotePathName)
{
	m_dataCommand = append;
	m_dataArgs [0] = localPathName;
	m_dataArgs [1] = remotePathName;
	if (m_passiveEnabled)
		InitiatePassiveDataTransfer ();
	else
		InitiateActiveDataTransfer ();
}

/****************************************************************/
/* Function:    FtpClient::List ()                              */
/* In:          remotePathName - remote file name in current    */
/*                    working directory                         */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initiate list request                           */
/****************************************************************/

void FtpClient::List (string remotePathName)
{
	m_dataCommand = list;
	m_dataArgs [0] = remotePathName;
	if (m_passiveEnabled)
		InitiatePassiveDataTransfer ();
	else
		InitiateActiveDataTransfer ();
}

/****************************************************************/
/* Function:    FtpClient::NameList ()                          */
/* In:          remotePathName - remote file name in current    */
/*                    working directory                         */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initiate named list request                     */
/****************************************************************/

void FtpClient::NameList (string remotePathName)
{
	m_dataCommand = nameList;
	m_dataArgs [0] = remotePathName;
	if (m_passiveEnabled)
		InitiatePassiveDataTransfer ();
	else
		InitiateActiveDataTransfer ();
}

/****************************************************************/
/* Function:    FtpClient::InitiatePassiveDataTransfer ()       */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initiate passive data transfer                  */
/****************************************************************/

void FtpClient::InitiatePassiveDataTransfer ()
{
	Passive ();
}

int getifaddr (int s, const char* name, struct sockaddr* addr)
{
	struct ifreq ifr;

	memset (&ifr, 0, sizeof(struct ifreq));
	strncpy (ifr.ifr_ifrn.ifrn_name, name, IFNAMSIZ);
	if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
		return -1;
	memcpy (addr, &ifr.ifr_ifru.ifru_addr, sizeof(struct sockaddr));
	return 0;
}

/****************************************************************/
/* Function:    FtpClient::InitiateActiveDataTransfer ()        */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initiate active data transfer                   */
/****************************************************************/

void FtpClient::InitiateActiveDataTransfer ()
{
	struct sockaddr saddr;
	if (getifaddr (m_ctrlSocket, "eth0", &saddr) < 0)
	{
		void* args [] =
			{ (void*) "cannot acquire ethernet interface info", (void*) "getifaddr", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}

	if (saddr.sa_family != AF_INET)
	{
		void* args [] =
			{ (void*) "ethernet interface address family is not AF_INET", (void*) "", (void*) 0 };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}

	if ((m_listenSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		void* args [] =
			{ (void*) "cannot create FTP active data listening socket", (void*) "socket", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}

	int flags = fcntl (m_listenSocket, F_GETFL, 0);
	if (flags < 0)
	{
		void* args [] =
			{ (void*) "cannot acquire FTP active data listening socket status", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}

	if (fcntl (m_listenSocket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		void* args [] =
			{ (void*) "cannot change FTP active data listening socket to nonblocking mode", (void*) "fcntl", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}

	struct sockaddr_in* addr = (struct sockaddr_in*) &saddr;
	addr->sin_port = 0;

	if (bind (m_listenSocket, (struct sockaddr*) addr, sizeof(struct sockaddr_in)) < 0)
	{
		void* args [] =
			{ (void*) "cannot bind FTP active data listening socket", (void*) "bind", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}

	socklen_t slen = sizeof(struct sockaddr_in);
	memset (addr, 0, sizeof(struct sockaddr_in));
	getsockname (m_listenSocket, (struct sockaddr*) addr, &slen);

	if (listen (m_listenSocket, 1) < 0)
	{
		void* args [] =
			{ (void*) "cannot listen FTP active data listening socket", (void*) "listen", (void*) (long) errno };
		InvokeFtpCallback (ErrorEvent, data, m_status, m_state, args);
		return;
	}

	m_listenDes = m_ctx->RegisterDescriptor (EPOLLIN, m_listenSocket, ListenSocketHandler, this, ctx_info);

	struct timespec now = m_ctx->realTime ();
	now.tv_sec += 10;
	m_listenTmr = m_ctx->RegisterTimer (now, ListenTimerHandler, this, ctx_info);
	DataPort ((addr->sin_addr.s_addr), (addr->sin_port));
}

void FtpClient::Allocate (int fileSize, int recordSize)
{
	char request [128];

	void* args [] =
		{ (void*) (long) fileSize, (void*) (long) recordSize };
	InvokeFtpCallback (AlloPreparedEvent, ctrl, success, m_state = AlloPrepared, args);
	m_replyHandler = &FtpClient::HandleAllocateReply;
	if (recordSize > 0)
		sprintf (request, "allo %d R %d\r\n", fileSize, recordSize);
	else
		sprintf (request, "allo %d\r\n", fileSize);
	m_ctrlRequest = request;
	PostCtrlRequest ();
}

void FtpClient::System ()
{
	InvokeFtpCallback (SystPreparedEvent, ctrl, success, m_state = SystPrepared, NULL);
	m_replyHandler = &FtpClient::HandleSystemReply;
	m_ctrlRequest = "syst\r\n";
	PostCtrlRequest ();
}

void FtpClient::Status (string pathName)
{
	void* args [] =
		{ (void*) pathName.c_str () };
	InvokeFtpCallback (StatPreparedEvent, ctrl, success, m_state = StatPrepared, args);
	m_replyHandler = &FtpClient::HandleStatusReply;
	if (pathName.empty ())
		m_ctrlRequest = "stat\r\n";
	else
		m_ctrlRequest = "stat " + pathName + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::Rename (string oldPathName, string newPathName)
{
	m_dataArgs [0] = oldPathName;
	m_dataArgs [1] = newPathName;

	void* args [] =
		{ (void*) oldPathName.c_str (), (void*) newPathName.c_str () };
	InvokeFtpCallback (RnfrPreparedEvent, ctrl, success, m_state = RnfrPrepared, args);
	m_replyHandler = &FtpClient::HandleRenameFromReply;
	m_ctrlRequest = "rnfr " + oldPathName + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::PrepareRenameToRequest ()
{
	InvokeFtpCallback (RntoPreparedEvent, ctrl, success, m_state = RntoPrepared, NULL);
	m_replyHandler = &FtpClient::HandleRenameToReply;
	m_ctrlRequest = "rnto " + m_dataArgs [1] + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::Abort ()
{
	InvokeFtpCallback (AborPreparedEvent, ctrl, success, m_state = AborPrepared, NULL);
	m_replyHandler = &FtpClient::HandleAbortReply;
	m_ctrlRequest = "abor\r\n";
	PostCtrlRequest ();
}

void FtpClient::Delete (string pathName)
{
	void* args [] =
		{ (void*) pathName.c_str () };
	InvokeFtpCallback (DelePreparedEvent, ctrl, success, m_state = DelePrepared, args);
	m_replyHandler = &FtpClient::HandleDeleteReply;
	m_ctrlRequest = "dele " + pathName + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::RemoveDirectory (string pathName)
{
	void* args [] =
		{ (void*) pathName.c_str () };
	InvokeFtpCallback (RmdPreparedEvent, ctrl, success, m_state = RmdPrepared, args);
	m_replyHandler = &FtpClient::HandleRemoveDirectoryReply;
	m_ctrlRequest = "rmd " + pathName + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::MakeDirectory (string pathName)
{
	void* args [] =
		{ (void*) pathName.c_str () };
	InvokeFtpCallback (MkdPreparedEvent, ctrl, success, m_state = MkdPrepared, args);
	m_replyHandler = &FtpClient::HandleMakeDirectoryReply;
	m_ctrlRequest = "mkd " + pathName + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::PrintWorkingDirectory ()
{
	InvokeFtpCallback (PwdPreparedEvent, ctrl, success, m_state = PwdPrepared, NULL);
	m_replyHandler = &FtpClient::HandlePrintWorkingDirectoryReply;
	m_ctrlRequest = "pwd\r\n";
	PostCtrlRequest ();
}

void FtpClient::Help (string help)
{
	void* args [] =
		{ help.empty () ? 0 : (void*) help.c_str () };
	InvokeFtpCallback (HelpPreparedEvent, ctrl, success, m_state = HelpPrepared, args);
	m_replyHandler = &FtpClient::HandleHelpReply;
	if (help.empty ())
		m_ctrlRequest = "help\r\n";
	else
		m_ctrlRequest = "help" + help + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::Noop ()
{
	InvokeFtpCallback (NoopPreparedEvent, ctrl, success, m_state = NoopPrepared, NULL);
	m_replyHandler = &FtpClient::HandleNoopReply;
	m_ctrlRequest = "noop\r\n";
	PostCtrlRequest ();
}

void FtpClient::Feature ()
{
	InvokeFtpCallback (FeatPreparedEvent, ctrl, success, m_state = FeatPrepared, NULL);
	m_replyHandler = &FtpClient::HandleFeatureReply;
	m_ctrlRequest = "feat\r\n";
	PostCtrlRequest ();
}

void FtpClient::Options (string command)
{
	InvokeFtpCallback (OptsPreparedEvent, ctrl, success, m_state = OptsPrepared, NULL);
	m_replyHandler = &FtpClient::HandleOptionsReply;
	m_ctrlRequest = "opts " + command + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::AuthMechanism (string command)
{
	FtpEventIndex event;
	FtpState state;

	m_replyHandler = &FtpClient::HandleSSLAuthReply;
	event = AuthPreparedEvent;
	state = AuthPrepared;

	if (command.find ("TLS", 0, 3) == 0)
	{
		if (m_sslCtx == 0)
		{
			m_sslCtx = SSL_CTX_new (TLSv1_method ());
			SSL_CTX_set_verify (m_sslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verifyCallback);
			SSL_CTX_set_verify_depth (m_sslCtx, 0);
		}
	}
#ifndef OPENSSL_NO_SSL2
	else if (command.find ("SSLv2", 0, 5) == 0)
	{
		if (m_sslCtx == 0)
		{
			m_sslCtx = SSL_CTX_new (SSLv2_method ());
			SSL_CTX_set_verify (m_sslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verifyCallback);
			SSL_CTX_set_verify_depth (m_sslCtx, 0);
		}
	}
#endif
	else if (command.find ("SSLv3", 0, 5) == 0)
	{
		if (m_sslCtx == 0)
		{
			m_sslCtx = SSL_CTX_new (SSLv3_method ());
			SSL_CTX_set_verify (m_sslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verifyCallback);
			SSL_CTX_set_verify_depth (m_sslCtx, 0);
		}
	}
	else if (command.find ("SSLv23", 0, 6) == 0)
	{
		if (m_sslCtx == 0)
		{
			m_sslCtx = SSL_CTX_new (SSLv23_method ());
			SSL_CTX_set_verify (m_sslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verifyCallback);
			SSL_CTX_set_verify_depth (m_sslCtx, 0);
		}
	}
	else
		m_replyHandler = &FtpClient::HandleAuthReply;

	InvokeFtpCallback (event, ctrl, success, m_state = state, NULL);
	m_ctrlRequest = "auth " + command + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::InitCtrlTlsHandshake ()
{
	m_ctrlBioSocket = BIO_new_socket (m_ctrlSocket, BIO_NOCLOSE);
	if (m_ctrlBioSocket != 0)
		if (g_debug)
			cout << "*** SUCCESS: CTRL BIO SOCKET CREATED ***" << endl;
	m_ctrlSslSocket = SSL_new (m_sslCtx);
	SSL_set_bio (m_ctrlSslSocket, m_ctrlBioSocket, m_ctrlBioSocket);

	m_ctx->ChangeDescriptorHandler (m_ctrlDes, CtrlSslSocketConnectHandler);
	m_ctx->EnableDescriptor (m_ctrlDes, EPOLLOUT);
}

void FtpClient::InitDataTlsHandshake ()
{
	m_dataBusy = false;
	m_dataBioSocket = BIO_new_socket (m_dataSocket, BIO_NOCLOSE);
	if (m_dataBioSocket != 0)
		if (g_debug)
			cout << "*** SUCCESS: DATA BIO SOCKET CREATED ***" << endl;
	m_dataSslSocket = SSL_new (m_sslCtx);
	SSL_set_bio (m_dataSslSocket, m_dataBioSocket, m_dataBioSocket);

	m_dataDes = m_ctx->RegisterDescriptor (EPOLLIN, m_dataSocket, DataSslSocketConnectHandler, this, ctx_info);
	m_ctx->EnableDescriptor (m_dataDes, EPOLLOUT);
}

void FtpClient::AuthData (string command)
{
	InvokeFtpCallback (AdatPreparedEvent, ctrl, success, m_state = AdatPrepared, NULL);
	m_replyHandler = &FtpClient::HandleAdatReply;
	m_ctrlRequest = "adat " + command + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::ProtectionLevel (string command)
{
	InvokeFtpCallback (ProtPreparedEvent, ctrl, success, m_state = ProtPrepared, NULL);
	m_replyHandler = &FtpClient::HandleProtReply;
	m_ctrlRequest = "prot " + command + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::ProtectionBufferSize (string command)
{
	InvokeFtpCallback (PbszPreparedEvent, ctrl, success, m_state = PbszPrepared, NULL);
	m_replyHandler = &FtpClient::HandlePbszReply;
	m_ctrlRequest = "pbsz " + command + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::ClearCommandChannel (string command)
{
	InvokeFtpCallback (CccPreparedEvent, ctrl, success, m_state = CccPrepared, NULL);
	m_replyHandler = &FtpClient::HandleCccReply;
	m_ctrlRequest = "ccc " + command + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::IntegrityProtectedCommand (string command)
{
	InvokeFtpCallback (MicPreparedEvent, ctrl, success, m_state = MicPrepared, NULL);
	m_replyHandler = &FtpClient::HandleMicReply;
	m_ctrlRequest = "mic " + command + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::ConfidentialityProtectedCommand (string command)
{
	InvokeFtpCallback (ConfPreparedEvent, ctrl, success, m_state = ConfPrepared, NULL);
	m_replyHandler = &FtpClient::HandleConfReply;
	m_ctrlRequest = "conf " + command + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::PrivacyProtectedCommand (string command)
{
	InvokeFtpCallback (EncPreparedEvent, ctrl, success, m_state = EncPrepared, NULL);
	m_replyHandler = &FtpClient::HandleEncReply;
	m_ctrlRequest = "enc " + command + "\r\n";
	PostCtrlRequest ();
}

void FtpClient::PostCtrlRequest ()
{
	void* args [] =
		{ (void*) m_ctrlRequest.c_str (), 0 };

	m_ctx->EnableDescriptor (m_ctrlDes, EPOLLOUT);
	InvokeFtpCallback (RequestEvent, request, success, m_state, args);
}

void FtpClient::HandleRawReply (char* buffer, void* appData)
{
	char* sptr;
	char* eptr;
	FtpClient* ftp = (FtpClient*) appData;

	for (sptr = buffer; (eptr = strstr (sptr, "\r\n")) != NULL; sptr = eptr + 2)
	{
		*eptr = 0;
		m_ctrlReply = sptr;
		void* args [2] =
			{ (void*) sptr, 0 };
		InvokeFtpCallback (ReplyEvent, reply, success, m_state, args);
		if (isdigit (*sptr) == 0)
			continue;
		if (sptr [3] == '-')
			continue;
		if (ftp->m_replyHandler == NULL)
			continue;
		(ftp->*(ftp->m_replyHandler)) ((const char*) sptr);
	}
}

void FtpClient::HandleConnectReply (const char*line)
{
	switch (line [0])
	{
	case '1':
		m_status = error;
		break;
	case '2':
		m_status = success;
		{
			struct sockaddr_in saddr, paddr;
			socklen_t slen = sizeof(struct sockaddr_in);
			socklen_t plen = sizeof(struct sockaddr_in);
			memset (&saddr, 0, sizeof(struct sockaddr_in));
			memset (&paddr, 0, sizeof(struct sockaddr_in));
			getsockname (m_ctrlSocket, (struct sockaddr*) &saddr, &slen);
			getpeername (m_ctrlSocket, (struct sockaddr*) &paddr, &plen);
			char* sockaddr = inet_ntoa (saddr.sin_addr);
			char* peeraddr = inet_ntoa (paddr.sin_addr);

			void* args [] =
				{ (void*) sockaddr, (void*) (long) saddr.sin_port, (void*) peeraddr, (void*) (long) paddr.sin_port };
			InvokeFtpCallback (ConnectedEvent, ctrl, success, m_state = Connected, args);
		}
		return;
	case '3':
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Connect failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, Connected, args);
}

void FtpClient::HandleUserReply (const char*line)
{
	switch (line [0])
	{
	case '1':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (ClientAuthenticatedEvent, ctrl, m_status = success, m_state = ClientAuthenticated, args);
	}
		return;
	case '3':
		m_status = success;
		PreparePasswordRequest ();
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "User Name (USER) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandlePasswordReply (const char*line)
{
	switch (line [0])
	{
	case '1':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (ClientAuthenticatedEvent, ctrl, m_status = success, m_state = ClientAuthenticated, args);
	}
		return;
	case '3':
		m_status = success;
//		PrepareAccountRequest ();
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Password (PASS) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleAccountReply (const char*line)
{
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (ClientAuthenticatedEvent, ctrl, m_status = success, m_state = ClientAuthenticated, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Account (ACCT) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleChangeWorkingDirectoryReply (const char*line)
{
	m_state = CwdFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (CwdFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Change Working Directory (CWD) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleChangeToParentDirectoryReply (const char*line)
{
	m_state = CdupFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (CdupFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Change to Parent Directory (CDUP) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleStructureMountReply (const char*line)
{
	m_state = SmntFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (SmntFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Structure Mount (SMNT) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleReinitializeReply (const char*line)
{
	m_state = ReinFinished;
	switch (line [0])
	{
	case '1':
		m_state = ReinPrepared;
		m_status = success;
		m_replyHandler = &FtpClient::HandleReinitializeReply;
		break;
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (ReinFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Reinitialize (REIN) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleLogoutReply (const char*line)
{
	m_state = QuitFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (QuitFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Logout (QUIT) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleDataPortReply (const char*line)
{
	m_state = PortFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (PortFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		PrepareDataCommand ();
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Data Port (PORT) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandlePassiveReply (const char*line)
{
	m_state = PasvFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (PasvFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		PreparePasvDataCommand ();
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Passive (PASV) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleRetrieveReply (const char*line)
{
	m_state = RetrFinished;
	switch (line [0])
	{
	case '1':
		m_status = success;
		m_state = RetrPrepared;
		m_replyHandler = &FtpClient::HandleRetrieveReply;
		if (m_dataProtection)
			InitDataTlsHandshake ();
		else
			PrepareDataHandler ();
		return;
	case '3':
		m_status = error;
		break;
	case '2':
		if (m_dataSocket < 0)
		{
			void* args [] =
				{ (void*) line };
			InvokeFtpCallback (RetrFinishedEvent, ctrl, m_status = success, m_state, args);
		}
		else
		{
			m_dataBusy = true;
			m_busyLine = line;
		}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Retrieve (RETR) failed", (void*) line, (void*) 0 };
	DisposeData ();
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleStoreReply (const char*line)
{
	m_state = StorFinished;
	switch (line [0])
	{
	case '1':
		m_status = success;
		m_state = StorPrepared;
		m_replyHandler = &FtpClient::HandleStoreReply;
		if (m_dataProtection)
			InitDataTlsHandshake ();
		else
			PrepareDataHandler ();
		return;
	case '3':
		m_status = error;
		break;
	case '2':
		if (m_dataSocket < 0)
		{
			void* args [] =
				{ (void*) line };
			InvokeFtpCallback (StorFinishedEvent, ctrl, m_status = success, m_state = StorFinished, args);
		}
		else
		{
			m_dataBusy = true;
			m_busyLine = line;
		}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Store (STOR) failed", (void*) line, (void*) 0 };
	DisposeData ();
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleStoreUniqueReply (const char*line)
{
	m_state = StouFinished;
	switch (line [0])
	{
	case '1':
		m_status = success;
		m_state = StouPrepared;
		m_replyHandler = &FtpClient::HandleStoreUniqueReply;
		if (m_dataProtection)
			InitDataTlsHandshake ();
		else
			PrepareDataHandler ();
		return;
	case '3':
		m_status = error;
		break;
	case '2':
		if (m_dataSocket < 0)
		{
			void* args [] =
				{ (void*) line };
			InvokeFtpCallback (StouFinishedEvent, ctrl, m_status = success, m_state, args);
		}
		else
		{
			m_dataBusy = true;
			m_busyLine = line;
		}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Store Unique (STOU) failed", (void*) line, (void*) 0 };
	DisposeData ();
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleAppendReply (const char*line)
{
	m_state = AppeFinished;
	switch (line [0])
	{
	case '1':
		m_status = success;
		m_state = AppePrepared;
		m_replyHandler = &FtpClient::HandleAppendReply;
		if (m_dataProtection)
			InitDataTlsHandshake ();
		else
			PrepareDataHandler ();
		return;
	case '3':
		m_status = error;
		break;
	case '2':
		if (m_dataSocket < 0)
		{
			void* args [] =
				{ (void*) line };
			InvokeFtpCallback (AppeFinishedEvent, ctrl, m_status = success, m_state, args);
		}
		else
		{
			m_dataBusy = true;
			m_busyLine = line;
		}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Append - with Create (APPE) failed", (void*) line, (void*) 0 };
	DisposeData ();
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleListReply (const char*line)
{
	m_state = ListFinished;
	switch (line [0])
	{
	case '1':
		m_status = success;
		m_state = ListPrepared;
		m_replyHandler = &FtpClient::HandleListReply;
		if (m_dataProtection)
			InitDataTlsHandshake ();
		else
			PrepareDataHandler ();
		return;
	case '3':
		m_status = error;
		break;
	case '2':
		if (m_dataSocket < 0)
		{
			void* args [] =
				{ (void*) line };
			InvokeFtpCallback (ListFinishedEvent, ctrl, m_status = success, m_state, args);
		}
		else
		{
			m_dataBusy = true;
			m_busyLine = line;
		}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "List (LIST) failed", (void*) line, (void*) 0 };
	DisposeData ();
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleNameListReply (const char*line)
{
	m_state = NlstFinished;
	switch (line [0])
	{
	case '1':
		m_status = success;
		m_state = NlstPrepared;
		m_replyHandler = &FtpClient::HandleNameListReply;
		if (m_dataProtection)
			InitDataTlsHandshake ();
		else
			PrepareDataHandler ();
		return;
	case '3':
		m_status = error;
		break;
	case '2':
		if (m_dataSocket < 0)
		{
			void* args [] =
				{ (void*) line };
			InvokeFtpCallback (NlstFinishedEvent, ctrl, m_status = success, NlstFinished, args);
		}
		else
		{
			m_dataBusy = true;
			m_busyLine = line;
		}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Name List (NLST) failed", (void*) line, (void*) 0 };
	DisposeData ();
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleRepresentationTypeReply (const char*line)
{
	m_state = TypeFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (TypeFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Representation Type (TYPE) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleFileStructureReply (const char*line)
{
	m_state = StruFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (StruFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "File Structure (STRU) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleTransferModeReply (const char*line)
{
	m_state = ModeFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (ModeFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Transfer Mode (MODE) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleAllocateReply (const char*line)
{
	m_state = AlloFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (AlloFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Allocate (ALLO) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleSystemReply (const char*line)
{
	m_state = SystFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (SystFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "System (SYST) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleStatusReply (const char*line)
{
	m_state = StatFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (StatFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Status (STAT) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleRenameFromReply (const char*line)
{
	m_state = RntoPrepared;
	switch (line [0])
	{
	case '1':
	case '2':
		m_status = error;
		break;
	case '3':
		m_status = success;
		PrepareRenameToRequest ();
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Rename From (RNFR) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleRenameToReply (const char*line)
{
	m_state = RntoFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (RntoFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Rename to (RNTO) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleAbortReply (const char*line)
{
	m_state = AborFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (AborFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Abort (ABOR) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleDeleteReply (const char*line)
{
	m_state = DeleFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (DeleFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Delete (DELE) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleRemoveDirectoryReply (const char*line)
{
	m_state = RmdFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (RmdFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Remove directory (RMD) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleMakeDirectoryReply (const char*line)
{
	m_state = MkdFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (MkdFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Make Directory (MKD) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandlePrintWorkingDirectoryReply (const char*line)
{
	m_state = PwdFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (PwdFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Print Working Directory (PWD) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleHelpReply (const char*line)
{
	m_state = HelpFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (HelpFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Help (HELP) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleNoopReply (const char*line)
{
	m_state = NoopFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (NoopFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "No Operation (NOOP) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleFeatureReply (const char*line)
{
	m_state = FeatFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (FeatFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Feature (FEAT) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleOptionsReply (const char*line)
{
	m_state = OptsFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (OptsFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Options (OPTS) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleAuthReply (const char*line)
{
	m_state = AuthFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (AuthFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Authentication Mechanism (AUTH) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleSSLAuthReply (const char*line)
{
	m_state = AuthFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
		InitCtrlTlsHandshake ();
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Authentication Mechanism (AUTH) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleAdatReply (const char*line)
{
	m_state = AdatFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (AdatFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Authentication Data (ADAT) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleProtReply (const char*line)
{
	m_state = ProtFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
		m_dataProtection = true;
		{
			void* args [] =
				{ (void*) line };
			InvokeFtpCallback (ProtFinishedEvent, ctrl, m_status = success, m_state, args);
		}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Data Channel Protection Level (PROT) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandlePbszReply (const char*line)
{
	m_state = PbszFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (PbszFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Protection Buffer Size (PBSZ) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleCccReply (const char*line)
{
	m_state = CccFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (CccFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Clear Command Channel (CCC) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleMicReply (const char*line)
{
	m_state = MicFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (MicFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Integrity Protected Command (MIC) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleConfReply (const char*line)
{
	m_state = ConfFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (ConfFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Confidentiality Protected Command (CONF) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

void FtpClient::HandleEncReply (const char*line)
{
	m_state = EncFinished;
	switch (line [0])
	{
	case '1':
	case '3':
		m_status = error;
		break;
	case '2':
	{
		void* args [] =
			{ (void*) line };
		InvokeFtpCallback (EncFinishedEvent, ctrl, m_status = success, m_state, args);
	}
		return;
	case '4':
	case '5':
		m_status = failure;
		break;
	default:
		m_status = failure;
		break;
	}
	void* args [] =
		{ (void*) "Privacy Protecced Command (ENC) failed", (void*) line, (void*) 0 };
	InvokeFtpCallback (ErrorEvent, ctrl, m_status, m_state, args);
}

} /* namespace sftp */
