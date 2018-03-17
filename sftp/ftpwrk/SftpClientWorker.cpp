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

#include "SftpClientWorker.h"
#include <ftplog/SftpLog.h>

namespace sftp
{

#define	listFile	m_listDirInfo.m_listFile
#define	listFileName	m_listDirInfo.m_listFileName
#define	listFileSize	m_listDirInfo.m_listFileSize
#define	listFileAddr	m_listDirInfo.m_listFileAddr
#define	listFilePtr	m_listDirInfo.m_listFilePtr
#define	currentRemoteFile	m_listDirInfo.m_currentRemoteFile

//	statistical data
u_long SftpClientWorker::m_connContinued = 0;
u_long SftpClientWorker::m_connInited = 0;
u_long SftpClientWorker::m_connEstablished = 0;
u_long SftpClientWorker::m_userAuthenticated = 0;
u_long SftpClientWorker::m_connDisposed = 0;
u_long SftpClientWorker::m_connUnresolved = 0;

//	all currently available scenarios
SftpClient::SftpCallback* SftpClientWorker::g_undefinedScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_checkConnectivityScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_mkdirScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_getdirScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_storeScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_retrieveScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_storeAllScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_retrieveAllScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_deleteScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_deleteAllScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_makeWorkingEnvScenario = 0;
SftpClient::SftpCallback* SftpClientWorker::g_cleanDirScenario = 0;

//	scenario initializer - must follow (not precede) scenario declarations
SftpClientWorker* SftpClientWorker::g_scenarioInitializer = new SftpClientWorker (true);

EventDescriptor SftpClientWorker::g_evntab [] =
{
	{ AnyState, SftpInviteRequest::EventCode, HandleInviteRequestEvent },
	{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
	{ 0, 0, 0 },
};

MpxTaskBase::evnset SftpClientWorker::g_evnset = MpxTaskBase::CreateEventSet (g_evntab);

SftpClientWorker::SftpClientWorker (bool initialize) : SftpClient (g_evnset)
{
	if (!initialize)
		return;
	InitUndefinedScenario ();
	InitCheckConnectivityScenario ();
	InitMkdirScenario ();
	InitGetdirScenario ();
	InitStoreScenario ();
	InitRetrieveScenario ();
	InitStoreAllScenario ();
	InitRetrieveAllScenario ();
	InitDeleteScenario ();
	InitDeleteAllScenario ();
	InitMakeWorkingEnvScenario ();
	InitCleanDirScenario ();
}

void SftpClientWorker::InitUndefinedScenario ()
{
	g_undefinedScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_undefinedScenario [SftpNullEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SStartupEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SConnectPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SConnectedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SHandshakePreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SHandshakeFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SHashPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [Sha1HashEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [Md5HashEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SAuthPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SAuthFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SSftpPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SSftpFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SPwdPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SPwdFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SCwdPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SCwdFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SCdupPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SCdupFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SLPwdPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SLPwdFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SLCwdPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SLCwdFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SLCdupPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SLCdupFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SMkdirPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SMkdirFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SRmdirPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SRmdirFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SListDirPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SListDirProgressEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SListDirFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SPutFilePreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SPutFileProgressEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SPutFileFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SGetFilePreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SGetFileProgressEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SGetFileFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SRenamePreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SRenameFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SRemovePreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SRemoveFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SListPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SListFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SDisposedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SErrorEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SRequestEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SReplyEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SCtrlBusyTimerExpiredEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SCtrlIdleTimerExpiredEvent] = Common_NotImplementedEventHandler;
}

void SftpClientWorker::InitCheckConnectivityScenario ()
{
	g_checkConnectivityScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_checkConnectivityScenario [SftpNullEvent] = 0;
	g_checkConnectivityScenario [SStartupEvent] = Common_StartupEventHandler;
	g_checkConnectivityScenario [SConnectPreparedEvent] = 0;
	g_checkConnectivityScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_checkConnectivityScenario [SHandshakePreparedEvent] = 0;
	g_checkConnectivityScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_checkConnectivityScenario [SHashPreparedEvent] = 0;
	g_checkConnectivityScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_checkConnectivityScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_checkConnectivityScenario [SAuthPreparedEvent] = 0;
	g_checkConnectivityScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_checkConnectivityScenario [SSftpPreparedEvent] = 0;
	g_checkConnectivityScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_checkConnectivityScenario [SPwdPreparedEvent] = 0;
	g_checkConnectivityScenario [SPwdFinishedEvent] = CheckConnectivity_PwdFinishedEventHandler;
	g_checkConnectivityScenario [SCwdPreparedEvent] = 0;
	g_checkConnectivityScenario [SCwdFinishedEvent] = CheckConnectivity_CwdFinishedEventHandler;
	g_checkConnectivityScenario [SCdupPreparedEvent] = 0;
	g_checkConnectivityScenario [SCdupFinishedEvent] = 0;
	g_checkConnectivityScenario [SLPwdPreparedEvent] = 0;
	g_checkConnectivityScenario [SLPwdFinishedEvent] = 0;
	g_checkConnectivityScenario [SLCwdPreparedEvent] = 0;
	g_checkConnectivityScenario [SLCwdFinishedEvent] = 0;
	g_checkConnectivityScenario [SLCdupPreparedEvent] = 0;
	g_checkConnectivityScenario [SLCdupFinishedEvent] = 0;
	g_checkConnectivityScenario [SMkdirPreparedEvent] = 0;
	g_checkConnectivityScenario [SMkdirFinishedEvent] = 0;
	g_checkConnectivityScenario [SRmdirPreparedEvent] = 0;
	g_checkConnectivityScenario [SRmdirFinishedEvent] = 0;
	g_checkConnectivityScenario [SListDirPreparedEvent] = 0;
	g_checkConnectivityScenario [SListDirProgressEvent] = 0;
	g_checkConnectivityScenario [SListDirFinishedEvent] = 0;
	g_checkConnectivityScenario [SPutFilePreparedEvent] = 0;
	g_checkConnectivityScenario [SPutFileProgressEvent] = 0;
	g_checkConnectivityScenario [SPutFileFinishedEvent] = 0;
	g_checkConnectivityScenario [SGetFilePreparedEvent] = 0;
	g_checkConnectivityScenario [SGetFileProgressEvent] = 0;
	g_checkConnectivityScenario [SGetFileFinishedEvent] = 0;
	g_checkConnectivityScenario [SRenamePreparedEvent] = 0;
	g_checkConnectivityScenario [SRenameFinishedEvent] = 0;
	g_checkConnectivityScenario [SRemovePreparedEvent] = 0;
	g_checkConnectivityScenario [SRemoveFinishedEvent] = 0;
	g_checkConnectivityScenario [SListPreparedEvent] = 0;
	g_checkConnectivityScenario [SListFinishedEvent] = 0;
	g_checkConnectivityScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_checkConnectivityScenario [SErrorEvent] = CheckConnectivity_ErrorEventHandler;
	g_checkConnectivityScenario [SRequestEvent] = Common_RequestEventHandler;
	g_checkConnectivityScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_checkConnectivityScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_checkConnectivityScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitMkdirScenario ()
{
	g_mkdirScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_mkdirScenario [SftpNullEvent] = 0;
	g_mkdirScenario [SStartupEvent] = Common_StartupEventHandler;
	g_mkdirScenario [SConnectPreparedEvent] = 0;
	g_mkdirScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_mkdirScenario [SHandshakePreparedEvent] = 0;
	g_mkdirScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_mkdirScenario [SHashPreparedEvent] = 0;
	g_mkdirScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_mkdirScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_mkdirScenario [SAuthPreparedEvent] = 0;
	g_mkdirScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_mkdirScenario [SSftpPreparedEvent] = 0;
	g_mkdirScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_mkdirScenario [SPwdPreparedEvent] = 0;
	g_mkdirScenario [SPwdFinishedEvent] = Mkdir_PwdFinishedEventHandler;
	g_mkdirScenario [SCwdPreparedEvent] = 0;
	g_mkdirScenario [SCwdFinishedEvent] = Mkdir_CwdFinishedEventHandler;
	g_mkdirScenario [SCdupPreparedEvent] = 0;
	g_mkdirScenario [SCdupFinishedEvent] = 0;
	g_mkdirScenario [SLPwdPreparedEvent] = 0;
	g_mkdirScenario [SLPwdFinishedEvent] = 0;
	g_mkdirScenario [SLCwdPreparedEvent] = 0;
	g_mkdirScenario [SLCwdFinishedEvent] = 0;
	g_mkdirScenario [SLCdupPreparedEvent] = 0;
	g_mkdirScenario [SLCdupFinishedEvent] = 0;
	g_mkdirScenario [SMkdirPreparedEvent] = 0;
	g_mkdirScenario [SMkdirFinishedEvent] = Mkdir_MkdirFinishedEventHandler;
	g_mkdirScenario [SRmdirPreparedEvent] = 0;
	g_mkdirScenario [SRmdirFinishedEvent] = 0;
	g_mkdirScenario [SListDirPreparedEvent] = 0;
	g_mkdirScenario [SListDirProgressEvent] = 0;
	g_mkdirScenario [SListDirFinishedEvent] = 0;
	g_mkdirScenario [SPutFilePreparedEvent] = 0;
	g_mkdirScenario [SPutFileProgressEvent] = 0;
	g_mkdirScenario [SPutFileFinishedEvent] = 0;
	g_mkdirScenario [SGetFilePreparedEvent] = 0;
	g_mkdirScenario [SGetFileProgressEvent] = 0;
	g_mkdirScenario [SGetFileFinishedEvent] = 0;
	g_mkdirScenario [SRenamePreparedEvent] = 0;
	g_mkdirScenario [SRenameFinishedEvent] = 0;
	g_mkdirScenario [SRemovePreparedEvent] = 0;
	g_mkdirScenario [SRemoveFinishedEvent] = 0;
	g_mkdirScenario [SListPreparedEvent] = 0;
	g_mkdirScenario [SListFinishedEvent] = 0;
	g_mkdirScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_mkdirScenario [SErrorEvent] = Mkdir_ErrorEventHandler;
	g_mkdirScenario [SRequestEvent] = Common_RequestEventHandler;
	g_mkdirScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_mkdirScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_mkdirScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitGetdirScenario ()
{
	g_getdirScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_getdirScenario [SftpNullEvent] = 0;
	g_getdirScenario [SStartupEvent] = Common_StartupEventHandler;
	g_getdirScenario [SConnectPreparedEvent] = 0;
	g_getdirScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_getdirScenario [SHandshakePreparedEvent] = 0;
	g_getdirScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_getdirScenario [SHashPreparedEvent] = 0;
	g_getdirScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_getdirScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_getdirScenario [SAuthPreparedEvent] = 0;
	g_getdirScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_getdirScenario [SSftpPreparedEvent] = 0;
	g_getdirScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_getdirScenario [SPwdPreparedEvent] = 0;
	g_getdirScenario [SPwdFinishedEvent] = Getdir_PwdFinishedEventHandler;
	g_getdirScenario [SCwdPreparedEvent] = 0;
	g_getdirScenario [SCwdFinishedEvent] = Getdir_CwdFinishedEventHandler;
	g_getdirScenario [SCdupPreparedEvent] = 0;
	g_getdirScenario [SCdupFinishedEvent] = 0;
	g_getdirScenario [SLPwdPreparedEvent] = 0;
	g_getdirScenario [SLPwdFinishedEvent] = 0;
	g_getdirScenario [SLCwdPreparedEvent] = 0;
	g_getdirScenario [SLCwdFinishedEvent] = 0;
	g_getdirScenario [SLCdupPreparedEvent] = 0;
	g_getdirScenario [SLCdupFinishedEvent] = 0;
	g_getdirScenario [SMkdirPreparedEvent] = 0;
	g_getdirScenario [SMkdirFinishedEvent] = 0;
	g_getdirScenario [SRmdirPreparedEvent] = 0;
	g_getdirScenario [SRmdirFinishedEvent] = 0;
	g_getdirScenario [SListDirPreparedEvent] = Getdir_ListPreparedEventHandler;
	g_getdirScenario [SListDirProgressEvent] = Getdir_ListProgressEventHandler;
	g_getdirScenario [SListDirFinishedEvent] = Getdir_ListFinishedEventHandler;
	g_getdirScenario [SPutFilePreparedEvent] = 0;
	g_getdirScenario [SPutFileProgressEvent] = 0;
	g_getdirScenario [SPutFileFinishedEvent] = 0;
	g_getdirScenario [SGetFilePreparedEvent] = 0;
	g_getdirScenario [SGetFileProgressEvent] = 0;
	g_getdirScenario [SGetFileFinishedEvent] = 0;
	g_getdirScenario [SRenamePreparedEvent] = 0;
	g_getdirScenario [SRenameFinishedEvent] = 0;
	g_getdirScenario [SRemovePreparedEvent] = 0;
	g_getdirScenario [SRemoveFinishedEvent] = 0;
	g_getdirScenario [SListPreparedEvent] = 0;
	g_getdirScenario [SListFinishedEvent] = 0;
	g_getdirScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_getdirScenario [SErrorEvent] = Getdir_ErrorEventHandler;
	g_getdirScenario [SRequestEvent] = Common_RequestEventHandler;
	g_getdirScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_getdirScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_getdirScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitStoreScenario ()
{
	g_storeScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_storeScenario [SftpNullEvent] = 0;
	g_storeScenario [SStartupEvent] = Common_StartupEventHandler;
	g_storeScenario [SConnectPreparedEvent] = 0;
	g_storeScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_storeScenario [SHandshakePreparedEvent] = 0;
	g_storeScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_storeScenario [SHashPreparedEvent] = 0;
	g_storeScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_storeScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_storeScenario [SAuthPreparedEvent] = 0;
	g_storeScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_storeScenario [SSftpPreparedEvent] = 0;
	g_storeScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_storeScenario [SPwdPreparedEvent] = 0;
	g_storeScenario [SPwdFinishedEvent] = Store_PwdFinishedEventHandler;
	g_storeScenario [SCwdPreparedEvent] = 0;
	g_storeScenario [SCwdFinishedEvent] = Store_CwdFinishedEventHandler;
	g_storeScenario [SCdupPreparedEvent] = 0;
	g_storeScenario [SCdupFinishedEvent] = 0;
	g_storeScenario [SLPwdPreparedEvent] = 0;
	g_storeScenario [SLPwdFinishedEvent] = 0;
	g_storeScenario [SLCwdPreparedEvent] = 0;
	g_storeScenario [SLCwdFinishedEvent] = Store_LCwdFinishedEventHandler;
	g_storeScenario [SLCdupPreparedEvent] = 0;
	g_storeScenario [SLCdupFinishedEvent] = 0;
	g_storeScenario [SMkdirPreparedEvent] = 0;
	g_storeScenario [SMkdirFinishedEvent] = 0;
	g_storeScenario [SRmdirPreparedEvent] = 0;
	g_storeScenario [SRmdirFinishedEvent] = 0;
	g_storeScenario [SListDirPreparedEvent] = 0;
	g_storeScenario [SListDirProgressEvent] = 0;
	g_storeScenario [SListDirFinishedEvent] = 0;
	g_storeScenario [SPutFilePreparedEvent] = Store_PutFilePreparedEventHandler;
	g_storeScenario [SPutFileProgressEvent] = Store_PutFileProgressEventHandler;
	g_storeScenario [SPutFileFinishedEvent] = Store_PutFileFinishedEventHandler;
	g_storeScenario [SGetFilePreparedEvent] = 0;
	g_storeScenario [SGetFileProgressEvent] = 0;
	g_storeScenario [SGetFileFinishedEvent] = 0;
	g_storeScenario [SRenamePreparedEvent] = 0;
	g_storeScenario [SRenameFinishedEvent] = Store_RenameFinishedEventHandler;
	g_storeScenario [SRemovePreparedEvent] = 0;
	g_storeScenario [SRemoveFinishedEvent] = Store_RemoveFinishedEventHandler;
	g_storeScenario [SListPreparedEvent] = Store_ListPreparedEventHandler;
	g_storeScenario [SListFinishedEvent] = Store_ListFinishedEventHandler;
	g_storeScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_storeScenario [SErrorEvent] = Store_ErrorEventHandler;
	g_storeScenario [SRequestEvent] = Common_RequestEventHandler;
	g_storeScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_storeScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_storeScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitRetrieveScenario ()
{
	g_retrieveScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_retrieveScenario [SftpNullEvent] = 0;
	g_retrieveScenario [SStartupEvent] = Common_StartupEventHandler;
	g_retrieveScenario [SConnectPreparedEvent] = 0;
	g_retrieveScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_retrieveScenario [SHandshakePreparedEvent] = 0;
	g_retrieveScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_retrieveScenario [SHashPreparedEvent] = 0;
	g_retrieveScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_retrieveScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_retrieveScenario [SAuthPreparedEvent] = 0;
	g_retrieveScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_retrieveScenario [SSftpPreparedEvent] = 0;
	g_retrieveScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_retrieveScenario [SPwdPreparedEvent] = 0;
	g_retrieveScenario [SPwdFinishedEvent] = Retrieve_PwdFinishedEventHandler;
	g_retrieveScenario [SCwdPreparedEvent] = 0;
	g_retrieveScenario [SCwdFinishedEvent] = Retrieve_CwdFinishedEventHandler;
	g_retrieveScenario [SCdupPreparedEvent] = 0;
	g_retrieveScenario [SCdupFinishedEvent] = 0;
	g_retrieveScenario [SLPwdPreparedEvent] = 0;
	g_retrieveScenario [SLPwdFinishedEvent] = 0;
	g_retrieveScenario [SLCwdPreparedEvent] = 0;
	g_retrieveScenario [SLCwdFinishedEvent] = 0;
	g_retrieveScenario [SLCdupPreparedEvent] = 0;
	g_retrieveScenario [SLCdupFinishedEvent] = 0;
	g_retrieveScenario [SMkdirPreparedEvent] = 0;
	g_retrieveScenario [SMkdirFinishedEvent] = 0;
	g_retrieveScenario [SRmdirPreparedEvent] = 0;
	g_retrieveScenario [SRmdirFinishedEvent] = 0;
	g_retrieveScenario [SListDirPreparedEvent] = 0;
	g_retrieveScenario [SListDirProgressEvent] = 0;
	g_retrieveScenario [SListDirFinishedEvent] = 0;
	g_retrieveScenario [SPutFilePreparedEvent] = 0;
	g_retrieveScenario [SPutFileProgressEvent] = 0;
	g_retrieveScenario [SPutFileFinishedEvent] = 0;
	g_retrieveScenario [SGetFilePreparedEvent] = Retrieve_GetFilePreparedEventHandler;
	g_retrieveScenario [SGetFileProgressEvent] = Retrieve_GetFileProgressEventHandler;
	g_retrieveScenario [SGetFileFinishedEvent] = Retrieve_GetFileFinishedEventHandler;
	g_retrieveScenario [SRenamePreparedEvent] = 0;
	g_retrieveScenario [SRenameFinishedEvent] = 0;
	g_retrieveScenario [SRemovePreparedEvent] = 0;
	g_retrieveScenario [SRemoveFinishedEvent] = Retrieve_RemoveFinishedEventHandler;
	g_retrieveScenario [SListPreparedEvent] = 0;
	g_retrieveScenario [SListFinishedEvent] = Retrieve_ListFinishedEventHandler;
	g_retrieveScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_retrieveScenario [SErrorEvent] = Retrieve_ErrorEventHandler;
	g_retrieveScenario [SRequestEvent] = Common_RequestEventHandler;
	g_retrieveScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_retrieveScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_retrieveScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitStoreAllScenario ()
{
	g_storeAllScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_storeAllScenario [SftpNullEvent] = 0;
	g_storeAllScenario [SStartupEvent] = Common_StartupEventHandler;
	g_storeAllScenario [SConnectPreparedEvent] = 0;
	g_storeAllScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_storeAllScenario [SHandshakePreparedEvent] = 0;
	g_storeAllScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_storeAllScenario [SHashPreparedEvent] = 0;
	g_storeAllScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_storeAllScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_storeAllScenario [SAuthPreparedEvent] = 0;
	g_storeAllScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_storeAllScenario [SSftpPreparedEvent] = 0;
	g_storeAllScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_storeAllScenario [SPwdPreparedEvent] = 0;
	g_storeAllScenario [SPwdFinishedEvent] = StoreAll_PwdFinishedEventHandler;
	g_storeAllScenario [SCwdPreparedEvent] = 0;
	g_storeAllScenario [SCwdFinishedEvent] = StoreAll_CwdFinishedEventHandler;
	g_storeAllScenario [SCdupPreparedEvent] = 0;
	g_storeAllScenario [SCdupFinishedEvent] = 0;
	g_storeAllScenario [SLPwdPreparedEvent] = 0;
	g_storeAllScenario [SLPwdFinishedEvent] = 0;
	g_storeAllScenario [SLCwdPreparedEvent] = 0;
	g_storeAllScenario [SLCwdFinishedEvent] = StoreAll_LcwdFinishedEventHandler;
	g_storeAllScenario [SLCdupPreparedEvent] = 0;
	g_storeAllScenario [SLCdupFinishedEvent] = 0;
	g_storeAllScenario [SMkdirPreparedEvent] = 0;
	g_storeAllScenario [SMkdirFinishedEvent] = 0;
	g_storeAllScenario [SRmdirPreparedEvent] = 0;
	g_storeAllScenario [SRmdirFinishedEvent] = 0;
	g_storeAllScenario [SListDirPreparedEvent] = 0;
	g_storeAllScenario [SListDirProgressEvent] = 0;
	g_storeAllScenario [SListDirFinishedEvent] = 0;
	g_storeAllScenario [SPutFilePreparedEvent] = StoreAll_PutFilePreparedEventHandler;
	g_storeAllScenario [SPutFileProgressEvent] = StoreAll_PutFileProgressEventHandler;
	g_storeAllScenario [SPutFileFinishedEvent] = StoreAll_PutFileFinishedEventHandler;
	g_storeAllScenario [SGetFilePreparedEvent] = 0;
	g_storeAllScenario [SGetFileProgressEvent] = 0;
	g_storeAllScenario [SGetFileFinishedEvent] = 0;
	g_storeAllScenario [SRenamePreparedEvent] = 0;
	g_storeAllScenario [SRenameFinishedEvent] = StoreAll_RenameFinishedEventHandler;
	g_storeAllScenario [SRemovePreparedEvent] = 0;
	g_storeAllScenario [SRemoveFinishedEvent] = StoreAll_RemoveFinishedEventHandler;
	g_storeAllScenario [SListPreparedEvent] = 0;
	g_storeAllScenario [SListFinishedEvent] = StoreAll_ListFinishedEventHandler;
	g_storeAllScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_storeAllScenario [SErrorEvent] = StoreAll_ErrorEventHandler;
	g_storeAllScenario [SRequestEvent] = Common_RequestEventHandler;
	g_storeAllScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_storeAllScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_storeAllScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitRetrieveAllScenario ()
{
	g_retrieveAllScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_retrieveAllScenario [SftpNullEvent] = 0;
	g_retrieveAllScenario [SStartupEvent] = Common_StartupEventHandler;
	g_retrieveAllScenario [SConnectPreparedEvent] = 0;
	g_retrieveAllScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_retrieveAllScenario [SHandshakePreparedEvent] = 0;
	g_retrieveAllScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_retrieveAllScenario [SHashPreparedEvent] = 0;
	g_retrieveAllScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_retrieveAllScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_retrieveAllScenario [SAuthPreparedEvent] = 0;
	g_retrieveAllScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_retrieveAllScenario [SSftpPreparedEvent] = 0;
	g_retrieveAllScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_retrieveAllScenario [SPwdPreparedEvent] = 0;
	g_retrieveAllScenario [SPwdFinishedEvent] = RetrieveAll_PwdFinishedEventHandler;
	g_retrieveAllScenario [SCwdPreparedEvent] = 0;
	g_retrieveAllScenario [SCwdFinishedEvent] = RetrieveAll_CwdFinishedEventHandler;
	g_retrieveAllScenario [SCdupPreparedEvent] = 0;
	g_retrieveAllScenario [SCdupFinishedEvent] = 0;
	g_retrieveAllScenario [SLPwdPreparedEvent] = 0;
	g_retrieveAllScenario [SLPwdFinishedEvent] = 0;
	g_retrieveAllScenario [SLCwdPreparedEvent] = 0;
	g_retrieveAllScenario [SLCwdFinishedEvent] = 0;
	g_retrieveAllScenario [SLCdupPreparedEvent] = 0;
	g_retrieveAllScenario [SLCdupFinishedEvent] = 0;
	g_retrieveAllScenario [SMkdirPreparedEvent] = 0;
	g_retrieveAllScenario [SMkdirFinishedEvent] = 0;
	g_retrieveAllScenario [SRmdirPreparedEvent] = 0;
	g_retrieveAllScenario [SRmdirFinishedEvent] = 0;
	g_retrieveAllScenario [SListDirPreparedEvent] = RetrieveAll_ListDirPreparedEventHandler;
	g_retrieveAllScenario [SListDirProgressEvent] = RetrieveAll_ListDirProgressEventHandler;
	g_retrieveAllScenario [SListDirFinishedEvent] = RetrieveAll_ListDirFinishedEventHandler;
	g_retrieveAllScenario [SPutFilePreparedEvent] = 0;
	g_retrieveAllScenario [SPutFileProgressEvent] = 0;
	g_retrieveAllScenario [SPutFileFinishedEvent] = 0;
	g_retrieveAllScenario [SGetFilePreparedEvent] = RetrieveAll_GetFilePreparedEventHandler;
	g_retrieveAllScenario [SGetFileProgressEvent] = RetrieveAll_GetFileProgressEventHandler;
	g_retrieveAllScenario [SGetFileFinishedEvent] = RetrieveAll_GetFileFinishedEventHandler;
	g_retrieveAllScenario [SRenamePreparedEvent] = 0;
	g_retrieveAllScenario [SRenameFinishedEvent] = 0;
	g_retrieveAllScenario [SRemovePreparedEvent] = 0;
	g_retrieveAllScenario [SRemoveFinishedEvent] = RetrieveAll_RemoveFinishedEventHandler;
	g_retrieveAllScenario [SListPreparedEvent] = 0;
	g_retrieveAllScenario [SListFinishedEvent] = RetrieveAll_ListFinishedEventHandler;
	g_retrieveAllScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_retrieveAllScenario [SErrorEvent] = RetrieveAll_ErrorEventHandler;
	g_retrieveAllScenario [SRequestEvent] = Common_RequestEventHandler;
	g_retrieveAllScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_retrieveAllScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_retrieveAllScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitDeleteScenario ()
{
	g_deleteScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_deleteScenario [SftpNullEvent] = 0;
	g_deleteScenario [SStartupEvent] = Common_StartupEventHandler;
	g_deleteScenario [SConnectPreparedEvent] = 0;
	g_deleteScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_deleteScenario [SHandshakePreparedEvent] = 0;
	g_deleteScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_deleteScenario [SHashPreparedEvent] = 0;
	g_deleteScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_deleteScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_deleteScenario [SAuthPreparedEvent] = 0;
	g_deleteScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_deleteScenario [SSftpPreparedEvent] = 0;
	g_deleteScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_deleteScenario [SPwdPreparedEvent] = 0;
	g_deleteScenario [SPwdFinishedEvent] = Delete_PwdFinishedEventHandler;
	g_deleteScenario [SCwdPreparedEvent] = 0;
	g_deleteScenario [SCwdFinishedEvent] = Delete_CwdFinishedEventHandler;
	g_deleteScenario [SCdupPreparedEvent] = 0;
	g_deleteScenario [SCdupFinishedEvent] = 0;
	g_deleteScenario [SLPwdPreparedEvent] = 0;
	g_deleteScenario [SLPwdFinishedEvent] = 0;
	g_deleteScenario [SLCwdPreparedEvent] = 0;
	g_deleteScenario [SLCwdFinishedEvent] = 0;
	g_deleteScenario [SLCdupPreparedEvent] = 0;
	g_deleteScenario [SLCdupFinishedEvent] = 0;
	g_deleteScenario [SMkdirPreparedEvent] = 0;
	g_deleteScenario [SMkdirFinishedEvent] = 0;
	g_deleteScenario [SRmdirPreparedEvent] = 0;
	g_deleteScenario [SRmdirFinishedEvent] = 0;
	g_deleteScenario [SListDirPreparedEvent] = 0;
	g_deleteScenario [SListDirProgressEvent] = 0;
	g_deleteScenario [SListDirFinishedEvent] = 0;
	g_deleteScenario [SPutFilePreparedEvent] = 0;
	g_deleteScenario [SPutFileProgressEvent] = 0;
	g_deleteScenario [SPutFileFinishedEvent] = 0;
	g_deleteScenario [SGetFilePreparedEvent] = 0;
	g_deleteScenario [SGetFileProgressEvent] = 0;
	g_deleteScenario [SGetFileFinishedEvent] = 0;
	g_deleteScenario [SRenamePreparedEvent] = 0;
	g_deleteScenario [SRenameFinishedEvent] = 0;
	g_deleteScenario [SRemovePreparedEvent] = 0;
	g_deleteScenario [SRemoveFinishedEvent] = Delete_RemoveFinishedEventHandler;
	g_deleteScenario [SListPreparedEvent] = 0;
	g_deleteScenario [SListFinishedEvent] = 0;
	g_deleteScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_deleteScenario [SErrorEvent] = Delete_ErrorEventHandler;
	g_deleteScenario [SRequestEvent] = Common_RequestEventHandler;
	g_deleteScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_deleteScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_deleteScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitDeleteAllScenario ()
{
	g_deleteAllScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_deleteAllScenario [SftpNullEvent] = 0;
	g_deleteAllScenario [SStartupEvent] = Common_StartupEventHandler;
	g_deleteAllScenario [SConnectPreparedEvent] = 0;
	g_deleteAllScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_deleteAllScenario [SHandshakePreparedEvent] = 0;
	g_deleteAllScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_deleteAllScenario [SHashPreparedEvent] = 0;
	g_deleteAllScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_deleteAllScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_deleteAllScenario [SAuthPreparedEvent] = 0;
	g_deleteAllScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_deleteAllScenario [SSftpPreparedEvent] = 0;
	g_deleteAllScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_deleteAllScenario [SPwdPreparedEvent] = 0;
	g_deleteAllScenario [SPwdFinishedEvent] = DeleteAll_PwdFinishedEventHandler;
	g_deleteAllScenario [SCwdPreparedEvent] = 0;
	g_deleteAllScenario [SCwdFinishedEvent] = DeleteAll_CwdFinishedEventHandler;
	g_deleteAllScenario [SCdupPreparedEvent] = 0;
	g_deleteAllScenario [SCdupFinishedEvent] = 0;
	g_deleteAllScenario [SLPwdPreparedEvent] = 0;
	g_deleteAllScenario [SLPwdFinishedEvent] = 0;
	g_deleteAllScenario [SLCwdPreparedEvent] = 0;
	g_deleteAllScenario [SLCwdFinishedEvent] = 0;
	g_deleteAllScenario [SLCdupPreparedEvent] = 0;
	g_deleteAllScenario [SLCdupFinishedEvent] = 0;
	g_deleteAllScenario [SMkdirPreparedEvent] = 0;
	g_deleteAllScenario [SMkdirFinishedEvent] = 0;
	g_deleteAllScenario [SRmdirPreparedEvent] = 0;
	g_deleteAllScenario [SRmdirFinishedEvent] = 0;
	g_deleteAllScenario [SListDirPreparedEvent] = DeleteAll_ListDirPreparedEventHandler;
	g_deleteAllScenario [SListDirProgressEvent] = DeleteAll_ListDirProgressEventHandler;
	g_deleteAllScenario [SListDirFinishedEvent] = DeleteAll_ListDirFinishedEventHandler;
	g_deleteAllScenario [SPutFilePreparedEvent] = 0;
	g_deleteAllScenario [SPutFileProgressEvent] = 0;
	g_deleteAllScenario [SPutFileFinishedEvent] = 0;
	g_deleteAllScenario [SGetFilePreparedEvent] = 0;
	g_deleteAllScenario [SGetFileProgressEvent] = 0;
	g_deleteAllScenario [SGetFileFinishedEvent] = 0;
	g_deleteAllScenario [SRenamePreparedEvent] = 0;
	g_deleteAllScenario [SRenameFinishedEvent] = 0;
	g_deleteAllScenario [SRemovePreparedEvent] = 0;
	g_deleteAllScenario [SRemoveFinishedEvent] = DeleteAll_RemoveFinishedEventHandler;
	g_deleteAllScenario [SListPreparedEvent] = 0;
	g_deleteAllScenario [SListFinishedEvent] = 0;
	g_deleteAllScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_deleteAllScenario [SErrorEvent] = DeleteAll_ErrorEventHandler;
	g_deleteAllScenario [SRequestEvent] = Common_RequestEventHandler;
	g_deleteAllScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_deleteAllScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_deleteAllScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitMakeWorkingEnvScenario ()
{
	g_makeWorkingEnvScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_makeWorkingEnvScenario [SftpNullEvent] = 0;
	g_makeWorkingEnvScenario [SStartupEvent] = Common_StartupEventHandler;
	g_makeWorkingEnvScenario [SConnectPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_makeWorkingEnvScenario [SHandshakePreparedEvent] = 0;
	g_makeWorkingEnvScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_makeWorkingEnvScenario [SHashPreparedEvent] = 0;
	g_makeWorkingEnvScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_makeWorkingEnvScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_makeWorkingEnvScenario [SAuthPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_makeWorkingEnvScenario [SSftpPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_makeWorkingEnvScenario [SPwdPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SPwdFinishedEvent] = MakeWorkingEnv_PwdFinishedEventHandler;
	g_makeWorkingEnvScenario [SCwdPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SCwdFinishedEvent] = MakeWorkingEnv_CwdFinishedEventHandler;
	g_makeWorkingEnvScenario [SCdupPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SCdupFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SLPwdPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SLPwdFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SLCwdPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SLCwdFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SLCdupPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SLCdupFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SMkdirPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SMkdirFinishedEvent] = MakeWorkingEnv_MkdirFinishedEventHandler;
	g_makeWorkingEnvScenario [SRmdirPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SRmdirFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SListDirPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SListDirProgressEvent] = 0;
	g_makeWorkingEnvScenario [SListDirFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SPutFilePreparedEvent] = 0;
	g_makeWorkingEnvScenario [SPutFileProgressEvent] = 0;
	g_makeWorkingEnvScenario [SPutFileFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SGetFilePreparedEvent] = 0;
	g_makeWorkingEnvScenario [SGetFileProgressEvent] = 0;
	g_makeWorkingEnvScenario [SGetFileFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SRenamePreparedEvent] = 0;
	g_makeWorkingEnvScenario [SRenameFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SRemovePreparedEvent] = 0;
	g_makeWorkingEnvScenario [SRemoveFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SListPreparedEvent] = 0;
	g_makeWorkingEnvScenario [SListFinishedEvent] = 0;
	g_makeWorkingEnvScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_makeWorkingEnvScenario [SErrorEvent] = MakeWorkingEnv_ErrorEventHandler;
	g_makeWorkingEnvScenario [SRequestEvent] = Common_RequestEventHandler;
	g_makeWorkingEnvScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_makeWorkingEnvScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_makeWorkingEnvScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

void SftpClientWorker::InitCleanDirScenario ()
{
	g_cleanDirScenario = new SftpClient::SftpCallback [SftpEventCount];

	g_cleanDirScenario [SftpNullEvent] = 0;
	g_cleanDirScenario [SStartupEvent] = Common_StartupEventHandler;
	g_cleanDirScenario [SConnectPreparedEvent] = 0;
	g_cleanDirScenario [SConnectedEvent] = Common_ConnectedEventHandler;
	g_cleanDirScenario [SHandshakePreparedEvent] = 0;
	g_cleanDirScenario [SHandshakeFinishedEvent] = Common_HandshakeEventHandler;
	g_cleanDirScenario [SHashPreparedEvent] = 0;
	g_cleanDirScenario [Sha1HashEvent] = Common_Sha1HashEventHandler;
	g_cleanDirScenario [Md5HashEvent] = Common_Md5HashEventHandler;
	g_cleanDirScenario [SAuthPreparedEvent] = 0;
	g_cleanDirScenario [SAuthFinishedEvent] = Common_ClientAuthenticatedEventHandler;
	g_cleanDirScenario [SSftpPreparedEvent] = 0;
	g_cleanDirScenario [SSftpFinishedEvent] = Common_SftpFinishedEventHandler;
	g_cleanDirScenario [SPwdPreparedEvent] = 0;
	g_cleanDirScenario [SPwdFinishedEvent] = CleanDir_PwdFinishedEventHandler;
	g_cleanDirScenario [SCwdPreparedEvent] = 0;
	g_cleanDirScenario [SCwdFinishedEvent] = CleanDir_CwdFinishedEventHandler;
	g_cleanDirScenario [SCdupPreparedEvent] = 0;
	g_cleanDirScenario [SCdupFinishedEvent] = CleanDir_CdupFinishedEventHandler;
	g_cleanDirScenario [SLPwdPreparedEvent] = 0;
	g_cleanDirScenario [SLPwdFinishedEvent] = 0;
	g_cleanDirScenario [SLCwdPreparedEvent] = 0;
	g_cleanDirScenario [SLCwdFinishedEvent] = 0;
	g_cleanDirScenario [SLCdupPreparedEvent] = 0;
	g_cleanDirScenario [SLCdupFinishedEvent] = 0;
	g_cleanDirScenario [SMkdirPreparedEvent] = 0;
	g_cleanDirScenario [SMkdirFinishedEvent] = 0;
	g_cleanDirScenario [SRmdirPreparedEvent] = 0;
	g_cleanDirScenario [SRmdirFinishedEvent] = CleanDir_RmdFinishedEventHandler;
	g_cleanDirScenario [SListDirPreparedEvent] = CleanDir_ListDirPreparedEventHandler;
	g_cleanDirScenario [SListDirProgressEvent] = CleanDir_ListDirProgressEventHandler;
	g_cleanDirScenario [SListDirFinishedEvent] = CleanDir_ListDirFinishedEventHandler;
	g_cleanDirScenario [SPutFilePreparedEvent] = 0;
	g_cleanDirScenario [SPutFileProgressEvent] = 0;
	g_cleanDirScenario [SPutFileFinishedEvent] = 0;
	g_cleanDirScenario [SGetFilePreparedEvent] = 0;
	g_cleanDirScenario [SGetFileProgressEvent] = 0;
	g_cleanDirScenario [SGetFileFinishedEvent] = 0;
	g_cleanDirScenario [SRenamePreparedEvent] = 0;
	g_cleanDirScenario [SRenameFinishedEvent] = 0;
	g_cleanDirScenario [SRemovePreparedEvent] = 0;
	g_cleanDirScenario [SRemoveFinishedEvent] = CleanDir_RemoveFinishedEventHandler;
	g_cleanDirScenario [SListPreparedEvent] = 0;
	g_cleanDirScenario [SListFinishedEvent] = 0;
	g_cleanDirScenario [SDisposedEvent] = Common_DisposedEventHandler;
	g_cleanDirScenario [SErrorEvent] = CleanDir_ErrorEventHandler;
	g_cleanDirScenario [SRequestEvent] = Common_RequestEventHandler;
	g_cleanDirScenario [SReplyEvent] = Common_ReplyEventHandler;
	g_cleanDirScenario [SCtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_cleanDirScenario [SCtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
}

SftpClientWorker::SftpClientWorker () : SftpClient (g_evnset)
{
	m_size = 0;
	m_sessionId = 0;

	m_ctrlTask = 0;
	m_request = 0;
	m_addrinfo = 0;

	m_tmpDirName = m_mkdPtr = m_mkdEnd = 0;
	m_dirPtr = m_dirEnd = 0;

	m_args = 0;
	m_ftpRequest = 0;
	m_ftpConnectionInfo = 0;
	m_emptyRequest = 0;

	listFile = 0;
	listFileName = 0;
	listFileSize = 0;
	listFileAddr = listFilePtr = 0;

	m_localDirEntryPtr = 0;
	m_localDirHandle = 0;

	m_listDirInfoStack = 0;
	m_listDirInfoStackSize = 0;
	m_listDirInfoStackDepth = 0;
	m_listContext = 0;
	m_cwdContext = 0;

	m_released = false;
	m_requestCode = FirstRequest;
}

SftpClientWorker::~SftpClientWorker ()
{
	Dispose ();
}

void SftpClientWorker::Dispose ()
{
	if (g_debug)
		cout << "SftpClientWorker::Dispose - client = " << clientId () << endl;

	m_ctrlTask = 0;
	if (m_request != 0)
		xdr_free ((xdrproc_t) xdr_FtpRequest, (char*) m_request);
	delete m_request;
	m_request = 0;
	if (m_addrinfo != 0)
		freeaddrinfo (m_addrinfo);
	m_addrinfo = 0;

	m_ftpRequest = 0;
	m_ftpConnectionInfo = 0;
	m_requestCode = FirstRequest;
	m_emptyRequest = 0;
	m_args = 0;
	UnregisterFtpScenario ();
	do
	{
		if (listFile != 0)
			fclose (listFile);
		listFile = 0;
		if (listFileName != 0)
		{
			unlink (listFileName);
			free (listFileName);
		}
		listFileName = 0;
		if (listFileAddr != 0)
			munmap (listFileAddr, listFileSize);
		listFileAddr = 0;
		listFileSize = 0;
		DecListDirStack ();
	}
	while (m_listDirInfoStackDepth > 0);

	if (m_tmpDirName != 0)
		free (m_tmpDirName);
	m_tmpDirName = m_mkdPtr = m_mkdEnd = 0;
	m_released = true;

	m_dirPtr = m_dirEnd = 0;
	m_emptyRequest = 0;

	if (m_localDirHandle != 0)
		closedir (m_localDirHandle);
	m_localDirHandle = 0;
}

void SftpClientWorker::Release (int result, const char* const msg [], const char* api, int errnum)
{
	char buffer [2048];
	if (m_released)
		return;
	m_released = true;
	m_ctrlTask = 0;

	buffer [0] = 0;
	bool sepind = false;
	sprintf (buffer, "S%06ld: --- released (code = %d, ", m_sessionId % (1000 * 1000), result);
	if (msg != 0)
		while (*msg != 0)
		{
			sprintf (buffer, "%s%s", buffer, *msg++);
			sepind = true;
		}
	if (api != 0)
	{
		if (sepind)
			strcat (buffer, ", ");
		sprintf (buffer, "%s'%s' failed", buffer, api);
		sepind = true;
	}
	if (errnum != 0)
	{
		if (sepind)
			strcat (buffer, ", ");
		sprintf (buffer, "%serrno = %d", buffer, errnum);
	}
	strcat (buffer, ")");
	cf_sc_printf (SC_SFTP, (result != 0) ? SC_ERR : SC_APL, "%s", buffer);
}

void SftpClientWorker::printtrailer (bool force)
{
	if (!(g_debug || force))
		return;

	time_t now = time (0);
	struct tm lt = *localtime (&now);

	fprintf (stdout, "%04d-%02d-%02d %02d:%02d:%02d S%06ld %s: ", lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
		lt.tm_hour, lt.tm_min, lt.tm_sec, m_sessionId % (1000 * 1000),
		((this == 0) || (m_ftpConnectionInfo == 0) || (m_ftpConnectionInfo->hostname == 0)) ?
			"" : m_ftpConnectionInfo->hostname);
}

void SftpClientWorker::Execute (FtpRequest* req, void* addInfo)
{
	char* reqtxt = FtpRequestInterface::GetFtpRequestText (req, '\t', '\n');
	SftpCallback* scenario;

	++m_jobCount;
	m_sessionId = ++g_sessionId;
	m_ftpRequest = req;
	m_ftpConnectionInfo = &req->connection;
	switch (m_requestCode = req->request.requestCode)
	{
	case CheckConnectivity:
		m_checkConnectivity = &req->request.FtpRequestUnion_u.checkConnectivity;
		scenario = g_checkConnectivityScenario;
		break;
	case MakeDir:
		m_makeDir = &req->request.FtpRequestUnion_u.makeDir;
		scenario = g_mkdirScenario;
		break;
	case GetDir:
		m_getDir = &req->request.FtpRequestUnion_u.getDir;
		scenario = g_getdirScenario;
		break;
	case StoreFile:
		m_storeFile = &req->request.FtpRequestUnion_u.storeFile;
		scenario = g_storeScenario;
		break;
	case RetrieveFile:
		m_retrieveFile = &req->request.FtpRequestUnion_u.retrieveFile;
		scenario = g_retrieveScenario;
		break;
	case StoreAllFiles:
		m_storeAllFiles = &req->request.FtpRequestUnion_u.storeAllFiles;
		scenario = g_storeAllScenario;
		break;
	case RetrieveAllFiles:
		m_retrieveAllFiles = &req->request.FtpRequestUnion_u.retrieveAllFiles;
		scenario = g_retrieveAllScenario;
		break;
	case DeleteFile:
		m_deleteFile = &req->request.FtpRequestUnion_u.deleteFile;
		scenario = g_deleteScenario;
		break;
	case DeleteAllFiles:
		m_deleteAllFiles = &req->request.FtpRequestUnion_u.deleteAllFiles;
		scenario = g_deleteAllScenario;
		break;
	case MakeWorkingEnv:
		m_makeWorkingEnv = &req->request.FtpRequestUnion_u.makeWorkingEnv;
		scenario = g_makeWorkingEnvScenario;
		break;
	case CleanDir:
		m_cleanDir = &req->request.FtpRequestUnion_u.cleanDir;
		scenario = g_cleanDirScenario;
		break;
	default:
		scenario = 0;
		m_emptyRequest = &req->request.FtpRequestUnion_u.empty;
		break;
	}
	if (reqtxt != 0)
	{
		Send (m_ctrlTask, new SftpJobInfo (clientId(), m_jobCount, m_sessionId, reqtxt));
		free (reqtxt);
	}
	else
		Send (m_ctrlTask, new SftpJobInfo (clientId(), m_jobCount, m_sessionId, (char*) "CANNOT GENERATE FTP REQUEST DESCRIPTION"));

	Continue (scenario);
}

void SftpClientWorker::Continue (SftpCallback* scenario)
{
	if (g_debug)
		cout << "CONTINUE - client = " << clientId ();
	if (scenario == 0)
	{
		if (g_debug)
			cout << ", scenarion == 0" << endl;
		Stop ();
//		Release (0, 0, 0, 0);
		return;
	}

	m_released = false;
	if (IsAlive () && (strcmp (m_ftpConnectionInfo->hostname, (char*) m_hostname.c_str ()) == 0))
	{
		if (g_debug)
			cout << ", alive" << endl;
		RegisterFtpScenario (scenario, this);
		++m_connContinued;
		m_cwdContext = CWD_HOME_DIR;
		ChangeWorkingDirectory ((m_homeDir = homeDir ()).c_str ());
	}
	else
	{
		if (g_debug)
			cout << ", dead" << endl;
		Stop ();
//		Release (0, 0, 0, 0);
		RegisterFtpScenario (scenario, this);
		m_hostname = m_ftpConnectionInfo->hostname;
		Start ();
	}
}

ssize_t SftpClientWorker::OpenListDirFile ()
{
	char tmpFileName [L_tmpnam];
	int fdes;

	sprintf (tmpFileName, "%s/list_XXXXXX", P_tmpdir);
	if ((fdes = mkstemp (tmpFileName)) < 0)
	{
		const char* const msgs [] =
		{ "cannot create tmp file name", 0 };

		Release (SftpTempnamFailed, msgs, 0, 0);
		return -1;
	}
	listFileName = strdup (tmpFileName);
	if ((listFile = fdopen (fdes, "w")) == 0)
	{
		const char* const msgs [] =
		{ "cannot open tmp file name", 0 };

		Release (SftpOpenTempnam, msgs, 0, 0);
		return -1;
	}
	return 0;
}

ssize_t SftpClientWorker::WriteListDirEntry (LIBSSH2_SFTP_ATTRIBUTES*& attrs, char*& buffer, char*& description)
{
	size_t size;

	//	save attributes
	if (fwrite (attrs, sizeof(LIBSSH2_SFTP_ATTRIBUTES), 1, listFile) != 1)
		return -1;
	//	save file name size and file name
	size = strlen (buffer) + 1;
	if (fwrite (&size, sizeof(size_t), 1, listFile) != 1)
		return -1;
	if (fwrite (buffer, sizeof(char), size, listFile) != size)
		return -1;
	//	save additional info size and additional info
	size = strlen (description) + 1;
	if (fwrite (&size, sizeof(size_t), 1, listFile) != 1)
		return -1;
	if (fwrite (description, sizeof(char), size, listFile) != size)
		return -1;
	return 0;
}

ssize_t SftpClientWorker::ReadListDirEntry (LIBSSH2_SFTP_ATTRIBUTES& attrs, char*& buffer, char*& description)
{
	ssize_t count = 0;
	ssize_t space = listFileSize - (listFilePtr - listFileAddr);
	char* listPtr = listFilePtr;
	ssize_t size;

	if (space == 0)
		return 0;

	if (space < (ssize_t) sizeof(LIBSSH2_SFTP_ATTRIBUTES))
		return -1;
	memcpy (&attrs, listPtr, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
	listPtr += sizeof(LIBSSH2_SFTP_ATTRIBUTES);
	space -= sizeof(LIBSSH2_SFTP_ATTRIBUTES);
	count += sizeof(LIBSSH2_SFTP_ATTRIBUTES);

	if (space < (ssize_t) sizeof(size_t))
		return -1;
	memcpy (&size, listPtr, sizeof(size_t));
	listPtr += sizeof(size_t);
	space -= sizeof(size_t);
	count += sizeof(size_t);

	if (space < size)
		return -1;
	buffer = listPtr;
	listPtr += size;
	space -= size;
	count += size;

	if (space < (ssize_t) sizeof(size_t))
		return -1;
	memcpy (&size, listPtr, sizeof(size_t));
	listPtr += sizeof(size_t);
	space -= sizeof(size_t);
	count += sizeof(size_t);

	if (space < size)
		return -1;
	description = listPtr;
	listPtr += size;
	space -= size;
	count += size;

	listFilePtr = listPtr;

	return count;
}

ssize_t SftpClientWorker::CloseListDirFile ()
{
	fclose (listFile);
	listFile = 0;
	struct stat sbuf;
	if (stat (listFileName, &sbuf) < 0)
		return -1;
	if ((listFile = fopen (listFileName, "rb")) == 0)
		return -1;
	listFileSize = sbuf.st_size;
	if ((listFileAddr = (char*) mmap (0, listFileSize, PROT_READ, MAP_SHARED, fileno (listFile), 0)) == 0)
		return -1;

	fclose (listFile);
	listFile = 0;
	listFilePtr = listFileAddr;
	return 0;
}

void SftpClientWorker::IncListDirStack ()
{
	if (m_listDirInfoStackDepth >= m_listDirInfoStackSize)
	{
		int stackSize = m_listDirInfoStackSize;
		stackSize >>= 4;
		stackSize++;
		stackSize <<= 4;
		ListDirInfoPtr stack = (ListDirInfoPtr) malloc (stackSize * sizeof(ListDirInfo));
		if (stack == 0)
		{
			const char* const msgs [] =
			{ "malloc failed", 0 };

			Release (SftpMallocFailed, msgs, 0, 0);
			return;
		}
		if (m_listDirInfoStack != 0)
		{
			memcpy (stack, m_listDirInfoStack, m_listDirInfoStackSize * sizeof(ListDirInfo));
			free (m_listDirInfoStack);
		}
		m_listDirInfoStack = stack;
		m_listDirInfoStackSize = stackSize;
	}
	m_listDirInfoStack [m_listDirInfoStackDepth] = m_listDirInfo;
	m_listDirInfoStackDepth++;
}

void SftpClientWorker::DecListDirStack ()
{
	if (m_listDirInfoStackDepth <= 0)
		return;
	m_listDirInfoStackDepth--;
	m_listDirInfo = m_listDirInfoStack [m_listDirInfoStackDepth];
}

void SftpClientWorker::Common_NotImplementedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	cf_sc_printf (SC_SFTP, SC_ERR, "--- not implemented, context = %d, status = %d, state = %d, event = %d", context,
		status, state, (int) (long) args [0]);
}

void SftpClientWorker::Common_DisposedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	const char* const msgs [] =
	{ "disposed", 0 };

	ftp->Stop ();
	Release (SftpDisposed, msgs, 0, 0);
}

void SftpClientWorker::Common_CtrlBusyTimerExpiredEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	const char* const msgs [] =
	{ "busy timer expired", 0 };

	ftp->Stop ();
	Release (SftpCtrlBusyTimerExpired, msgs, 0, 0);
}

void SftpClientWorker::Common_CtrlIdleTimerExpiredEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	const char* const msgs [] =
	{ "idle timer expired", 0 };

	if (g_debug)
		cout << "IDLE TIMER: client = " << (int) (long) args [0] << ", enabled = " << (int) (long) args [1]
			<< ", disabled = " << (int) (long) args [2] << endl;
	ftp->Stop ();
	Release (SftpCtrlIdleTimerExpired, msgs, 0, 0);
}

//
//	FTP CALLBACKS COMMON TO ALL SCENARIOS
//

void SftpClientWorker::Common_StartupEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	in_addr_t hostAddr;
	sockaddr_in* sockaddr4 = 0;
	sockaddr_in6* sockaddr6 = 0;

	for (addrinfo* results = m_addrinfo; results != 0; results = results->ai_next)
	{
		if (results->ai_socktype != SOCK_DGRAM)
			continue;
		switch (results->ai_family)
		{
		case AF_INET:
			sockaddr4 = (sockaddr_in*) results->ai_addr;
			break;
		case AF_INET6:
			sockaddr6 = (sockaddr_in6*) results->ai_addr;
			break;
		}
	}

	char buff[64];

	if (sockaddr4 != 0)
	{
		++m_connInited;
		cf_sc_printf (SC_SFTP, SC_APL, "--- starting A: (%s)", inet_ntop (AF_INET, &sockaddr4->sin_addr, buff, 64));

		sockaddr_in addr;
		memset (&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons (IPPORT_SFTP);
		addr.sin_addr = sockaddr4->sin_addr;
		ftp->Connect (m_ftpConnectionInfo->hostname, (sockaddr*) &addr, sizeof(sockaddr_in));
	}
	else if (sockaddr6 != 0)
	{
		++m_connInited;
		cf_sc_printf (SC_SFTP, SC_APL, "--- starting AAAA: (%s)", inet_ntop (AF_INET6, &sockaddr6->sin6_addr, buff, 64));

		sockaddr_in6 addr;
		memset (&addr, 0, sizeof(struct sockaddr_in6));
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons (IPPORT_SFTP);
		addr.sin6_addr = sockaddr6->sin6_addr;
		ftp->Connect (m_ftpConnectionInfo->hostname, (sockaddr*) &addr, sizeof(sockaddr_in6));
	}
	else if ((hostAddr = inet_addr (m_ftpConnectionInfo->hostname)) != (in_addr_t) - 1)
	{
		++m_connInited;
		cf_sc_printf (SC_SFTP, SC_APL, "--- starting AA: (%s)", m_ftpConnectionInfo->hostname);

		sockaddr_in addr;
		memset (&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons (IPPORT_SFTP);
		addr.sin_addr.s_addr = hostAddr;
		ftp->Connect (m_ftpConnectionInfo->hostname, (sockaddr*) &addr, sizeof(sockaddr_in));
	}
	else
	{
		++m_connUnresolved;
		const char* const args [] =
		{ "unresolved hostname: ", m_ftpConnectionInfo->hostname, 0 };
		Release (SftpUnresolvedName, args, 0, 0);
	}
//	else	ftp->Connect(m_ftpConnectionInfo->hostname);
}

void SftpClientWorker::Common_ConnectedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	++m_connEstablished;
	ftp->Handshake ();
}

void SftpClientWorker::Common_HandshakeEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	ftp->CheckHash (LIBSSH2_HOSTKEY_HASH_SHA1);
}

void SftpClientWorker::Common_Sha1HashEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	int i;
	char buffer [64];
	u_char* fingerprint;
	size_t pos, spos;
	string authentication;
	string securityMechanism;
	string publickeyPath;

	for (fingerprint = (u_char*) args, i = 0; i < 20; ++i, ++fingerprint)
		sprintf (buffer + 3 * i, " %02X", *fingerprint);
	buffer [3 * i] = 0;
	cf_sc_printf (SC_SFTP, SC_APL, "--- sha1 fingerprint: %s", buffer);

	authentication = m_ftpConnectionInfo->authentication;
	if (((pos = authentication.find ("security-mechanism=", 0)) == string::npos)
		|| ((spos = authentication.find (';', pos)) == string::npos))
	{
		// authentication string does not contain security mechanism info
		// we have to use password authentication mechanism
		ftp->LoginPassword (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
		return;
	}
	securityMechanism = authentication.substr (pos + 19, spos - pos - 19);
	if (securityMechanism == "password")
	{
		// security mechanism equals to 'password',
		// we have to use password authentication mechanism
		ftp->LoginPassword (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
		return;
	}
	// suppose that not-password means public key
	if (((pos = authentication.find ("publickey-path=", 0)) == string::npos)
		|| ((spos = authentication.find (';', pos)) == string::npos))
	{
		// authentication string does not contain public key path
		// it is supposed that password string contains public key path
		// to be used in public key authentication mechanism
		ftp->LoginPublickey (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
		return;
	}
	// extract public key path from authentication string and use it
	// in public key authentication mechanism
	publickeyPath = authentication.substr (pos + 15, spos - pos - 15);
	ftp->LoginPublickey (m_ftpConnectionInfo->user, publickeyPath);
}

void SftpClientWorker::Common_Md5HashEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	int i;
	char buffer [64];
	u_char* fingerprint;
	size_t pos, spos;
	string authentication;
	string securityMechanism;
	string publickeyPath;

	for (fingerprint = (u_char*) args, i = 0; i < 16; ++i, ++fingerprint)
		sprintf (buffer + 3 * i, " %02X", *fingerprint);
	buffer [3 * i] = 0;
	cf_sc_printf (SC_SFTP, SC_APL, "--- md5 fingerprint: %s", buffer);

	authentication = m_ftpConnectionInfo->authentication;
	if (((pos = authentication.find ("security-mechanism=", 0)) == string::npos)
		|| ((spos = authentication.find (';', pos)) == string::npos))
	{
		// authentication string does not contain security mechanism info
		// we have to use password authentication mechanism
		ftp->LoginPassword (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
		return;
	}
	securityMechanism = authentication.substr (pos + 19, spos - pos - 19);
	if (securityMechanism == "password")
	{
		// security mechanism equals to 'password',
		// we have to use password authentication mechanism
		ftp->LoginPassword (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
		return;
	}
	// suppose that not-password means public key
	if (((pos = authentication.find ("publickey-path=", 0)) == string::npos)
		|| ((spos = authentication.find (';', pos)) == string::npos))
	{
		// authentication string does not contain public key path
		// it is supposed that password string contains public key path
		// to be used in public key authentication mechanism
		ftp->LoginPublickey (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
		return;
	}
	// extract public key path from authentication string and use it
	// in public key authentication mechanism
	publickeyPath = authentication.substr (pos + 15, spos - pos - 15);
	ftp->LoginPublickey (m_ftpConnectionInfo->user, publickeyPath);
}

void SftpClientWorker::Common_ClientAuthenticatedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	ftp->CreateFtp ();
}

void SftpClientWorker::Common_SftpFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_homeDir = homeDir ();
	ftp->PrintWorkingDirectory ();
}

void SftpClientWorker::Common_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
}

void SftpClientWorker::Common_RequestEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (conversation ())
	{
		char** ptr;
		char* msg;
		int size;

		for (size = 0, ptr = (char**) args; *ptr != 0; size += strlen (*ptr++))
			;
		++size;
		if ((msg = (char*) malloc (size)) == 0)
			return;
		for (size = 0, ptr = (char**) args; *ptr != 0; size += strlen (*ptr++))
			strcpy (msg + size, *ptr);

		if (detailed ())
			cf_sc_printf (SC_SFTP, SC_ERR, "S%06d: <-- %s", m_sessionId % (1000 * 1000), msg);
		else
			cf_sc_printf (SC_SFTP, SC_APL, "S%06d: <-- %s", m_sessionId % (1000 * 1000), msg);
		free (msg);
	}

	if (m_ctrlTask != 0)
		Send (m_ctrlTask, new SftpClientRequest (m_sessionId, (char**) args));
}

void SftpClientWorker::Common_ReplyEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (conversation ())
	{
		char** ptr;
		char* msg;
		int size;

		for (size = 0, ptr = (char**) args; *ptr != 0; size += strlen (*ptr++))
			;
		++size;
		if ((msg = (char*) malloc (size)) == 0)
			return;
		for (size = 0, ptr = (char**) args; *ptr != 0; size += strlen (*ptr++))
			strcpy (msg + size, *ptr);

		if (detailed ())
			cf_sc_printf (SC_SFTP, SC_ERR, "S%06d: --> %s", m_sessionId % (1000 * 1000), msg);
		else
			cf_sc_printf (SC_SFTP, SC_APL, "S%06d: --> %s", m_sessionId % (1000 * 1000), msg);
		free (msg);
	}
	if (m_ctrlTask != 0)
		Send (m_ctrlTask, new SftpClientReply (m_sessionId, (char**) args));
}

//
//	CHECK CONNECTIVITY SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::CheckConnectivity_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context,
	SftpStatus status, SftpState state, void* args [])
{
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (".");
}

void SftpClientWorker::CheckConnectivity_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context,
	SftpStatus status, SftpState state, void* args [])
{
	const char* const msgs [] =
	{ "check connection finished", 0 };

	Release (0, msgs, 0, 0);
}

void SftpClientWorker::CheckConnectivity_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	GenErrorReport (args);

	const char* const msgarr [] =
	{ (char*) args [0], 0 };
	Release (SftpCheckConnectivityFailed, msgarr, (char*) args [1], (int) (long) args [2]);
}

//
//	MAKE DIRECTORY PATH SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::Mkdir_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_cwdContext = CWD_REMOTE_DIR;
	Mkdir_CwdFinishedEventHandler (ftp, context, status, state, args);
}

void SftpClientWorker::Mkdir_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
	case CWD_REMOTE_DIR:
		m_mkdEnd = m_mkdPtr = m_makeDir->remoteDirName;
		while (*m_mkdEnd == '/')
			m_mkdEnd++;
		if (m_mkdEnd != m_mkdPtr)
		{
			m_cwdContext = CWD_ROOT;
			ftp->ChangeWorkingDirectory ("/");
		}
		else
		{
			string dirName;
			while ((*m_mkdEnd != 0) && (*m_mkdEnd != '/'))
				m_mkdEnd++;
			dirName.assign (m_mkdPtr, m_mkdEnd - m_mkdPtr);
			while (*m_mkdEnd == '/')
				m_mkdEnd++;
			m_cwdContext = CWD_MKDIR;
			ftp->ChangeWorkingDirectory (dirName);
		}
		break;
	case CWD_MKDIR:
	case CWD_ROOT:
		m_mkdPtr = m_mkdEnd;
		if (*m_mkdEnd == 0)
		{
			const char* const msgs [] =
			{ "mkdir finished", 0 };

			Release (0, msgs, 0, 0);
			return;
		}
		{
			string dirName;
			while ((*m_mkdEnd != 0) && (*m_mkdEnd != '/'))
				m_mkdEnd++;
			dirName.assign (m_mkdPtr, m_mkdEnd - m_mkdPtr);
			while (*m_mkdEnd == '/')
				m_mkdEnd++;
			m_cwdContext = CWD_MKDIR;
			ftp->ChangeWorkingDirectory (dirName);
		}
		break;
	default:
	{
		const char* const msgs [] =
		{ "mkdir failed, illegal context", 0 };

		Release (SftpMkdirCwdFailed, msgs, 0, 0);
		return; // don't delete this statement
	}
		break;
	}
}

void SftpClientWorker::Mkdir_MkdirFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	string dirName;
	dirName.assign (m_mkdPtr, m_mkdEnd - m_mkdPtr);
	m_cwdContext = CWD_MKDIR;
	ftp->ChangeWorkingDirectory (dirName);
}

void SftpClientWorker::Mkdir_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if ((state == SCwdPrepared) && (m_cwdContext == CWD_MKDIR))
	{
		ftp->MakeDirectory (m_mkdPtr, 0777);
		return;
	}
	if (state < SAuthFinished)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpMkdirFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	GET DIRECTORY LISTING SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::Getdir_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_getDir->remoteDirName);
}

void SftpClientWorker::Getdir_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_getDir->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		ftp->ListDirectory ("");
		break;
	default:
		break;
	}
}

void SftpClientWorker::Getdir_ListPreparedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
}

void SftpClientWorker::Getdir_ListProgressEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
}

void SftpClientWorker::Getdir_ListFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	const char* const msgs [] =
	{ "getdir(detailed list) finished", 0 };

