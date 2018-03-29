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

#pragma once

#include "libssh2_config.h"
#include <libssh2.h>
#include <libssh2_sftp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stropts.h>
#include <errno.h>
#include <mpx-core/MpxRunningContext.h>
using namespace mpx;
#include "FtpClientInterface.h"
#include "SftpClientEnums.h"

#include <string>
#include <map>
using namespace std;

#define	sftp_callback(x,y) \
inline	static	void	x (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state, void* args[], void* appData) \
{ \
	((y*)appData)->x (ftp, context, status, state, args); \
} \
void	x (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state, void* args[]);

#define	IPPORT_SFTP	22	/* SSH File Transfer Protocol */

namespace sftp
{

class CBInvokeInfo
{
public:
	CBInvokeInfo (SftpEventIndex index, SftpContext context, SftpStatus status, SftpState state, void* args [],
		void* _this) :
		m_index (index), m_context (context), m_status (status), m_state (state), m_args (args), m_this (_this)
	{
		++g_counter;
	}
	static inline int counter ()
	{
		return g_counter;
	}
	inline SftpEventIndex index ()
	{
		return m_index;
	}
	inline SftpContext context ()
	{
		return m_context;
	}
	inline SftpStatus status ()
	{
		return m_status;
	}
	inline SftpState state ()
	{
		return m_state;
	}
	inline void** args ()
	{
		return m_args;
	}
	inline void* _this ()
	{
		return m_this;
	}
private:
	static int g_counter;
	SftpEventIndex m_index;
	SftpContext m_context;
	SftpStatus m_status;
	SftpState m_state;
	void** m_args;
	void* m_this;
};

class SftpClient: public FtpClientInterface
{
private:
	SftpClient (bool initialize);
public:
	SftpClient (evnset& e);
	virtual ~SftpClient ();
	virtual void StartTask ()
	{
	}
	virtual void StopTask ()
	{
	}

	typedef void (SftpClient::*ReplyHandler) (void*args []);
	typedef void (SftpClient::*RequestHandler) (void*args []);
	typedef void (*SftpCallback) (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args [], void* appData);
	typedef map <SftpEventIndex, pair <SftpCallback, void*> *> ftpcbset;
	typedef pair <SftpCallback, void*> ftpcbdes;
private:
	void Dispose (const char* msg);

	fd_handler (ConnSocketHandler, SftpClient)
	timer_handler (ConnIdleTimerHandler, SftpClient)
	timer_handler (FtpCallbackInvoker, SftpClient)

	void RealFtpCallbackInvoker (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t, CBInvokeInfo* info);

	sftp_callback (FinalTimerExpiredEventHandler, SftpClient)

public:
	virtual bool IsAlive ()
	{
		return m_connSocket > 0;
	}
	virtual void Start ();
	virtual void Stop ();
	virtual void Execute (FtpRequest* req, void* addInfo = 0)
	{
	}

	void RegisterFtpCallback (SftpEventIndex index, SftpCallback cb, void* appData);
	void UnregisterFtpCallback (SftpEventIndex index);
	void UnregisterAllFtpCallbacks ();
	void RegisterFtpScenario (SftpCallback* scenarioEvents, void*appData);
	void UnregisterFtpScenario ();

	inline MpxRunningContext* ctx (void)
	{
		return m_ctx;
	}
	inline void ctx (MpxRunningContext* ctx)
	{
		m_ctx = ctx;
	}
	inline string homeDir ()
	{
		return m_homeDir;
	}
	inline string currentLocalDir ()
	{
		return m_currentLocalDir;
	}
	inline string currentRemoteDir ()
	{
		return m_currentRemoteDir;
	}

	void Connect (string hostname);
	void Connect (string hostname, sockaddr* addr, socklen_t addrlen);
	void Handshake ();
	void CheckHash (int algorithm);
	void LoginPassword (string user, string password);
	void LoginPublickey (string user, string publickey);
	void CreateFtp ();
	void PrintWorkingDirectory ();
	void ChangeWorkingDirectory (string dir);
	void ChangeToParentDirectory ();
	void PrintLocalWorkingDirectory ();
	void ChangeLocalWorkingDirectory (string dir);
	void ChangeToLocalParentDirectory ();
	void MakeDirectory (string dir, int mode);
	void DeleteDirectory (string dir);
	void ListDirectory (string dir);
	void PutFile (string sourceFile, string targetFile, long mode);
	void GetFile (string sourceFile, string targetFile, long mode);
	void RenameFile (string sourceFile, string targetFile);
	void RemoveFile (string targetFile);
	void ListFile (string targetFile);

private:
	void InvokeFtpCallback (SftpEventIndex index, SftpContext context, SftpStatus status, SftpState state,
		void* args []);
	void InvokeFtpCallbackTimer (SftpEventIndex index, SftpContext context, SftpStatus status, SftpState state,
		void* args []);
	void _Handshake ();
	void LoginPassword ();
	void LoginPublickey ();
	void _CreateFtp ();
	void _GetHomeDirectory ();
	void _PrintWorkingDirectory ();
	void ChangeWorkingDirectory ();
	void _ChangeToParentDirectory ();
	void ListDirectory ();
	void MakeDirectory ();
	void DeleteDirectory ();
	void CheckTargetPath ();
	void CloseDirHandle ();
	void ReadDirectoryEntry ();
	void GetLocalDirectory (string localDirName);
	string GetLocalFilePath (string localFile);
	string GetRemoteDirectoryPath (string remoteDir);
	string GetRemoteFilePath (string remoteFile);
	void PutFile ();
	void ReadFileChunk ();
	void PutFileChunk ();
	void PutFileCloseHandle ();
	void GetFile ();
	void GetFileChunk ();
	void WriteFileChunk ();
	void GetFileCloseHandle ();
	void RenameFile ();
	void RemoveFile ();
	void ListFile ();

