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

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
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
#include <string.h>
#include <mpx-core/MpxRunningContext.h>
using namespace mpx;
#include "FtpClientInterface.h"

#include <string>
#include <map>
using namespace std;

#define	ftp_callback(x,y) \
inline	static	void	x (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state, void* args[], void* appData) \
{ \
	((y*)appData)->x (ftp, context, status, state, args); \
} \
void	x (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state, void* args[]);

namespace sftp
{

typedef unsigned char byte;

enum FtpType
{
	undefinedType, ascii, ebcdic, image, local
};

enum FtpFormat
{
	undefinedFormat, nonprint, telnet, asa
};

enum FtpStructure
{
	undefinedStructure, file, record, page
};

enum FtpMode
{
	undefinedMode, stream, block, compressed
};

enum FtpContext
{
	unknown, ctrl, data, request, reply, start, stop
};

enum FtpStatus
{
	success, failure, error
};

typedef enum FtpState
{
	//	initial state (value = 0)
	Null,

	/* 001 - 002 */Startup, Connected,
	/* 003 - 004 */UserPrepared, PassPrepared,
	/* 005 - 006 */AcctPrepared, ClientAuthenticated,
	/* 007 - 010 */CwdPrepared, CwdFinished, CdupPrepared, CdupFinished,
	/* 011 - 014 */SmntPrepared, SmntFinished, ReinPrepared, ReinFinished,
	/* 015 - 018 */QuitPrepared, QuitFinished, PortPrepared, PortFinished,
	/* 019 - 022 */PasvPrepared, PasvFinished, TypePrepared, TypeFinished,
	/* 023 - 026 */StruPrepared, StruFinished, ModePrepared, ModeFinished,
	/* 027 - 030 */RetrPrepared, RetrProgress, RetrFinished, StorPrepared,
	/* 031 - 034 */StorProgress, StorFinished, StouPrepared, StouProgress,
	/* 035 - 038 */StouFinished, AppePrepared, AppeProgress, AppeFinished,
	/* 039 - 042 */ListPrepared, ListProgress, ListFinished, NlstPrepared,
	/* 041 - 046 */NlstProgress, NlstFinished, AlloPrepared, AlloFinished,
	/* 047 - 050 */SystPrepared, SystFinished, StatPrepared, StatFinished,
	/* 051 - 054 */RnfrPrepared, RnfrFinished, RntoPrepared, RntoFinished,
	/* 055 - 058 */AborPrepared, AborFinished, DelePrepared,
	DeleFinished,
	/* 059 - 062 */RmdPrepared,  RmdFinished,  MkdPrepared,  MkdFinished,
	/* 063 - 066 */PwdPrepared,  PwdFinished,  HelpPrepared, HelpFinished,
	/* 067 - 070 */NoopPrepared, NoopFinished, FeatPrepared, FeatFinished,
	/* 071 - 074 */OptsPrepared, OptsFinished, AuthPrepared, AuthFinished,
	/* 075 - 078 */AdatPrepared, AdatFinished, ProtPrepared, ProtFinished,
	/* 079 - 082 */PbszPrepared, PbszFinished, CccPrepared,  CccFinished,
	/* 083 - 086 */MicPrepared,  MicFinished,  ConfPrepared, ConfFinished,
	/* 087 - 088 */EncPrepared,  EncFinished,
	/* 089 - 089 */Disposed,

	FtpStateCount
} FtpState;

typedef enum FtpEventIndex
{
	//	initial event (value = 0)
	FtpNullEvent,

	//	pure protocol events
	/* 001 - 002 */StartupEvent, ConnectedEvent,
	/* 003 - 004 */UserPreparedEvent, PassPreparedEvent,
	/* 005 - 006 */AcctPreparedEvent, ClientAuthenticatedEvent,
	/* 007 - 010 */CwdPreparedEvent,  CwdFinishedEvent,  CdupPreparedEvent, CdupFinishedEvent,
	/* 011 - 014 */SmntPreparedEvent, SmntFinishedEvent, ReinPreparedEvent, ReinFinishedEvent,
	/* 015 - 018 */QuitPreparedEvent, QuitFinishedEvent, PortPreparedEvent, PortFinishedEvent,
	/* 019 - 022 */PasvPreparedEvent, PasvFinishedEvent, TypePreparedEvent, TypeFinishedEvent,
	/* 023 - 026 */StruPreparedEvent, StruFinishedEvent, ModePreparedEvent, ModeFinishedEvent,
	/* 027 - 030 */RetrPreparedEvent, RetrProgressEvent, RetrFinishedEvent, StorPreparedEvent,
	/* 031 - 034 */StorProgressEvent, StorFinishedEvent, StouPreparedEvent, StouProgressEvent,
	/* 035 - 038 */StouFinishedEvent, AppePreparedEvent, AppeProgressEvent, AppeFinishedEvent,
	/* 039 - 042 */ListPreparedEvent, ListProgressEvent, ListFinishedEvent, NlstPreparedEvent,
	/* 043 - 046 */NlstProgressEvent, NlstFinishedEvent, AlloPreparedEvent, AlloFinishedEvent,
	/* 047 - 050 */SystPreparedEvent, SystFinishedEvent, StatPreparedEvent, StatFinishedEvent,
	/* 051 - 054 */RnfrPreparedEvent, RnfrFinishedEvent, RntoPreparedEvent, RntoFinishedEvent,
	/* 055 - 058 */AborPreparedEvent, AborFinishedEvent, DelePreparedEvent, DeleFinishedEvent,
	/* 059 - 062 */RmdPreparedEvent,  RmdFinishedEvent,  MkdPreparedEvent,  MkdFinishedEvent,
	/* 063 - 066 */PwdPreparedEvent,  PwdFinishedEvent,  HelpPreparedEvent, HelpFinishedEvent,
	/* 067 - 070 */NoopPreparedEvent, NoopFinishedEvent, FeatPreparedEvent, FeatFinishedEvent,
	/* 071 - 074 */OptsPreparedEvent, OptsFinishedEvent, AuthPreparedEvent, AuthFinishedEvent,
	/* 075 - 078 */AdatPreparedEvent, AdatFinishedEvent, ProtPreparedEvent, ProtFinishedEvent,
	/* 079 - 082 */PbszPreparedEvent, PbszFinishedEvent, CccPreparedEvent,  CccFinishedEvent,
	/* 083 - 086 */MicPreparedEvent,  MicFinishedEvent,  ConfPreparedEvent, ConfFinishedEvent,
	/* 087 - 088 */EncPreparedEvent,  EncFinishedEvent,
	/* 089 - 089 */DisposedEvent,

	//	utility events
	/* 090 - 092 */ErrorEvent, RequestEvent, ReplyEvent,

	//	control events
	/* 093 - 096 */CtrlBusyTimerExpiredEvent, CtrlIdleTimerExpiredEvent, DataTimerExpiredEvent, ListenTimerExpiredEvent,

	FtpEventCount
} FtpEventIndex;

enum FtpDataCommand
{
	undefined, retrieve, store, storeUnique, append, allocate, list, nameList
};

typedef int (*entityHandler) (u_char* value, u_int size);

class FtpClient: public FtpClientInterface
{
private:
	FtpClient (bool initialize);
public:
	FtpClient ();
	virtual ~FtpClient ();