	Release (0, msgs, 0, 0);
}

void SftpClientWorker::Getdir_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpGetdirFailed, msg, (char*) args [1], (int) (long) args [2]);
}

//
//	STORE SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

/****************************************************************/
/* Many functions (FTP callbacks and helpers invoked by them) in*/
/* this file use common collection of input parameters. Their   */
/* meaning is explained in this comment block. Throughout of    */
/* this file they will be referred as 'common FTP parameters'.  */
/* These parameters are:                                        */
/* - ftp: object which executes FTP request. It is used to      */
/*   access its public properties (to get or set FTP transfer   */
/*   parameters) and methods (to invoke FTP protocol commands)  */
/* - context: there is a small set of different situations in   */
/*   which FTP callbacks are invoked, for example: when a FTP   */
/*   control message is sent or received, when a piece of data  */
/*   is being sent or received, etc. This situation is          */
/*   considered a context in which an FTP event is handeled and */
/*   its callback function is called accordingly. This          */
/*   information is very helpful whenever some event should     */
/*   trigger in different situations                            */
/* - status: indicates outcome of FTP action handeled by given  */
/*   callback method. It should be success, failure or error    */
/* - state: every possible FTP event is associated with its own */
/*   state. In fact they are synonyms although they have        */
/*   different names, for example: StorFinished is state name,  */
/*   StoreFinishedEvent is event name. There are many different */
/*   states although they are almost not used, especially by    */
/*   FTP object itself. But nevertheless they are helpful in    */
/*   some conflicting situations, for example in resolving some */
/*   errors which should not be considered true errors, like    */
/*   listing non existing files, etc.                           */
/* - args: collection of arguments. This collection is prepared */
/*   by 'FTP runtime' and is strongly dependent of FTP event    */
/*   which triggered callback execution. Every FTP event has its*/
/*   own collection of arguments: sizes of collections are      */
/*   different, types of members in different collections are   */
/*   different although they are typed as void* and must be cast*/
/*   appropriately before use                                    */
/****************************************************************/