	void HandleConnectReply (void* args []);
	void HandleHandshakeBusyReply (void* args []);
	void HandleAuthPasswordBusyReply (void* args []);
	void HandleAuthPublickeyBusyReply (void* args []);
	void HandleSftpBusyReply (void* args []);
	void HandleHomeBusyReply (void* args []);
	void HandlePwdBusyReply (void* args []);
	void HandleCwdBusyReply (void* args []);
	void HandleCdupBusyReply (void* args []);
	void HandleMkdirBusyReply (void* args []);
	void HandleRmdirBusyReply (void* args []);
	void HandleListDirBusyReply (void* args []);
	void HandleReadDirEntryReply (void* args []);
	void HandleListDirCloseReply (void* args []);
	void HandlePutFileBusyReply (void* args []);
	void HandlePutFileChunkReply (void* args []);
	void HandleReadFileChunkReply (void* args []);
	void HandlePutFileCloseReply (void* args []);
	void HandleGetFileBusyReply (void* args []);
	void HandleGetFileChunkReply (void* args []);
	void HandleGetFileCloseReply (void* args []);
	void HandleRenameBusyReply (void* args []);
	void HandleRemoveBusyReply (void* args []);
	void HandleListBusyReply (void* args []);

	static void InitDisposedScenario ();

private:
	MpxRunningContext* m_ctx;
	static SftpClient* g_disposedScenarioInitializer;
	static SftpCallback* g_disposedScenario;

	ftpcbdes* m_ftpEvents [SftpEventCount];
	SftpCallback* m_scenarioEvents;
	void* m_appData;

	string m_hostname;
	string m_user;
	string m_password;
	string m_publickey;

	int m_connSocket;
	LIBSSH2_SESSION* m_session;
	LIBSSH2_SFTP* m_sftp;

	SftpStatus m_status;
	SftpState m_state;
	RequestHandler m_requestHandler;
	ReplyHandler m_replyHandler;

	ctx_fddes_t m_connDes;
	ctx_timer_t m_connTmr;
	int m_connTimerEnabled;
	int m_connTimerDisabled;

	char* m_targetPath;
	int m_targetPathLen;

	string m_homeDir;
	string m_currentLocalDir;
	string m_currentRemoteDir;

	string m_remoteDirName;
	int m_dirMode;
	LIBSSH2_SFTP_HANDLE* m_dirHandle;

	string m_sourceFile;
	string m_targetFile;
	long m_mode;
	FILE* m_localFileHandle;
	LIBSSH2_SFTP_HANDLE* m_remoteFileHandle;
	u_char* m_fileChunk;
	u_char* m_fileChunkPtr;
	u_char* m_fileChunkEnd;

	bool m_useTrash;
};

//	Object which makes all necessary but lengthy clean-up operations
//	on allocated SSH resources supposed to be done by SftpClient
//	but which are done by SftpTrash instead while SftpClient
//	continues with another file transfers.

class SftpTrash
{
	typedef enum
	{
		InitialStage,
		ReleaseRemoteFileHandleStage,
		ReleaseDirHandleStage,
		ReleaseSftpStage,
		ReleaseSessionStage,
		ReleaseConnSocketStage,
		FinalStage
	} Stage;

public:
	SftpTrash (MpxRunningContext* ctx, int connSocket, LIBSSH2_SESSION* session, LIBSSH2_SFTP* sftp,
		LIBSSH2_SFTP_HANDLE* dirHandle, LIBSSH2_SFTP_HANDLE* remoteFileHandle);
	virtual ~SftpTrash ();

	void CleanUp ();
	static inline bool debug ()
	{
		return g_debug;
	}
	static inline void debug (bool debug)
	{
		g_debug = debug;
	}

private:
	fd_handler (HandleClientConnection, SftpTrash)
	timer_handler (StageTimer, SftpTrash)
	timer_handler (ConnectionTimer, SftpTrash)

	void InvokeStageOperation ();

	int ReleaseRemoteFileHandle ();
	int ReleaseDirHandle ();
	int ReleaseSftp ();
	int ReleaseSession ();
	int ReleaseConnSocket ();
private:
	static bool g_debug;
	static long g_trashId;
	long m_trashId;
	MpxRunningContext* m_ctx;
	int m_connSocket;
	LIBSSH2_SESSION* m_session;
	LIBSSH2_SFTP* m_sftp;
	LIBSSH2_SFTP_HANDLE* m_dirHandle;
	LIBSSH2_SFTP_HANDLE* m_remoteFileHandle;

	ctx_timer_t m_stageTimer;
	ctx_timer_t m_connTimer;
	ctx_fddes_t m_connHandle;
	Stage m_stage;
};

} /* namespace sftp */