	typedef void (FtpClient::*ReplyHandler) (const char*line);
	typedef void (*FtpCallback) (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state, void* args [], void* appData);
	typedef map < FtpEventIndex, pair < FtpCallback, void* > * > ftpcbset;
	typedef pair < FtpCallback, void* > ftpcbdes;

private:

	void Dispose ();
	void DisposeData ();
	void DisposeCtrl ();
	void DisposePort ();

	fd_handler (CtrlSocketHandler, FtpClient);
	fd_handler (CtrlSslSocketConnectHandler, FtpClient);

	fd_handler (DataSocketHandler, FtpClient);
	fd_handler (RetrieveDataSocketHandler, FtpClient);
	fd_handler (StoreDataSocketHandler, FtpClient);
	fd_handler (StoreUniqueDataSocketHandler, FtpClient);
	fd_handler (AppendDataSocketHandler, FtpClient);
	fd_handler (ListDataSocketHandler, FtpClient);
	fd_handler (NameListDataSocketHandler, FtpClient);

	fd_handler (DataSslSocketConnectHandler, FtpClient);
	fd_handler (RetrieveSslDataSocketHandler, FtpClient);
	fd_handler (StoreSslDataSocketHandler, FtpClient);
	fd_handler (StoreUniqueSslDataSocketHandler, FtpClient);
	fd_handler (AppendSslDataSocketHandler, FtpClient);
	fd_handler (ListSslDataSocketHandler, FtpClient);
	fd_handler (NameListSslDataSocketHandler, FtpClient);