/****************************************************************/
/* Function:    SftpClientWorker::Store_***EventHandler()       */
/* In:          common FTP parameters                           */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: These functions handle different stages of STORE*/
/*              file request                                    */
/****************************************************************/

/****************************************************************/
/* Function:    SftpClientWorker::Store_PwdFinishedEventHandler()*/
/* In:          common FTP parameters                           */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: PWD handler in 'store file' operation. This     */
/*              handler is called once the connection has been  */
/*              established and it is not invoked again for     */
/*              example for another FTP request if it shares    */
/*              alive FTP object with previous request. It      */
/*              remembers FTP home directory and tries to change*/
/*              working directory to remote directory contained */
/*              in 'store file' request                         */
/****************************************************************/

void SftpClientWorker::Store_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_storeFile->remoteDirName);
}

/****************************************************************/
/* Function:    SftpClientWorker::Store_CwdFinishedEventHandler()*/
/* In:          common FTP parameters                           */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: CWD handler in 'store file' operation. CWD is   */
/*              called many times during file transfer.         */
/*              Different invocations are indicated by case     */
/*              labels in switch statement which implements this*/
/*              callback:                                       */
/****************************************************************/

void SftpClientWorker::Store_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_storeFile->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		m_remoteDir = (char*) args;
		if (m_ftpRequest->flags & FTP_PROTECT_EXISTING_FILE)
		{
			m_listContext = LIST_PROTECT_EXISTING_FILE;
			ftp->ListFile (m_storeFile->remoteFileName); // NLIST
		}
		else
			ftp->RemoveFile (m_storeFile->remoteFileName);
		break;
	case CWD_WORKING_DIR:
		Store_ChangeLocalDir (ftp, context, status, state, args);
		break;
	default:
		break;
	}
}

