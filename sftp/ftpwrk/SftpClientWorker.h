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

#include <mpx-events/MpxEvents.h>
#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-working-threads/MpxJobGetAddrInfo.h>
using namespace mpx;
#include <events/SftpEvents.h>
using namespace sftp;

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <unistd.h>
#include <regex.h>		// regular expressions
#include <ftpcln/SftpClient.h>

namespace sftp
{

typedef enum SftpErrorCode
{
	SftpNoError,
	SftpTempnamFailed,
	SftpOpenTempnam,
	SftpMallocFailed,
	SftpCtrlBusyTimerExpired,
	SftpCtrlIdleTimerExpired,
	SftpUnresolvedName,
	SftpCheckConnectivityFailed,
	SftpMkdirCwdFailed,
	SftpMkdirFailed,
	SftpGetdirFailed,
	SftpStoreWorkingNull,
	SftpFileExist,
	SftpFileSizeIncorrect,
	SftpUnlinkFailed,
	SftpStoreFailed,
	SftpAccessFailed,
	SftpRetrieveWorkingNull,
	SftpRetrieveMkdirFailed,
	SftpRetrieveRenameFailed,
	SftpRetrieveFailed,
	SftpStoreAllOpendirFailed,
	SftpStoreAllReaddirFailed,
	SftpStoreAllStatFailed,
	SftpStoreAllIllFileSize,
	SftpStoreAllFileExist,
	SftpStoreAllWorkingNull,
	SftpStoreAllUnlinkFailed,
	SftpStoreAllFailed,
	SftpRetrieveAllCwdFailed,
	SftpRetrieveAllWriteListDirEntry,
	SftpRetrieveAllAccessFailed,
	SftpRetrieveAllWorkingNull,
	SftpRetrieveAllMkdirFailed,
	SftpRetrieveAllMapFailed,
	SftpRetrieveAllReadListDirEntry,
	SftpRetrieveAllIllFileSize,
	SftpRetrieveAllRenameFailed,
	SftpRetrieveAllFailed,
	SftpDeleteCwdFailed,
	SftpDeleteFailed,
	SftpDeleteAllCwdContext,
	SftpDeleteAllWriteDirEntry,
	SftpDeleteAllMapFailed,
	SftpDeleteAllReadDirEntry,
	SftpDeleteAllFailed,
	SftpMakeWorkingEnvFailed,
	SftpCleanDirWriteDirEntry,
	SftpCleanDirMapFailed,
	SftpCleanDirReadDirEntry,
	SftpCleanDirFailed,
	SftpDisposed
} SftpErrorCode;

struct _ListDirInfo
{
	FILE* m_listFile;
	char* m_listFileName;
	size_t m_listFileSize;
	char* m_listFileAddr;
	char* m_listFilePtr;
	char* m_currentRemoteFile;
};
typedef struct _ListDirInfo ListDirInfo, *ListDirInfoPtr;

class SftpClientWorker: public SftpClient
{
private:
	// scenario initializer must be private - created only by c++ runtime
	SftpClientWorker (bool initialize);
public:
	SftpClientWorker ();
	virtual ~SftpClientWorker ();

private:

	mpx_event_handler(HandleStartEvent, SftpClientWorker)
	mpx_event_handler(HandleStopEvent, SftpClientWorker)
	mpx_event_handler(HandleInviteRequestEvent, SftpClientWorker)
	mpx_event_handler(HandleJobFinishedEvent, SftpClientWorker)

	//	Callbacks common to all scenarios
	sftp_callback (Common_NotImplementedEventHandler, SftpClientWorker)

	sftp_callback (Common_DisposedEventHandler, SftpClientWorker)
	sftp_callback (Common_CtrlBusyTimerExpiredEventHandler, SftpClientWorker)
	sftp_callback (Common_CtrlIdleTimerExpiredEventHandler, SftpClientWorker)