	fd_handler (ListenSocketHandler, FtpClient);

	timer_handler (CtrlIdleTimerHandler, FtpClient);
	timer_handler (DataTimerHandler, FtpClient);
	timer_handler (ListenTimerHandler, FtpClient);

	ftp_callback (FinalTimerExpiredEventHandler, FtpClient)

public:
	virtual bool IsAlive ()
	{
		return m_ctrlSocket > 0;
	}
	virtual void Start ();
	virtual void Stop ();
	virtual void Execute (FtpRequest* req, void* addInfo = 0) {}

	void RegisterFtpCallback (FtpEventIndex index, FtpCallback cb, void* appData);
	void UnregisterFtpCallback (FtpEventIndex index);
	void UnregisterAllFtpCallbacks ();
	void RegisterFtpScenario (FtpCallback* scenarioEvents, void*appData);
	void UnregisterFtpScenario ();

	void Connect (string hostname);
	void Connect (string hostname, sockaddr* addr, socklen_t addrlen);
	void Login (string user, string password);
	void ChangeWorkingDirectory (string pathName);
	void ChangeToParentDirectory ();
	void StructureMount (string pathName);
	void Reinitialize ();
	void Logout ();
	void RepresentationType (FtpType type, FtpFormat format, int byteSize);
	void FileStructure (FtpStructure str);
	void TransferMode (FtpMode mode);
	void Retrieve (string remotePathName, string localPathName);
	void Store (string localPathName, string remotePathName);
	void StoreUnique (string localPathName);
	void Append (string localPathName, string remotePathName);
	void List (string remotePathName);
	void NameList (string remotePathName);
	void Allocate (int fileSize, int recordSize);
	void System ();
	void Status (string pathName);
	void Rename (string oldPathName, string newPathName);
	void Abort ();
	void Delete (string pathName);
	void RemoveDirectory (string pathName);
	void MakeDirectory (string pathName);
	void PrintWorkingDirectory ();
	void Help (string help);
	void Noop ();
	void Feature ();
	void Options (string command);
	void AuthMechanism (string command);
	void AuthData (string command);
	void ProtectionLevel (string command);
	void ProtectionBufferSize (string command);
	void ClearCommandChannel (string command);
	void IntegrityProtectedCommand (string command);
	void ConfidentialityProtectedCommand (string command);
	void PrivacyProtectedCommand (string command);

private:
	void InvokeFtpCallback (FtpEventIndex index, FtpContext context, FtpStatus status, FtpState state, void* args []);
	void PostCtrlRequest ();
	void HandleRawReply (char* buffer, void* appData);
	static int verifyCallback (int depth, X509_STORE_CTX* storeCtx);

	void PreparePasswordRequest ();
	void PrepareAccountRequest ();
	void InitCtrlTlsHandshake ();
	void InitDataTlsHandshake ();

	void DataPort (ulong hostAddress, ulong port);
	void Passive ();
	void PreparePasvDataCommand ();
	void PrepareDataCommand ();
	void PrepareRetrRequest (string remotePathName, string localPathName);
	void PrepareStorRequest (string localPathName, string remotePathName);
	void PrepareStouRequest (string localPathName);
	void PrepareAppeRequest (string localPathName, string remotePathName);
	void PrepareListRequest (string remotePathName);
	void PrepareNlstRequest (string remotePathName);
	void PrepareDataHandler ();
	void PrepareSslDataHandler ();