/****************************************************************/
/* Function:    SftpClientWorker::Store_RemoveFinishedEventHandler()*/
/* In:          common FTP parameters                           */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: SRemoveFinishedEvent handler called by SFTP     */
/*              client in response to successfully removed file */
/*              during STORE file request. Function continues   */
/*              execution of STORE file request*/
/****************************************************************/

void SftpClientWorker::Store_RemoveFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		char* workingDirName = m_storeFile->workingDirName;
		if ((workingDirName == 0) || (*workingDirName == 0))
		{
			const char* const msgs [] =
			{ "store: working directory undefined", 0 };

			Release (SftpStoreWorkingNull, msgs, 0, 0);
			return;
		}
		string workingDir;
		if (*workingDirName == '/')
			workingDir = workingDirName;
		else
		{
			workingDir = homeDir ();
			workingDir += '/';
			workingDir += workingDirName;
		}
		m_cwdContext = CWD_WORKING_DIR;
		ftp->ChangeWorkingDirectory (workingDir);
	}
	else
		Store_ChangeLocalDir (ftp, context, status, state, args);
}

void SftpClientWorker::Store_ChangeLocalDir (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
	void* args [])
{
	ftp->ChangeLocalWorkingDirectory (m_storeFile->localDirName);
}

void SftpClientWorker::Store_LCwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	ftp->PutFile (m_storeFile->localFileName, m_storeFile->remoteFileName, 0777);
}