	sftp_callback (Common_StartupEventHandler, SftpClientWorker)
	sftp_callback (Common_ConnectedEventHandler, SftpClientWorker)
	sftp_callback (Common_HandshakeEventHandler, SftpClientWorker)
	sftp_callback (Common_Sha1HashEventHandler, SftpClientWorker)
	sftp_callback (Common_Md5HashEventHandler, SftpClientWorker)
	sftp_callback (Common_ClientAuthenticatedEventHandler, SftpClientWorker)
	sftp_callback (Common_SftpFinishedEventHandler, SftpClientWorker)
	sftp_callback (Common_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Common_RequestEventHandler, SftpClientWorker)
	sftp_callback (Common_ReplyEventHandler, SftpClientWorker)

	// Check connection scenario callbacks
	sftp_callback (CheckConnectivity_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (CheckConnectivity_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (CheckConnectivity_ErrorEventHandler, SftpClientWorker)

	// Make directory path scenario callbacks
	sftp_callback (Mkdir_ErrorEventHandler, SftpClientWorker)
	sftp_callback (Mkdir_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Mkdir_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Mkdir_MkdirFinishedEventHandler, SftpClientWorker)

	// Get directory path scenario callbacks
	sftp_callback (Getdir_ErrorEventHandler, SftpClientWorker)
	sftp_callback (Getdir_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Getdir_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Getdir_ListPreparedEventHandler, SftpClientWorker)
	sftp_callback (Getdir_ListProgressEventHandler, SftpClientWorker)
	sftp_callback (Getdir_ListFinishedEventHandler, SftpClientWorker)

	// Store file scenario callbacks
	sftp_callback (Store_ErrorEventHandler, SftpClientWorker)
	sftp_callback (Store_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Store_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Store_LCwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Store_ListPreparedEventHandler, SftpClientWorker)
	sftp_callback (Store_ListFinishedEventHandler, SftpClientWorker)
	sftp_callback (Store_PutFilePreparedEventHandler, SftpClientWorker)
	sftp_callback (Store_PutFileProgressEventHandler, SftpClientWorker)
	sftp_callback (Store_PutFileFinishedEventHandler, SftpClientWorker)
	sftp_callback (Store_RemoveFinishedEventHandler, SftpClientWorker)
	sftp_callback (Store_RenameFinishedEventHandler, SftpClientWorker)

	// Store file scenario helpers
	void Store_ChangeLocalDir (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state, void* args []);
	void Store_RenameWorkingFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);
	void Store_RemoveSourceFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);

	// Retrieve file scenario callbacks
	sftp_callback (Retrieve_ErrorEventHandler, SftpClientWorker)
	sftp_callback (Retrieve_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Retrieve_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Retrieve_GetFilePreparedEventHandler, SftpClientWorker)
	sftp_callback (Retrieve_GetFileProgressEventHandler, SftpClientWorker)
	sftp_callback (Retrieve_GetFileFinishedEventHandler, SftpClientWorker)
	sftp_callback (Retrieve_ListFinishedEventHandler, SftpClientWorker)
	sftp_callback (Retrieve_RemoveFinishedEventHandler, SftpClientWorker)

