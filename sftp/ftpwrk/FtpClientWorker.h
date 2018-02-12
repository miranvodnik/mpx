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
#include <dirent.h>
#include <regex.h>		// regular expressions
#include <ftpcln/FtpClient.h>
#include <string>
using namespace std;

namespace sftp
{

typedef enum FtpErrorCode
{
	FtpNoError,
	FtpCtrlConnBusyTimerExpired,
	FtpCtrlConnIdleTimerExpired,
	FtpDataConnTimerExpired,
	FtpListenConnTimerExpired,
	FtpUnresolvedHostname,
	FtpIllegalCwdContext,
	FtpCheckConnectivityFailed,
	FtpMkdirFailed,
	FtpGetdirFailed,
	FtpStoreNotListed,
	FtpStoreWorkDirEmpty,
	FtpStoreIllList,
	FtpStoreLocalUnlink,
	FtpStoreFailed,
	FtpRetrLocalAccess,
	FtpRetrWorkDirEmpty,
	FtpRetrMkWorkDir,
	FtpRetrIllList,
	FtpRetrRename,
	FtpRetrFailed,
	FtpStoreAllLocDirOpen,
	FtpStoreAllLocDirRead,
	FtpStoreAllStat,
	FtpStoreAllListReply,
	FtpStoreAllWorkDirEmpty,
	FtpStoreAllSrcRemove,
	FtpStoreAllFailed,
	FtpRetrAllCwdContext,
	FtpRetrAllEmptyList,
	FtpRetrAllMallocFailed,
	FtpRetrAllLocalAccess,
	FtpRetrAllWrkDirEmpty,
	FtpRetrAllWrkDirMake,
	FtpRetrAllIllListReply,
	FtpRetrAllLocalRename,
	FtpRetrAllFailed,
	FtpDeleteContext,
	FtpDeleteFailed,
	FtpDeleteAllContext,
	FtpDeleteAllNlist,
	FtpDeleteAllMallocFailed,
	FtpDeleteAllFailed,
	FtpMakeWorkingEnvFailed,
	FtpCleanDirFailed,
	FtpCleanDirMallocFailed,
} FtpErrorCode;

typedef enum SystemType
{
	UnknownSystem, UnixSystem, LinuxSystem, WindowsSystem
} SystemType;

class DirEntryInfo
{
public:
	char* name;
	u_int type;
	u_long size;
};

class FtpClientWorker: public FtpClient
{
private:
	// scenario initializer must be private - created only by c++ runtime
	FtpClientWorker (bool initialize);
public:
	FtpClientWorker ();
	virtual ~FtpClientWorker ();

private:

	mpx_event_handler(HandleStartEvent, FtpClientWorker)
	mpx_event_handler(HandleStopEvent, FtpClientWorker)
	mpx_event_handler(HandleInviteRequestEvent, FtpClientWorker)
	mpx_event_handler(HandleJobFinishedEvent, FtpClientWorker)

	//	Callbacks common to all scenarios
	ftp_callback (Common_NotImplementedEventHandler, FtpClientWorker)

	ftp_callback (Common_CtrlBusyTimerExpiredEventHandler, FtpClientWorker)
	ftp_callback (Common_CtrlIdleTimerExpiredEventHandler, FtpClientWorker)
	ftp_callback (Common_DataTimerExpiredEventHandler, FtpClientWorker)
	ftp_callback (Common_ListenTimerExpiredEventHandler, FtpClientWorker)
	ftp_callback (Common_RequestEventHandler, FtpClientWorker)
	ftp_callback (Common_ReplyEventHandler, FtpClientWorker)

	ftp_callback (Common_StartupEventHandler, FtpClientWorker)
	ftp_callback (Common_ConnectedEventHandler, FtpClientWorker)
	ftp_callback (Common_ClientAuthenticatedEventHandler, FtpClientWorker)
	ftp_callback (Common_SystFinishedEventHandler, FtpClientWorker)
	ftp_callback (Common_FeatFinishedEventHandler, FtpClientWorker)
	ftp_callback (Common_AuthFinishedEventHandler, FtpClientWorker)
	ftp_callback (Common_PbszFinishedEventHandler, FtpClientWorker)
	ftp_callback (Common_ProtFinishedEventHandler, FtpClientWorker)
	ftp_callback (Common_QuitFinishedEventHandler, FtpClientWorker)
	ftp_callback (Common_DisposedEventHandler, FtpClientWorker)