void SftpClientWorker::Store_PutFilePreparedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_start = ctx ()->realTime ();
	m_size = 0;
}

void SftpClientWorker::Store_PutFileProgressEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_size += (int) (long) args;
}

void SftpClientWorker::Store_PutFileFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_stop = ctx ()->realTime ();
	if (m_ftpRequest->flags & FTP_CHECK_FILE_SIZE)
	{
		string remotePathName;
		m_listContext = LIST_CHECK_FILE_SIZE;
		ftp->ListFile (m_storeFile->remoteFileName); // LIST
	}
	else
		Store_RenameWorkingFile (ftp, context, status, state, args);
}

void SftpClientWorker::Store_ListPreparedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
}

void SftpClientWorker::Store_ListFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_attrs = *((LIBSSH2_SFTP_ATTRIBUTES*) args);

	switch (m_listContext)
	{
	case LIST_PROTECT_EXISTING_FILE:
	{
		const char* const args [] =
		{ "store failed, destination file exists: ", (char*) m_storeFile->remoteFileName, 0 };
		Release (SftpFileExist, args, 0, 0);
	}
		break;
	case LIST_CHECK_FILE_SIZE:
		if (m_attrs.filesize == m_size)
			Store_RenameWorkingFile (ftp, context, status, state, args);
		else
		{
			const char* const args [] =
			{ "store failed, file size is not correct: ", (char*) m_storeFile->remoteFileName, 0 };
			Release (SftpFileSizeIncorrect, args, 0, 0);
		}
		break;
	}
}