	void InitiatePassiveDataTransfer ();
	void InitiateActiveDataTransfer ();

	void PrepareRenameToRequest ();

	void HandleConnectReply (const char*line);
	void HandleUserReply (const char*line);
	void HandlePasswordReply (const char*line);
	void HandleAccountReply (const char*line);
	void HandleChangeWorkingDirectoryReply (const char*line);
	void HandleChangeToParentDirectoryReply (const char*line);
	void HandleStructureMountReply (const char*line);
	void HandleReinitializeReply (const char*line);
	void HandleLogoutReply (const char*line);
	void HandleDataPortReply (const char*line);
	void HandlePassiveReply (const char*line);
	void HandleRetrieveReply (const char*line);
	void HandleStoreReply (const char*line);
	void HandleStoreUniqueReply (const char*line);
	void HandleAppendReply (const char*line);
	void HandleListReply (const char*line);
	void HandleNameListReply (const char*line);
	void HandleRepresentationTypeReply (const char*line);
	void HandleFileStructureReply (const char*line);
	void HandleTransferModeReply (const char*line);
	void HandleAllocateReply (const char*line);
	void HandleSystemReply (const char*line);
	void HandleStatusReply (const char*line);
	void HandleRenameFromReply (const char*line);
	void HandleRenameToReply (const char*line);
	void HandleAbortReply (const char*line);
	void HandleDeleteReply (const char*line);
	void HandleRemoveDirectoryReply (const char*line);
	void HandleMakeDirectoryReply (const char*line);
	void HandlePrintWorkingDirectoryReply (const char*line);
	void HandleHelpReply (const char*line);
	void HandleNoopReply (const char*line);
	void HandleFeatureReply (const char*line);
	void HandleOptionsReply (const char*line);
	void HandleAuthReply (const char*line);
	void HandleSSLAuthReply (const char*line);
	void HandleAdatReply (const char*line);
	void HandleProtReply (const char*line);
	void HandlePbszReply (const char*line);
	void HandleCccReply (const char*line);
	void HandleMicReply (const char*line);
	void HandleConfReply (const char*line);
	void HandleEncReply (const char*line);

	void InitDisposedScenario ();
public:
	inline MpxRunningContext* ctx (void)
	{
		return m_ctx;
	}
	inline void ctx (MpxRunningContext* ctx)
	{
		m_ctx = ctx;
	}
	inline void passiveEnabled (bool passive)
	{
		m_passiveEnabled = passive;
	}
	inline bool passiveEnabled (void)
	{
		return m_passiveEnabled;
	}
private:
	static FtpClient* m_initializer;
	MpxRunningContext* m_ctx;
	static FtpClient* g_disposedScenarioInitializer;
	static FtpCallback* g_disposedScenario;

	bool m_ctrlProtection;
	bool m_dataProtection;

	ftpcbdes* m_ftpEvents [FtpEventCount];
	FtpCallback* m_scenarioEvents;
	void* m_appData;

	string m_hostname;
	string m_user;
	string m_password;

	int m_ctrlSocket;
	int m_dataSocket;
	int m_listenSocket;

	SSL_CTX* m_sslCtx;

	BIO* m_ctrlBioSocket;
	BIO* m_dataBioSocket;

	SSL* m_ctrlSslSocket;
	SSL* m_dataSslSocket;

	FtpStatus m_status;
	FtpState m_state;
	ReplyHandler m_replyHandler;

	string m_ctrlRequest;
	string m_ctrlReply;

	ctx_fddes_t m_ctrlDes;
	ctx_fddes_t m_dataDes;
	ctx_fddes_t m_listenDes;
	bool m_dataBusy;
	string m_busyLine;
	bool m_dataProgress;

	ctx_timer_t m_ctrlTmr;
	ctx_timer_t m_dataTmr;
	ctx_timer_t m_listenTmr;

	FtpDataCommand m_dataCommand;
	string m_dataArgs [2];
	bool m_passiveEnabled;

	FILE* m_ftpFile;
	u_char* m_ftpBuffer;
	u_char* m_ftpBufferPtr;
	u_char* m_ftpBufferEnd;
};

} /* namespace sftp */