	// Retrieve file scenario helpers
	void Retrieve_GetFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state, void* args []);
	void Retrieve_MoveRetrievedFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);

	// StoreAll files scenario callbacks
	sftp_callback (StoreAll_ErrorEventHandler, SftpClientWorker)
	sftp_callback (StoreAll_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (StoreAll_LcwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (StoreAll_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (StoreAll_ListFinishedEventHandler, SftpClientWorker)
	sftp_callback (StoreAll_PutFilePreparedEventHandler, SftpClientWorker)
	sftp_callback (StoreAll_PutFileProgressEventHandler, SftpClientWorker)
	sftp_callback (StoreAll_PutFileFinishedEventHandler, SftpClientWorker)
	sftp_callback (StoreAll_RemoveFinishedEventHandler, SftpClientWorker)
	sftp_callback (StoreAll_RenameFinishedEventHandler, SftpClientWorker)

	// StoreAll files scenario helpers
	void StoreAll_StoreNextFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);
	void StoreAll_PutFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state, void* args []);
	void StoreAll_RenameWorkingFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);
	void StoreAll_RemoveSourceFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);

	// RetrieveAll files scenario callbacks
	sftp_callback (RetrieveAll_ErrorEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_ListDirPreparedEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_ListDirProgressEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_ListDirFinishedEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_GetFilePreparedEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_GetFileProgressEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_GetFileFinishedEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_ListFinishedEventHandler, SftpClientWorker)
	sftp_callback (RetrieveAll_RemoveFinishedEventHandler, SftpClientWorker)

	// RetrieveAll files scenario helpers
	void RetrieveAll_MoveRetrievedFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);
	void RetrieveAll_RetrieveNextFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);

	// Delete file scenario callbacks
	sftp_callback (Delete_ErrorEventHandler, SftpClientWorker)
	sftp_callback (Delete_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Delete_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (Delete_RemoveFinishedEventHandler, SftpClientWorker)

	// DeleteAll files scenario callbacks
	sftp_callback (DeleteAll_ErrorEventHandler, SftpClientWorker)
	sftp_callback (DeleteAll_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (DeleteAll_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (DeleteAll_ListDirPreparedEventHandler, SftpClientWorker)
	sftp_callback (DeleteAll_ListDirProgressEventHandler, SftpClientWorker)
	sftp_callback (DeleteAll_ListDirFinishedEventHandler, SftpClientWorker)
	sftp_callback (DeleteAll_RemoveFinishedEventHandler, SftpClientWorker)

	// DeleteAll files scenario helpers
	void DeleteAll_DeleteNextFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);

	// Make working environment scenario callbacks
	sftp_callback (MakeWorkingEnv_ErrorEventHandler, SftpClientWorker)
	sftp_callback (MakeWorkingEnv_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (MakeWorkingEnv_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (MakeWorkingEnv_MkdirFinishedEventHandler, SftpClientWorker)

	// Make working environment scenario helpers
	void MakeWorkingEnv_CwdFirstRemoteComponent (SftpClient *ftp);
	void MakeWorkingEnv_CwdNextRemoteComponent (SftpClient *ftp);
	void MakeWorkingEnv_CwdLastRemoteComponent (SftpClient *ftp);
	void MakeWorkingEnv_CwdFirstWorkingComponent (SftpClient *ftp);
	void MakeWorkingEnv_CwdNextWorkingComponent (SftpClient *ftp);
	void MakeWorkingEnv_Finished (SftpClient *ftp);

	// Clean directory scenario callbacks
	sftp_callback (CleanDir_ErrorEventHandler, SftpClientWorker)
	sftp_callback (CleanDir_PwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (CleanDir_CwdFinishedEventHandler, SftpClientWorker)
	sftp_callback (CleanDir_RmdFinishedEventHandler, SftpClientWorker)
	sftp_callback (CleanDir_ListDirPreparedEventHandler, SftpClientWorker)
	sftp_callback (CleanDir_ListDirProgressEventHandler, SftpClientWorker)
	sftp_callback (CleanDir_ListDirFinishedEventHandler, SftpClientWorker)
	sftp_callback (CleanDir_CdupFinishedEventHandler, SftpClientWorker)
	sftp_callback (CleanDir_RemoveFinishedEventHandler, SftpClientWorker)

	// Clean directory scenario helpers
	void CleanDir_DeleteNextComponent (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
		void* args []);

	void GenErrorReport (void*args []);
	void printtrailer (bool force);
	void Dispose ();
	void Release (int result, const char* const msg [], const char* api, int errnum);
	ssize_t OpenListDirFile ();
	ssize_t WriteListDirEntry (LIBSSH2_SFTP_ATTRIBUTES*& attrs, char*& buffer, char*& description);
	ssize_t ReadListDirEntry (LIBSSH2_SFTP_ATTRIBUTES& attrs, char*& buffer, char*& description);
	ssize_t CloseListDirFile ();
	void IncListDirStack ();
	void DecListDirStack ();

	static void InitUndefinedScenario ();
	static void InitCheckConnectivityScenario ();
	static void InitMkdirScenario ();
	static void InitGetdirScenario ();
	static void InitStoreScenario ();
	static void InitRetrieveScenario ();
	static void InitStoreAllScenario ();
	static void InitRetrieveAllScenario ();
	static void InitDeleteScenario ();
	static void InitDeleteAllScenario ();
	static void InitMakeWorkingEnvScenario ();
	static void InitCleanDirScenario ();

public:
	virtual void Execute (FtpRequest* req, void* addInfo = 0);
	static u_long connContinued ()
	{
		return m_connContinued;
	}
	static u_long connInited ()
	{
		return m_connInited;
	}
	static u_long connEstablished ()
	{
		return m_connEstablished;
	}
	static u_long userAuthenticated ()
	{
		return m_userAuthenticated;
	}
	static u_long connDisposed ()
	{
		return m_connDisposed;
	}
	static u_long connUnresolved ()
	{
		return m_connUnresolved;
	}
private:
	void Continue (SftpCallback* scenario);
public:
	static void GenerateReport ();
private:
	u_long m_sessionId;

	MpxTaskBase* m_ctrlTask;
	FtpRequest* m_request;
	addrinfo* m_addrinfo;

	char* m_tmpDirName;
	char* m_mkdPtr;
	char* m_mkdEnd;
	bool m_released;

	char* m_dirPtr;
	char* m_dirEnd;

	void* m_args;
	FtpRequest* m_ftpRequest;
	FtpConnectionInfo* m_ftpConnectionInfo;
	FtpRequestCode m_requestCode;
	union
	{
		EmptyFtpRequest* m_emptyRequest;
		CheckConnectivityRequest* m_checkConnectivity;
		MakeDirRequest* m_makeDir;
		GetDirRequest* m_getDir;
		StoreFileRequest* m_storeFile;
		RetrieveFileRequest* m_retrieveFile;
		StoreAllFilesRequest* m_storeAllFiles;
		RetrieveAllFilesRequest* m_retrieveAllFiles;
		DeleteFileRequest* m_deleteFile;
		DeleteAllFilesRequest* m_deleteAllFiles;
		MakeWorkingEnvRequest* m_makeWorkingEnv;
		CleanDirRequest* m_cleanDir;
	};

	string m_hostname;
	string m_homeDir;
	string m_remoteDir;
	string m_remoteWorkingDir;
	string m_localDir;
	string m_localWorkingDir;

	struct timespec m_start;
	struct timespec m_stop;

	int m_cwdContext;
	int m_listContext;
	LIBSSH2_SFTP_ATTRIBUTES m_attrs;
	u_long m_size;

	DIR* m_localDirHandle;
	struct dirent m_localDirEntry;
	struct dirent* m_localDirEntryPtr;

	ListDirInfo m_listDirInfo;
	ListDirInfoPtr m_listDirInfoStack;
	ssize_t m_listDirInfoStackSize;
	ssize_t m_listDirInfoStackDepth;

private:
	static SftpClientWorker* g_scenarioInitializer;
	static SftpCallback* g_undefinedScenario;
	static SftpCallback* g_checkConnectivityScenario;
	static SftpCallback* g_mkdirScenario;
	static SftpCallback* g_getdirScenario;
	static SftpCallback* g_storeScenario;
	static SftpCallback* g_retrieveScenario;
	static SftpCallback* g_storeAllScenario;
	static SftpCallback* g_retrieveAllScenario;
	static SftpCallback* g_deleteScenario;
	static SftpCallback* g_deleteAllScenario;
	static SftpCallback* g_makeWorkingEnvScenario;
	static SftpCallback* g_cleanDirScenario;

	static u_long m_connContinued;
	static u_long m_connInited;
	static u_long m_connEstablished;
	static u_long m_userAuthenticated;
	static u_long m_connDisposed;
	static u_long m_connUnresolved;

	static EventDescriptor g_evntab [];
};

} /* namespace sftp */