void SftpClientWorker::Store_RenameWorkingFile (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		string srcFileName = m_storeFile->remoteFileName;
		string dstFileName = m_remoteDir;
		dstFileName += '/';
		dstFileName += m_storeFile->remoteFileName;
		ftp->RenameFile (srcFileName, dstFileName);
	}
	else
		Store_RemoveSourceFile (ftp, context, status, state, args);
}

void SftpClientWorker::Store_RenameFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	Store_RemoveSourceFile (ftp, context, status, state, args);
}

void SftpClientWorker::Store_RemoveSourceFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
	void* args [])
{
	if (m_ftpRequest->flags & FTP_REMOVE_SOURCE_FILE)
	{
		string localFile = m_storeFile->localDirName;
		localFile += '/';
		localFile += m_storeFile->localFileName;

		if (unlink (localFile.c_str ()) != 0)
		{
			const char* const args [] =
			{ "store failed, cannot remove local file '", (char*) localFile.c_str (), "'", 0 };
			Release (SftpUnlinkFailed, args, "unlink", errno);
			return; // don't delete this statement
		}
	}
	{
		const char* const msgs [] =
		{ "store finished", 0 };

		Release (0, msgs, 0, 0);
	}
	return; // don't delete this statement
}

void SftpClientWorker::Store_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (state)
	{
	case SRemovePrepared:
		//	'DELE remoteFile' failed: it's OK - continue with next request
		((SftpClientWorker*) ftp)->Store_RemoveFinishedEventHandler (ftp, context, status, state, args);
		return;
	case SListPrepared:
		if (m_listContext == LIST_PROTECT_EXISTING_FILE)
		{
			//	'LIST remoteFile' failed while checking file existence: it's OK - continue with next request
			((SftpClientWorker*) ftp)->Store_RemoveFinishedEventHandler (ftp, context, status, state, args);
			return;
		}
		break;
	default:
		if (state < SAuthFinished)
			Stop ();
		//	other failures are not permitted
		//	in case of connection and login errors dispose all resources
		//	save failed request into recovery queue
		//	for new FTP request
		break;
	}

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpStoreFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	RETRIEVE SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::Retrieve_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_retrieveFile->remoteDirName);
}

void SftpClientWorker::Retrieve_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_retrieveFile->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
	{
		char* localDir = m_retrieveFile->localDirName;
		if (localDir == 0)
			m_localDir = "";
		else
			m_localDir = localDir;
		char* localWorkingDir = m_retrieveFile->workingDirName;
		if (localWorkingDir == 0)
			m_localWorkingDir = "";
		else
			m_localWorkingDir = localWorkingDir;
	}
		if (m_ftpRequest->flags & FTP_PROTECT_EXISTING_FILE)
		{
			string localFile = m_retrieveFile->localDirName;
			localFile += '/';
			localFile += m_retrieveFile->localFileName;
			if (access (localFile.c_str (), F_OK) == 0)
			{
				const char* const args [] =
				{ "retrieve failed: cannot access local file '", m_retrieveFile->localFileName, "'", 0 };
				Release (SftpAccessFailed, args, "access", errno);
				return;
			}
		}
		m_remoteDir = (char*) args;
		if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
		{
			char* workingDir = m_retrieveFile->workingDirName;
			if ((workingDir == 0) || (*workingDir == 0))
			{
				const char* const msgs [] =
				{ "retrieve: empty working directory name", 0 };

				Release (SftpRetrieveWorkingNull, msgs, 0, 0);
				return;
			}
			string workingPath = m_retrieveFile->localDirName;
			if (*workingDir == '/')
				workingPath = m_retrieveFile->workingDirName;
			else
			{
				workingPath = m_retrieveFile->localDirName;
				workingPath += '/';
				workingPath += m_retrieveFile->workingDirName;
			}
			workingDir = (char*) workingPath.c_str ();
			if (access (workingDir, F_OK) != 0)
			{
				umask (0);
				if (mkdir (workingDir, 0777) != 0)
				{
					const char* const args [] =
					{ "retrieve: mkdir '", workingDir, "' failed", 0 };
					Release (SftpRetrieveMkdirFailed, args, "mkdir", errno);
					return;
				}
			}
		}
		Retrieve_GetFile (ftp, context, status, state, args);
		break;
	default:
		break;
	}
}

void SftpClientWorker::Retrieve_GetFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
	void* args [])
{
	string localPathName;
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		char* workingDirName = m_retrieveFile->workingDirName;
		if (*workingDirName == '/')
			localPathName = "";
		else
		{
			localPathName = m_retrieveFile->localDirName;
			localPathName += '/';
		}
		localPathName += workingDirName;
	}
	else
		localPathName = m_retrieveFile->localDirName;
	localPathName += '/';
	localPathName += m_retrieveFile->localFileName;
	ftp->GetFile (localPathName, m_retrieveFile->remoteFileName, 0777);
	m_start = ctx ()->realTime ();
	m_size = 0;
}

void SftpClientWorker::Retrieve_GetFilePreparedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
}

void SftpClientWorker::Retrieve_GetFileProgressEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_size += (int) (long) args;
}

void SftpClientWorker::Retrieve_GetFileFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_stop = ctx ()->realTime ();
	if (m_ftpRequest->flags & FTP_CHECK_FILE_SIZE)
		ftp->ListFile (m_retrieveFile->remoteFileName);
	else
		Retrieve_MoveRetrievedFile (ftp, context, status, state, args);
}

void SftpClientWorker::Retrieve_ListFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_attrs = *((LIBSSH2_SFTP_ATTRIBUTES*) args);
	Retrieve_MoveRetrievedFile (ftp, context, status, state, args);
}

void SftpClientWorker::Retrieve_MoveRetrievedFile (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		string workingPath = m_retrieveFile->localDirName;
		workingPath += '/';
		workingPath += m_retrieveFile->workingDirName;
		workingPath += '/';
		workingPath += m_retrieveFile->localFileName;
		string localPath = m_retrieveFile->localDirName;
		localPath += '/';
		localPath += m_retrieveFile->localFileName;
		if (rename (workingPath.c_str (), localPath.c_str ()) != 0)
		{
			const char* const args [] =
			{ "rename (", (char*) workingPath.c_str (), ", ", (char*) localPath.c_str (), ") failed", 0 };
			Release (SftpRetrieveRenameFailed, args, "rename", errno);
			return;
		}
	}
	if (m_ftpRequest->flags & FTP_REMOVE_SOURCE_FILE)
	{
		string remoteFile = m_remoteDir;
		remoteFile += '/';
		remoteFile += m_retrieveFile->remoteFileName;
		ftp->RemoveFile (remoteFile);
	}
	else
	{
		const char* const msgs [] =
		{ "retrieve finished", 0 };

		Release (0, msgs, 0, 0);
		return; // don't delete this statement
	}
}

void SftpClientWorker::Retrieve_RemoveFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	const char* const msgs [] =
	{ "retrieve finished", 0 };

	Release (0, msgs, 0, 0);
	return; // don't delete this statement
}

void SftpClientWorker::Retrieve_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (state < SAuthFinished)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpRetrieveFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	STORE ALL SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::StoreAll_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	ftp->ChangeLocalWorkingDirectory (m_storeAllFiles->localDirName);
}

void SftpClientWorker::StoreAll_LcwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_storeAllFiles->remoteDirName);
}

void SftpClientWorker::StoreAll_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		ftp->ChangeLocalWorkingDirectory (m_storeAllFiles->localDirName);
		break;
	case CWD_REMOTE_DIR:
		m_remoteDir = (char*) args;
		if ((m_localDirHandle = opendir (m_storeAllFiles->localDirName)) == 0)
		{
			const char* const args [] =
			{ "storeall: opendir (", m_storeAllFiles->localDirName, ") failed", 0 };
			Release (SftpStoreAllOpendirFailed, args, "opendir", errno);
			return;
		}
		StoreAll_StoreNextFile (ftp, context, status, state, args);
		break;
	case CWD_REMOTE_DIR_AGAIN:
		StoreAll_StoreNextFile (ftp, context, status, state, args);
		break;
	case CWD_WORKING_DIR:
		m_remoteWorkingDir = (char*) args;
		StoreAll_PutFile (ftp, context, status, state, args);
		break;
	default:
		break;
	}
}

void SftpClientWorker::StoreAll_StoreNextFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
	void* args [])
{
	ExceptionFileList* exceptions = m_storeAllFiles->exceptions;

	while (true)
	{
		if (readdir_r (m_localDirHandle, &m_localDirEntry, &m_localDirEntryPtr) != 0)
		{
			const char* const args [] =
			{ "storeall: readdir (", m_storeAllFiles->localDirName, ") failed", 0 };
			Release (SftpStoreAllReaddirFailed, args, "readdir_r", errno);
			return;
		}
		if (m_localDirEntryPtr == 0)
		{
			const char* const msgs [] =
			{ "storeall finished", 0 };

			Release (0, msgs, 0, 0);
			return;
		}

		ExceptionFileList* exPtr;
		bool exception;
		for (exception = false, exPtr = exceptions; exPtr != 0; exPtr = exPtr->next)
		{
			regex_t reg;
			if (regcomp (&reg, exPtr->fileNameTemplate, REG_EXTENDED) == 0)
			{
				regmatch_t match;
				memset (&match, 0, sizeof(regmatch_t));
				if (regexec (&reg, m_localDirEntry.d_name, 1, &match, 0) == 0)
					if (exPtr->action == ExceptionDeny)
						exception = true;
				regfree (&reg);
			}
		}

		if (exception)
			continue;

		string localPathName = m_storeAllFiles->localDirName;
		localPathName += '/';
		localPathName += m_localDirEntry.d_name;

		struct stat sbuff;
		if (stat (localPathName.c_str (), &sbuff) != 0)
		{
			const char* const args [] =
			{ "storeall: stat (", (char*) localPathName.c_str (), ") failed", 0 };
			Release (SftpStoreAllStatFailed, args, "stat", errno);
			return;
		}
		if (sbuff.st_mode & __S_IFREG)
			break;
	}

	if (m_ftpRequest->flags & FTP_PROTECT_EXISTING_FILE)
	{
		m_listContext = LIST_PROTECT_EXISTING_FILE;
		ftp->ListFile (m_localDirEntry.d_name);
	}
	else
		ftp->RemoveFile (m_localDirEntry.d_name);
}

void SftpClientWorker::StoreAll_PutFile (SftpClient *ftp, SftpContext context, SftpStatus status, SftpState state,
	void* args [])
{
	m_size = 0;
	m_start = ftp->ctx ()->realTime ();
	ftp->PutFile (m_localDirEntry.d_name, m_localDirEntry.d_name, 0777);
}

void SftpClientWorker::StoreAll_PutFilePreparedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
}

void SftpClientWorker::StoreAll_PutFileProgressEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	int n = (int) (long) args;
	m_size += n;
}

void SftpClientWorker::StoreAll_PutFileFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_stop = ftp->ctx ()->realTime ();
	if (m_ftpRequest->flags & FTP_CHECK_FILE_SIZE)
	{
		m_listContext = LIST_CHECK_FILE_SIZE;
		ftp->ListFile (m_localDirEntry.d_name);
	}
	else
		StoreAll_RenameWorkingFile (ftp, context, status, state, args);
}

void SftpClientWorker::StoreAll_ListFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_attrs = *((LIBSSH2_SFTP_ATTRIBUTES*) args);

	switch (m_listContext)
	{
	case LIST_CHECK_FILE_SIZE:
		if (m_attrs.filesize == m_size)
			StoreAll_RenameWorkingFile (ftp, context, status, state, args);
		else
		{
			const char* const msgs [] =
			{ "storeall: incorrect file size", 0 };

			Release (SftpStoreAllIllFileSize, msgs, 0, 0);
			return; // don't delete this statement
		}
		break;
	case LIST_PROTECT_EXISTING_FILE:
	{
		const char* const msgs [] =
		{ "storeall: file exist", 0 };

		Release (SftpStoreAllFileExist, msgs, 0, 0);
		return; // don't delete this statement
	}
	}
}

void SftpClientWorker::StoreAll_RenameWorkingFile (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		string srcPathName = m_remoteWorkingDir;
		srcPathName += '/';
		srcPathName += m_localDirEntry.d_name;
		string dstFileName = m_remoteDir;
		dstFileName += '/';
		dstFileName += m_localDirEntry.d_name;
		ftp->RenameFile (srcPathName, dstFileName);
	}
	else
		StoreAll_RemoveSourceFile (ftp, context, status, state, args);
}