	// Check connection scenario callbacks
	ftp_callback (CheckConnectivity_ErrorEventHandler, FtpClientWorker)
	ftp_callback (CheckConnectivity_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (CheckConnectivity_CwdFinishedEventHandler, FtpClientWorker)

	// Make directory path scenario callbacks
	ftp_callback (Mkdir_ErrorEventHandler, FtpClientWorker)
	ftp_callback (Mkdir_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Mkdir_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Mkdir_MkdFinishedEventHandler, FtpClientWorker)

	// Get directory path scenario callbacks
	ftp_callback (Getdir_ErrorEventHandler, FtpClientWorker)
	ftp_callback (Getdir_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Getdir_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Getdir_ListPreparedEventHandler, FtpClientWorker)
	ftp_callback (Getdir_ListProgressEventHandler, FtpClientWorker)
	ftp_callback (Getdir_ListFinishedEventHandler, FtpClientWorker)
	ftp_callback (Getdir_NlstPreparedEventHandler, FtpClientWorker)
	ftp_callback (Getdir_NlstProgressEventHandler, FtpClientWorker)
	ftp_callback (Getdir_NlstFinishedEventHandler, FtpClientWorker)

	// Store file scenario callbacks
	ftp_callback (Store_ErrorEventHandler, FtpClientWorker)
	ftp_callback (Store_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Store_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Store_NlstProgressEventHandler, FtpClientWorker)
	ftp_callback (Store_NlstFinishedEventHandler, FtpClientWorker)
	ftp_callback (Store_ListProgressEventHandler, FtpClientWorker)
	ftp_callback (Store_ListFinishedEventHandler, FtpClientWorker)
	ftp_callback (Store_StorPreparedEventHandler, FtpClientWorker)
	ftp_callback (Store_StorProgressEventHandler, FtpClientWorker)
	ftp_callback (Store_StorFinishedEventHandler, FtpClientWorker)
	ftp_callback (Store_DeleFinishedEventHandler, FtpClientWorker)
	ftp_callback (Store_TypeFinishedEventHandler, FtpClientWorker)
	ftp_callback (Store_RntoFinishedEventHandler, FtpClientWorker)

	// Store file scenario helpers
	void Store_InvokeRepresentationType (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);
	void Store_RenameWorkingFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state, void* args []);
	void Store_RemoveSourceFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state, void* args []);

	// Retrieve file scenario callbacks
	ftp_callback (Retrieve_ErrorEventHandler, FtpClientWorker)
	ftp_callback (Retrieve_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Retrieve_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Retrieve_TypeFinishedEventHandler, FtpClientWorker)
	ftp_callback (Retrieve_RetrPreparedEventHandler, FtpClientWorker)
	ftp_callback (Retrieve_RetrProgressEventHandler, FtpClientWorker)
	ftp_callback (Retrieve_RetrFinishedEventHandler, FtpClientWorker)
	ftp_callback (Retrieve_ListProgressEventHandler, FtpClientWorker)
	ftp_callback (Retrieve_ListFinishedEventHandler, FtpClientWorker)
	ftp_callback (Retrieve_DeleFinishedEventHandler, FtpClientWorker)