void SftpClientWorker::StoreAll_RemoveFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		char* workingDirName = m_storeAllFiles->workingDirName;
		if ((workingDirName == 0) || (*workingDirName == 0))
		{
			const char* const msgs [] =
			{ "store: cannot check unnamed working directory", 0 };

			Release (SftpStoreAllWorkingNull, msgs, 0, 0);
			return;
		}
		string workingDir;
		if (*workingDirName == '/')
			workingDir = workingDirName;
		else
		{
			workingDir = homeDir ();
			workingDir += "/";
			workingDir += workingDirName;
		}
		m_cwdContext = CWD_WORKING_DIR;
		ftp->ChangeWorkingDirectory (workingDir);
	}
	else
		StoreAll_PutFile (ftp, context, status, state, args);
}

void SftpClientWorker::StoreAll_RenameFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	string remoteDir;
	if (*m_storeAllFiles->remoteDirName == '/')
		remoteDir = m_storeAllFiles->remoteDirName;
	else
	{
		remoteDir = homeDir ();
		remoteDir += '/';
		remoteDir += m_storeAllFiles->remoteDirName;
	}
	StoreAll_RemoveSourceFile (ftp, context, status, state, args);
}

void SftpClientWorker::StoreAll_RemoveSourceFile (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_REMOVE_SOURCE_FILE)
	{
		string localFile = m_storeAllFiles->localDirName;
		localFile += '/';
		localFile += m_localDirEntry.d_name;

		if (unlink (localFile.c_str ()) != 0)
		{
			const char* const args [] =
			{ "storeall: remove (", (char*) localFile.c_str (), ") failed", 0 };
			Release (SftpStoreAllUnlinkFailed, args, "unlink", errno);
			return;
		}
	}
	m_cwdContext = CWD_REMOTE_DIR_AGAIN;
	ftp->ChangeWorkingDirectory (m_remoteDir);
}

void SftpClientWorker::StoreAll_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (state)
	{
	case SRemovePrepared:
		//	'DELE remoteFile' or 'NLST remoteFile' failed: it's OK - continue with next request
		((SftpClientWorker*) ftp)->StoreAll_RemoveFinishedEventHandler (ftp, context, status, state, args);
		return;
	case SListPrepared:
		if (m_listContext == LIST_PROTECT_EXISTING_FILE)
		{
			ftp->RemoveFile (m_localDirEntry.d_name);
			return;
		}
		break;
	default:
		if (state < SAuthFinished)
			Stop ();
		//	other failures are not permitted
		//	in case of connection and login errors dispose all resources
		//	save failed request into recovery queue
		//	for new FTP request
		break;
	}

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpStoreAllFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	RETRIEVE SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::RetrieveAll_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	string remoteDir;
	if (*m_retrieveAllFiles->remoteDirName == '/')
		remoteDir = m_retrieveAllFiles->remoteDirName;
	else
	{
		remoteDir = homeDir ();
		remoteDir += '/';
		remoteDir += m_retrieveAllFiles->remoteDirName;
	}
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (remoteDir);
}

void SftpClientWorker::RetrieveAll_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
	{
		string remoteDir;
		if (*m_retrieveAllFiles->remoteDirName == '/')
			remoteDir = m_retrieveAllFiles->remoteDirName;
		else
		{
			remoteDir = homeDir ();
			remoteDir += '/';
			remoteDir += m_retrieveAllFiles->remoteDirName;
		}
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (remoteDir);
	}
		break;
	case CWD_REMOTE_DIR:
		m_remoteDir = (char*) args;
		ftp->ListDirectory (m_remoteDir);
		break;
	default:
	{
		const char* const msgs [] =
		{ "retrieveall: illegal cwd context", 0 };

		Release (SftpRetrieveAllCwdFailed, msgs, 0, 0);
		return; // don't delete this statement
	}
		break;
	}
}

void SftpClientWorker::RetrieveAll_ListDirPreparedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	OpenListDirFile ();
}

void SftpClientWorker::RetrieveAll_ListDirProgressEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	char* buffer = (char*) args [0];
	char* description = (char*) args [1];
	LIBSSH2_SFTP_ATTRIBUTES* attrs = (LIBSSH2_SFTP_ATTRIBUTES*) args [2];

	if (WriteListDirEntry (attrs, buffer, description) < 0)
	{
		const char* const msgs [] =
		{ "cannot save tmp file info", 0 };

		Release (SftpRetrieveAllWriteListDirEntry, msgs, 0, 0);
	}
}

void SftpClientWorker::RetrieveAll_ListDirFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_PROTECT_EXISTING_FILE)
	{
		string localFile = m_retrieveAllFiles->localDirName;
		localFile += '/';
		localFile += currentRemoteFile;
		if (access (localFile.c_str (), F_OK) == 0)
		{
			const char* const args [] =
			{ "retrieveall: access (", (char*) localFile.c_str (), ") failed", 0 };
			Release (SftpRetrieveAllAccessFailed, args, "access", errno);
			return;
		}
	}
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		char* workingDir = m_retrieveAllFiles->workingDirName;
		if ((workingDir == 0) || (*workingDir == 0))
		{
			const char* const msgs [] =
			{ "retrieveall: empty working dir name", 0 };

			Release (SftpRetrieveAllWorkingNull, msgs, 0, 0);
			return;
		}
		string workingPath = m_retrieveAllFiles->localDirName;
		workingPath += '/';
		workingPath += m_retrieveAllFiles->workingDirName;
		workingDir = (char*) workingPath.c_str ();
		if (access (workingDir, F_OK) != 0)
		{
			umask (0);
			if (mkdir (workingDir, 0777) != 0)
			{
				const char* const args [] =
				{ "retrieveall: mkdir (", workingDir, ") failed", 0 };
				Release (SftpRetrieveAllMkdirFailed, args, "mkdir", errno);
				return;
			}
		}
	}

	if (CloseListDirFile () == 0)
		RetrieveAll_RetrieveNextFile (ftp, context, status, state, args);
	else
	{
		char* fileName = (char*) listFileName;
		const char* const args [] =
		{ "retrieveall: mmap (", fileName, ") failed", 0 };
		Release (SftpRetrieveAllMapFailed, args, "mmap", errno);
		return;
	}
}

void SftpClientWorker::RetrieveAll_RetrieveNextFile (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	ExceptionFileList* exceptions = m_retrieveAllFiles->exceptions;

	while (true)
	{
		char* description = 0;
		LIBSSH2_SFTP_ATTRIBUTES attrs;
		ssize_t count = ReadListDirEntry (attrs, currentRemoteFile, description);

		if (count == 0) // end of temporary list
		{
			const char* const msgs [] =
			{ "retrieveall finished", 0 };

			Release (0, msgs, 0, 0);
			return;
		}

		if (count < 0) // error in temporary list
		{
			const char* const msgs [] =
			{ "retrieveall failed", 0 };

			Release (SftpRetrieveAllReadListDirEntry, msgs, 0, 0);
			return;
		}

		if (!(attrs.permissions & LIBSSH2_SFTP_S_IFREG))
			continue; // not regular file

		ExceptionFileList* exPtr;
		bool exception;
		for (exception = false, exPtr = exceptions; exPtr != 0; exPtr = exPtr->next)
		{
			regex_t reg;
			if (regcomp (&reg, exPtr->fileNameTemplate, REG_EXTENDED) == 0)
			{
				regmatch_t match;
				memset (&match, 0, sizeof(regmatch_t));
				if (regexec (&reg, currentRemoteFile, 1, &match, 0) == 0)
					if (exPtr->action == ExceptionDeny)
						exception = true;
				regfree (&reg);
			}
		}

		if (exception)
			continue; // file listed in exception list

		//	regular file
		string localPathName = m_retrieveAllFiles->localDirName;
		localPathName += '/';
		if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
		{
			localPathName += m_retrieveAllFiles->workingDirName;
			localPathName += '/';
		}
		localPathName += currentRemoteFile;
		string remotePathName;
		remotePathName = m_remoteDir;
		remotePathName += '/';
		remotePathName += currentRemoteFile;
		ftp->GetFile (localPathName, remotePathName, 007);
		m_start = ftp->ctx ()->realTime ();
		m_size = 0;
		break;
	}
}

void SftpClientWorker::RetrieveAll_GetFilePreparedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
}

void SftpClientWorker::RetrieveAll_GetFileProgressEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	int n = (int) (long) args;
	m_size += n;
}

void SftpClientWorker::RetrieveAll_GetFileFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_stop = ftp->ctx ()->realTime ();
	if (m_ftpRequest->flags & FTP_CHECK_FILE_SIZE)
	{
		ftp->ListFile (currentRemoteFile);
	}
	else
		RetrieveAll_MoveRetrievedFile (ftp, context, status, state, args);
}

void SftpClientWorker::RetrieveAll_ListFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	LIBSSH2_SFTP_ATTRIBUTES* attrs = (LIBSSH2_SFTP_ATTRIBUTES*) args;

	if (attrs->filesize == m_size)
		RetrieveAll_MoveRetrievedFile (ftp, context, status, state, args);
	else
	{
		const char* const msgs [] =
		{ "retrieveall: incorrect file size", 0 };

		Release (SftpRetrieveAllIllFileSize, msgs, 0, 0);
	}
}

void SftpClientWorker::RetrieveAll_MoveRetrievedFile (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		string workingPath = m_retrieveAllFiles->localDirName;
		workingPath += '/';
		workingPath += m_retrieveAllFiles->workingDirName;
		workingPath += '/';
		workingPath += currentRemoteFile;
		string localPath = m_retrieveAllFiles->localDirName;
		localPath += '/';
		localPath += currentRemoteFile;
		if (rename (workingPath.c_str (), localPath.c_str ()) != 0)
		{
			const char* const args [] =
			{ "retrieveall: rename (", (char*) workingPath.c_str (), ", ", (char*) localPath.c_str (), ") failed", 0 };
			Release (SftpRetrieveAllRenameFailed, args, "rename", errno);
			return;
		}
	}
	if (m_ftpRequest->flags & FTP_REMOVE_SOURCE_FILE)
		ftp->RemoveFile (currentRemoteFile);
	else
		RetrieveAll_RetrieveNextFile (ftp, context, status, state, args);
}

void SftpClientWorker::RetrieveAll_RemoveFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	RetrieveAll_RetrieveNextFile (ftp, context, status, state, args);
}

void SftpClientWorker::RetrieveAll_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (state == SGetFileFinished)
	{
		RetrieveAll_RetrieveNextFile (ftp, context, status, state, args);
		return;
	}
	if (state < SAuthFinished)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpRetrieveAllFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	DELETE SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::Delete_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_deleteFile->remoteDirName);
}

void SftpClientWorker::Delete_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_deleteFile->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		m_remoteDir = (char*) args;
		ftp->RemoveFile (m_deleteFile->remoteFileName);
		break;
	default:
	{
		const char* const msgs [] =
		{ "delete: illegal cwd context", 0 };

		Release (SftpDeleteCwdFailed, msgs, 0, 0);
		return; // don't delete this statement
	}
		break;
	}
}

void SftpClientWorker::Delete_RemoveFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	const char* const msgs [] =
	{ "delete finished", 0 };

	Release (0, msgs, 0, 0);
	return; // don't delete this statement
}

void SftpClientWorker::Delete_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (state < SAuthFinished)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpDeleteFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	DELETE ALL SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::DeleteAll_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_deleteAllFiles->remoteDirName);
}

void SftpClientWorker::DeleteAll_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_deleteAllFiles->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		m_remoteDir = (char*) args;
		ftp->ListDirectory ("");
		break;
	default:
	{
		const char* const msgs [] =
		{ "deleteall: illegal cwd context", 0 };

		Release (SftpDeleteAllCwdContext, msgs, 0, 0);
		return; // don't delete this statement
	}
		break;
	}
}

void SftpClientWorker::DeleteAll_ListDirPreparedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	OpenListDirFile ();
}

void SftpClientWorker::DeleteAll_ListDirProgressEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	char* buffer = (char*) args [0];
	char* description = (char*) args [1];
	LIBSSH2_SFTP_ATTRIBUTES* attrs = (LIBSSH2_SFTP_ATTRIBUTES*) args [2];

	if (WriteListDirEntry (attrs, buffer, description) < 0)
	{
		const char* const msgs [] =
		{ "cannot save tmp file info", 0 };

		Release (SftpDeleteAllWriteDirEntry, msgs, 0, 0);
	}
}

void SftpClientWorker::DeleteAll_ListDirFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (CloseListDirFile () == 0)
		DeleteAll_DeleteNextFile (ftp, context, status, state, args);
	else
	{
		char* fileName = (char*) listFileName;
		const char* const args [] =
		{ "retrieveall: mmap (", fileName, ") failed", 0 };
		Release (SftpDeleteAllMapFailed, args, "mmap", errno);
		return;
	}
}

void SftpClientWorker::DeleteAll_DeleteNextFile (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	ExceptionFileList* exceptions = m_deleteAllFiles->exceptions;

	while (true)
	{
		char* description = 0;
		LIBSSH2_SFTP_ATTRIBUTES attrs;
		ssize_t count = ReadListDirEntry (attrs, currentRemoteFile, description);

		if (count == 0) // end of temporary list
		{
			const char* const msgs [] =
			{ "retrieveall finished", 0 };

			Release (0, msgs, 0, 0);
			return;
		}

		if (count < 0) // error in temporary list
		{
			const char* const msgs [] =
			{ "cannot read list dir entry", 0 };

			Release (SftpDeleteAllReadDirEntry, msgs, 0, 0);
			return;
		}

		if (!(attrs.permissions & LIBSSH2_SFTP_S_IFREG))
			continue; // not regular file

		ExceptionFileList* exPtr;
		bool exception;
		for (exception = false, exPtr = exceptions; exPtr != 0; exPtr = exPtr->next)
		{
			regex_t reg;
			if (regcomp (&reg, exPtr->fileNameTemplate, REG_EXTENDED) == 0)
			{
				regmatch_t match;
				memset (&match, 0, sizeof(regmatch_t));
				if (regexec (&reg, currentRemoteFile, 1, &match, 0) == 0)
					if (exPtr->action == ExceptionDeny)
						exception = true;
				regfree (&reg);
			}
		}

		if (exception)
			continue; // file listed in exception list

		//	regular file
		string remotePathName;
		remotePathName = m_remoteDir;
		remotePathName += '/';
		remotePathName += currentRemoteFile;
		ftp->RemoveFile (remotePathName);
		break;
	}
}

void SftpClientWorker::DeleteAll_RemoveFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	DeleteAll_DeleteNextFile (ftp, context, status, state, args);
}

void SftpClientWorker::DeleteAll_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (state == SRemoveFinished)
	{
		DeleteAll_DeleteNextFile (ftp, context, status, state, args);
		return;
	}
	if (state < SAuthFinished)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpDeleteAllFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	MAKE WORKING ENVIRONMENT SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::MakeWorkingEnv_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	MakeWorkingEnv_CwdFirstRemoteComponent (ftp);
}

void SftpClientWorker::MakeWorkingEnv_CwdFirstRemoteComponent (SftpClient *ftp)
{
	if (m_tmpDirName != 0)
		free (m_tmpDirName);
	m_tmpDirName = (char*) malloc (strlen (m_makeWorkingEnv->remoteDirName) + 1);
	if (m_tmpDirName == 0)
	{
		MakeWorkingEnv_Finished (ftp);
		return;
	}
	strcpy (m_tmpDirName, m_makeWorkingEnv->remoteDirName);

	char* ptr;
	ptr = m_dirPtr = m_tmpDirName;
	while ((*ptr == '/') || (*ptr == '\\'))
		++ptr;
	if ((m_dirEnd = strchr (ptr, '/')) == 0)
		m_dirEnd = strchr (ptr, '\\');
	if (m_dirEnd != 0)
		*m_dirEnd = 0;
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_dirPtr);
}

void SftpClientWorker::MakeWorkingEnv_CwdNextRemoteComponent (SftpClient *ftp)
{
	if (m_dirEnd == 0)
		MakeWorkingEnv_CwdLastRemoteComponent (ftp);
	else
	{
		m_dirPtr = m_dirEnd + 1;
		while ((*m_dirPtr == '/') || (*m_dirPtr == '\\'))
			++m_dirPtr;
		if (*m_dirPtr == 0)
			MakeWorkingEnv_CwdLastRemoteComponent (ftp);
		else
		{
			char* ptr = m_dirPtr;
			if ((m_dirEnd = strchr (ptr, '/')) == 0)
				m_dirEnd = strchr (ptr, '\\');
			if (m_dirEnd != 0)
				*m_dirEnd = 0;
			m_cwdContext = CWD_REMOTE_DIR;
			ftp->ChangeWorkingDirectory (m_dirPtr);
		}
	}
}

void SftpClientWorker::MakeWorkingEnv_CwdLastRemoteComponent (SftpClient *ftp)
{
	m_cwdContext = CWD_HOME_DIR_AGAIN;
	ftp->ChangeWorkingDirectory (homeDir ());
}

void SftpClientWorker::MakeWorkingEnv_CwdFirstWorkingComponent (SftpClient *ftp)
{
	if (m_tmpDirName != 0)
		free (m_tmpDirName);
	m_tmpDirName = (char*) malloc (strlen (m_makeWorkingEnv->workingDirName) + 1);
	if (m_tmpDirName == 0)
	{
		MakeWorkingEnv_Finished (ftp);
		return;
	}
	strcpy (m_tmpDirName, m_makeWorkingEnv->workingDirName);

	char* ptr;
	ptr = m_dirPtr = m_tmpDirName;
	while ((*ptr == '/') || (*ptr == '\\'))
		++ptr;
	if ((m_dirEnd = strchr (ptr, '/')) == 0)
		m_dirEnd = strchr (ptr, '\\');
	if (m_dirEnd != 0)
		*m_dirEnd = 0;
	m_cwdContext = CWD_WORKING_DIR;
	ftp->ChangeWorkingDirectory (m_dirPtr);
}

void SftpClientWorker::MakeWorkingEnv_CwdNextWorkingComponent (SftpClient *ftp)
{
	if (m_dirEnd == 0)
		MakeWorkingEnv_Finished (ftp);
	else
	{
		m_dirPtr = m_dirEnd + 1;
		while ((*m_dirPtr == '/') || (*m_dirPtr == '\\'))
			++m_dirPtr;
		if (*m_dirPtr == 0)
			MakeWorkingEnv_Finished (ftp);
		else
		{
			char* ptr = m_dirPtr;
			if ((m_dirEnd = strchr (ptr, '/')) == 0)
				m_dirEnd = strchr (ptr, '\\');
			if (m_dirEnd != 0)
				*m_dirEnd = 0;
			m_cwdContext = CWD_WORKING_DIR;
			ftp->ChangeWorkingDirectory (m_dirPtr);
		}
	}
}

void SftpClientWorker::MakeWorkingEnv_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		MakeWorkingEnv_CwdFirstRemoteComponent (ftp);
		break;
	case CWD_HOME_DIR_AGAIN:
		MakeWorkingEnv_CwdFirstWorkingComponent (ftp);
		break;
	case CWD_REMOTE_DIR:
		MakeWorkingEnv_CwdNextRemoteComponent (ftp);
		break;
	case CWD_WORKING_DIR:
		MakeWorkingEnv_CwdNextWorkingComponent (ftp);
		break;
	}
}

void SftpClientWorker::MakeWorkingEnv_MkdirFinishedEventHandler (SftpClient *ftp, SftpContext context,
	SftpStatus status, SftpState state, void* args [])
{
	ftp->ChangeWorkingDirectory (m_dirPtr);
}

void SftpClientWorker::MakeWorkingEnv_Finished (SftpClient *ftp)
{
	const char* const msgs [] =
	{ "make working environment finished", 0 };

	Release (0, msgs, 0, 0);
}

void SftpClientWorker::MakeWorkingEnv_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (state)
	{
	case SCwdPrepared:
	{
		switch (m_cwdContext)
		{
		case CWD_REMOTE_DIR:
		case CWD_WORKING_DIR:
			//	'CWD working' failed: it's OK - make directory and try again
			ftp->MakeDirectory (m_dirPtr, 0777);
			return;
			break;
		default:
			break;
		}
	}
		break;
	default:
		if (state < SAuthFinished)
			Stop ();
		//	other failures are not permitted
		//	in case of connection and login errors dispose all resources
		//	save failed request into recovery queue
		//	for new FTP request
		break;
	}

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpMakeWorkingEnvFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	CLEAN DIRECTORY SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void SftpClientWorker::CleanDir_PwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	m_listDirInfoStackDepth = 0;
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_cleanDir->remoteDirName);
}

void SftpClientWorker::CleanDir_CwdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_listDirInfoStackDepth = 0;
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_cleanDir->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		ftp->ListDirectory ("");
		break;
	}
}

void SftpClientWorker::CleanDir_ListDirPreparedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	OpenListDirFile ();
}

void SftpClientWorker::CleanDir_ListDirProgressEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	char* buffer = (char*) args [0];
	char* description = (char*) args [1];
	LIBSSH2_SFTP_ATTRIBUTES* attrs = (LIBSSH2_SFTP_ATTRIBUTES*) args [2];

	if (WriteListDirEntry (attrs, buffer, description) < 0)
	{
		const char* msgs [] =
		{ "cannot save tmp file info", 0 };

		Release (SftpCleanDirWriteDirEntry, msgs, 0, 0);
	}
}

void SftpClientWorker::CleanDir_ListDirFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (CloseListDirFile () < 0)
	{
		char* fileName = (char*) listFileName;
		const char* const args [] =
		{ "retrieveall: mmap (", fileName, ") failed", 0 };
		Release (SftpCleanDirMapFailed, args, "mmap", errno);
		return;
	}
	CleanDir_DeleteNextComponent (ftp, context, status, state, args);
}

void SftpClientWorker::CleanDir_CdupFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_DESTRUCTIVE_OPERATION)
	{
		ftp->DeleteDirectory (currentRemoteFile);
	}
	else
		CleanDir_DeleteNextComponent (ftp, context, status, state, args);
}

void SftpClientWorker::CleanDir_RmdFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	CleanDir_DeleteNextComponent (ftp, context, status, state, args);
}

void SftpClientWorker::CleanDir_RemoveFinishedEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	CleanDir_DeleteNextComponent (ftp, context, status, state, args);
}

void SftpClientWorker::CleanDir_DeleteNextComponent (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	ExceptionFileList* exceptions = 0;

	while (true)
	{
		char* description = 0;
		LIBSSH2_SFTP_ATTRIBUTES attrs;
		ssize_t count = ReadListDirEntry (attrs, currentRemoteFile, description);

		if (count == 0) // end of temporary list
		{
			if (m_listDirInfoStackDepth <= 0)
			{
				const char* const msgs [] =
				{ "clean directory finished finished", 0 };

				Release (0, msgs, 0, 0);
			}
			else
			{
				if (listFile != 0)
					fclose (listFile);
				listFile = 0;
				if (listFileName != 0)
				{
					unlink (listFileName);
					free (listFileName);
				}
				listFileName = 0;
				if (listFileAddr != 0)
					munmap (listFileAddr, listFileSize);
				listFileAddr = 0;
				listFileSize = 0;
				DecListDirStack ();
				ftp->ChangeToParentDirectory ();
			}
			return;
		}

		if (count < 0) // error in temporary list
		{
			const char* const msgs [] =
			{ "cannot read list dir entry", 0 };

			Release (SftpCleanDirReadDirEntry, msgs, 0, 0);
			return;
		}

		if (attrs.permissions & LIBSSH2_SFTP_S_IFREG)
		{
			ExceptionFileList* exPtr;
			bool exception;
			for (exception = false, exPtr = exceptions; exPtr != 0; exPtr = exPtr->next)
			{
				regex_t reg;
				if (regcomp (&reg, exPtr->fileNameTemplate, REG_EXTENDED) == 0)
				{
					regmatch_t match;
					memset (&match, 0, sizeof(regmatch_t));
					if (regexec (&reg, currentRemoteFile, 1, &match, 0) == 0)
						if (exPtr->action == ExceptionDeny)
							exception = true;
					regfree (&reg);
				}
			}

			if (exception)
				continue; // file listed in exception list

			//	regular file
			ftp->RemoveFile (currentRemoteFile);
		}
		else if (attrs.permissions & LIBSSH2_SFTP_S_IFDIR)
		{
			if (!(m_ftpRequest->flags & FTP_RECURSIVE_OPERATION))
				continue;
			if (strcmp (currentRemoteFile, ".") == 0)
				continue;
			if (strcmp (currentRemoteFile, "..") == 0)
				continue;
			IncListDirStack ();
			ftp->ChangeWorkingDirectory (currentRemoteFile);
		}
		else
			continue;
		break;
	}
}

void SftpClientWorker::CleanDir_ErrorEventHandler (SftpClient *ftp, SftpContext context, SftpStatus status,
	SftpState state, void* args [])
{
	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (SftpCleanDirFailed, msg, (char*) args [1], (int) (long) args [2]);
}

void SftpClientWorker::GenErrorReport (void*args [])
{
	char* msg = (char*) args [0];
	char* sys = (char*) args [1];
	int err = (int) (long) args [2];
	char* reqtxt = FtpRequestInterface::GetFtpRequestText (m_ftpRequest, '\t', '\n');

	if (reqtxt != 0)
	{
		cf_sc_printf (SC_SFTP, SC_ERR, "SSH job %d.%d.%d (S%06d) --- SFTP failed:\n\t%s\n\t%s%s, %s%d\n\t%s",
			clientId (), m_jobCount, m_sessionId, m_sessionId % (1000 * 1000), msg, sys, ((err != 0) ? " failed" : ""),
			((err != 0) ? "errno = " : ""), err, reqtxt);
		free (reqtxt);
	}
	else
	{
		reqtxt = (char*) "CANNOT CREATE SFTP REQUEST TEXT";
		cf_sc_printf (SC_SFTP, SC_ERR, "SSH job %d.%d.%d (S%06d) --- SFTP failed:\n\t%s\n\t%s%s, %s%d\n\t%s",
			clientId (), m_jobCount, m_sessionId, m_sessionId % (1000 * 1000), msg, sys, ((err != 0) ? " failed" : ""),
			((err != 0) ? "errno = " : ""), err, reqtxt);
	}
}

void SftpClientWorker::StartTask ()
{
	ctx (((MpxTaskMultiplexer*) mpx ())->ctx ());
}

void SftpClientWorker::StopTask ()
{
	Dispose ();
}

void SftpClientWorker::HandleInviteRequestEvent (MpxEventBase* event)
{
	SftpInviteRequest* inviteRequest = dynamic_cast <SftpInviteRequest*> (event);
	if (inviteRequest == 0)
		return;

	if (m_ctrlTask != 0)
		Send ((MpxTaskBase*) event->src (), new SftpInviteReply (false));
	else
	{
		if (m_request != 0)
			xdr_free ((xdrproc_t)xdr_FtpRequest, (char*) m_request);
		delete m_request;
		if ((m_request = inviteRequest->request ()) == 0)
			Send ((MpxTaskBase*) event->src (), new SftpInviteReply (false));
		else
		{
			Send (m_ctrlTask = (MpxTaskBase*) event->src (), new SftpInviteReply (true));
			MpxWorkingQueue::Put (new MpxJobGetAddrInfo (this, m_request->connection.hostname, "", 0));
		}
	}
}

void SftpClientWorker::HandleJobFinishedEvent (MpxEventBase* event)
{
	MpxJobFinishedEvent* jobFinishedEvent = dynamic_cast < MpxJobFinishedEvent* > (event);
	if (jobFinishedEvent == 0)
		return;

	MpxJobGetAddrInfo* jobGetAddrInfo = dynamic_cast < MpxJobGetAddrInfo* > (jobFinishedEvent->job());
	if (jobGetAddrInfo == 0)
		return;

	m_addrinfo = jobGetAddrInfo->results();
	delete jobGetAddrInfo;
	Execute (m_request);
}

} /* namespace sftp */