	// Retrieve file scenario helpers
	void Retrieve_InvokeRepresentationType (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);
	void Retrieve_MoveRetrievedFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);

	// StoreAll files scenario callbacks
	ftp_callback (StoreAll_ErrorEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_NlstProgressEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_NlstFinishedEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_ListProgressEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_ListFinishedEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_StorPreparedEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_StorProgressEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_StorFinishedEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_DeleFinishedEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_TypeFinishedEventHandler, FtpClientWorker)
	ftp_callback (StoreAll_RntoFinishedEventHandler, FtpClientWorker)

	// StoreAll files scenario helpers
	void StoreAll_StoreNextFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state, void* args []);
	void StoreAll_InvokeRepresentationType (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);
	void StoreAll_RenameWorkingFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);
	void StoreAll_RemoveSourceFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);

	// RetrieveAll files scenario callbacks
	ftp_callback (RetrieveAll_ErrorEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_NlstProgressEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_NlstFinishedEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_TypeFinishedEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_RetrPreparedEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_RetrProgressEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_RetrFinishedEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_ListProgressEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_ListFinishedEventHandler, FtpClientWorker)
	ftp_callback (RetrieveAll_DeleFinishedEventHandler, FtpClientWorker)

	// RetrieveAll files scenario helpers
	void RetrieveAll_InvokeRepresentationType (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);
	void RetrieveAll_MoveRetrievedFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);
	void RetrieveAll_RetrieveNextFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);

	// Delete file scenario callbacks
	ftp_callback (Delete_ErrorEventHandler, FtpClientWorker)
	ftp_callback (Delete_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Delete_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (Delete_DeleFinishedEventHandler, FtpClientWorker)

	// DeleteAll files scenario callbacks
	ftp_callback (DeleteAll_ErrorEventHandler, FtpClientWorker)
	ftp_callback (DeleteAll_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (DeleteAll_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (DeleteAll_NlstProgressEventHandler, FtpClientWorker)
	ftp_callback (DeleteAll_NlstFinishedEventHandler, FtpClientWorker)
	ftp_callback (DeleteAll_DeleFinishedEventHandler, FtpClientWorker)

	// DeleteAll files scenario helpers
	void DeleteAll_DeleteNextFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state, void* args []);

	// Make working environment scenario callbacks
	ftp_callback (MakeWorkingEnv_ErrorEventHandler, FtpClientWorker)
	ftp_callback (MakeWorkingEnv_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (MakeWorkingEnv_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (MakeWorkingEnv_MkdFinishedEventHandler, FtpClientWorker)

	// Make working environment scenario helpers
	void MakeWorkingEnv_CwdFirstRemoteComponent (FtpClient *ftp);
	void MakeWorkingEnv_CwdNextRemoteComponent (FtpClient *ftp);
	void MakeWorkingEnv_CwdLastRemoteComponent (FtpClient *ftp);
	void MakeWorkingEnv_CwdFirstWorkingComponent (FtpClient *ftp);
	void MakeWorkingEnv_CwdNextWorkingComponent (FtpClient *ftp);
	void MakeWorkingEnv_Finished (FtpClient *ftp);

	// Clean directory scenario callbacks
	ftp_callback (CleanDir_ErrorEventHandler, FtpClientWorker)
	ftp_callback (CleanDir_PwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (CleanDir_CwdFinishedEventHandler, FtpClientWorker)
	ftp_callback (CleanDir_RmdFinishedEventHandler, FtpClientWorker)
	ftp_callback (CleanDir_ListPreparedEventHandler, FtpClientWorker)
	ftp_callback (CleanDir_ListProgressEventHandler, FtpClientWorker)
	ftp_callback (CleanDir_ListFinishedEventHandler, FtpClientWorker)
	ftp_callback (CleanDir_CdupFinishedEventHandler, FtpClientWorker)
	ftp_callback (CleanDir_DeleFinishedEventHandler, FtpClientWorker)

	// Clean directory scenario helpers
	void CleanDir_DeleteNextComponent (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
		void* args []);
	void CleanDir_IncStack ();
	void CleanDir_DecStack ();

	bool ReadListLine (char* line, DirEntryInfo &ent);
	bool ReadUnixListLine (char* parts [], int count, DirEntryInfo &ent);
	bool ReadLinuxListLine (char* parts [], int count, DirEntryInfo &ent);
	bool ReadWindowsListLine (char* parts [], int count, DirEntryInfo &ent);

	void GenErrorReport (void*args []);
	void printtrailer (bool force);
	void Dispose ();
	void Release (int result, const char* const msg [], const char* api, int errnum);

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
private:
	void Continue (FtpCallback* scenario);
	void ExtractHomeDir (char* line);
public:
	static void GenerateReport ();
private:
	u_long m_sessionId;

	MpxTaskBase* m_ctrlTask;
	FtpRequest* m_request;
	addrinfo* m_addrinfo;

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
	string m_name;
	string m_system;
	string m_homeDir;

	SystemType m_sysType;

	bool m_ctrlProtection;
	bool m_dataProtection;

	struct timespec m_start;
	struct timespec m_stop;
	long m_size;
	bool m_newLine;

	int m_listContext;
	int m_cwdContext;
	string m_listReply;

	DIR* m_localDir;
	struct dirent m_localDirEntry;
	struct dirent* m_localDirEntryPtr;

	char* m_currentRemoteFile;
	char* m_listBuffer;
	char* m_listPtr;
	char* m_listEnd;
	char* m_listStart;
	char* m_listRead;
	char* m_listName;
	int m_startIndex;

	char* m_tmpDirName;
	char* m_mkdPtr;
	char* m_mkdEnd;
	bool m_released;

	char* m_dirPtr;
	char* m_dirEnd;

	int* m_stack;
	int m_stackSize;
	int m_stackDepth;
private:
	static FtpClientWorker* g_scenarioInitializer;
	static FtpCallback* g_undefinedScenario;
	static FtpCallback* g_checkConnectivityScenario;
	static FtpCallback* g_mkdirScenario;
	static FtpCallback* g_getdirScenario;
	static FtpCallback* g_storeScenario;
	static FtpCallback* g_retrieveScenario;
	static FtpCallback* g_storeAllScenario;
	static FtpCallback* g_retrieveAllScenario;
	static FtpCallback* g_deleteScenario;
	static FtpCallback* g_deleteAllScenario;
	static FtpCallback* g_makeWorkingEnvScenario;
	static FtpCallback* g_cleanDirScenario;

	static u_long m_connContinued;
	static u_long m_connInited;
	static u_long m_connEstablished;
	static u_long m_userAuthenticated;
	static u_long m_connDisposed;
	static u_long m_connUnresolved;

	static EventDescriptor g_evntab[];
};

} /* namespace sftp */
