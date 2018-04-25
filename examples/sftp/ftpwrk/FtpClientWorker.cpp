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

#include "FtpClientWorker.h"
#include <ftplog/SftpLog.h>

namespace sftp
{

//	statistical data
u_long FtpClientWorker::m_connContinued = 0;
u_long FtpClientWorker::m_connInited = 0;
u_long FtpClientWorker::m_connEstablished = 0;
u_long FtpClientWorker::m_userAuthenticated = 0;
u_long FtpClientWorker::m_connDisposed = 0;
u_long FtpClientWorker::m_connUnresolved = 0;

//	all currently available scenarios
FtpClient::FtpCallback* FtpClientWorker::g_undefinedScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_checkConnectivityScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_mkdirScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_getdirScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_storeScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_retrieveScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_storeAllScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_retrieveAllScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_deleteScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_deleteAllScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_makeWorkingEnvScenario = 0;
FtpClient::FtpCallback* FtpClientWorker::g_cleanDirScenario = 0;

//	scenario initializer - must follow (not precede) scenario declarations
FtpClientWorker* FtpClientWorker::g_scenarioInitializer = new FtpClientWorker (true);

EventDescriptor FtpClientWorker::g_evntab [] =
{
	{ AnyState, SftpInviteRequest::EventCode, HandleInviteRequestEvent },
	{ AnyState, MpxJobFinishedEvent::EventCode, HandleJobFinishedEvent },
	{ 0, 0, 0 },
};

MpxTaskBase::evnset FtpClientWorker::g_evnset = MpxTaskBase::CreateEventSet (FtpClientWorker::g_evntab);

void FtpClientWorker::GenerateReport ()
{
	cout << "\tFTP continued: " << m_connContinued << endl;
	cout << "\tFTP inited: " << m_connInited << endl;
	cout << "\tFTP established: " << m_connEstablished << endl;
	cout << "\tFTP authenticated: " << m_userAuthenticated << endl;
	cout << "\tFTP disposed: " << m_connDisposed << endl;
	cout << "\tFTP unresolved: " << m_connUnresolved << endl;
}

FtpClientWorker::FtpClientWorker (bool initialize) : FtpClient (g_evnset)
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

void FtpClientWorker::InitUndefinedScenario ()
{
	g_undefinedScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_undefinedScenario [FtpNullEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StartupEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ConnectedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [UserPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [PassPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AcctPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ClientAuthenticatedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [CwdPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [CwdFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [CdupPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [CdupFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SmntPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SmntFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ReinPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ReinFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [QuitPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [QuitFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [PortPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [PortFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [PasvPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [PasvFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [TypePreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [TypeFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StruPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StruFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ModePreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ModeFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RetrPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RetrProgressEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RetrFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StorPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StorProgressEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StorFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StouPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StouProgressEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StouFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AppePreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AppeProgressEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AppeFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ListPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ListProgressEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ListFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [NlstPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [NlstProgressEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [NlstFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AlloPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AlloFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SystPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [SystFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StatPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [StatFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RnfrPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RnfrFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RntoPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RntoFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AborPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AborFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [DelePreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [DeleFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RmdPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RmdFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [MkdPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [MkdFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [PwdPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [PwdFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [HelpPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [HelpFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [NoopPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [NoopFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [FeatPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [FeatFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [OptsPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [OptsFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AuthPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AuthFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AdatPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [AdatFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ProtPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ProtFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [PbszPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [PbszFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [CccPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [CccFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [MicPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [MicFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ConfPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ConfFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [EncPreparedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [EncFinishedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [DisposedEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ErrorEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [RequestEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ReplyEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [CtrlBusyTimerExpiredEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [CtrlIdleTimerExpiredEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [DataTimerExpiredEvent] = Common_NotImplementedEventHandler;
	g_undefinedScenario [ListenTimerExpiredEvent] = Common_NotImplementedEventHandler;
}

void FtpClientWorker::InitCheckConnectivityScenario ()
{
	g_checkConnectivityScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_checkConnectivityScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StartupEvent] = Common_StartupEventHandler;
	g_checkConnectivityScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_checkConnectivityScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_checkConnectivityScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [CwdFinishedEvent] = CheckConnectivity_CwdFinishedEventHandler;
	g_checkConnectivityScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_checkConnectivityScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [TypeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [RetrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [RetrProgressEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [RetrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StorPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StorProgressEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StorFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ListPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ListProgressEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ListFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [NlstProgressEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [NlstFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_checkConnectivityScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [RntoFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [DeleFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [MkdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [PwdFinishedEvent] = CheckConnectivity_PwdFinishedEventHandler;
	g_checkConnectivityScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_checkConnectivityScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_checkConnectivityScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_checkConnectivityScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_checkConnectivityScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_checkConnectivityScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_checkConnectivityScenario [ErrorEvent] = CheckConnectivity_ErrorEventHandler;
	g_checkConnectivityScenario [RequestEvent] = Common_RequestEventHandler;
	g_checkConnectivityScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_checkConnectivityScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_checkConnectivityScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_checkConnectivityScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_checkConnectivityScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitMkdirScenario ()
{
	g_mkdirScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_mkdirScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StartupEvent] = Common_StartupEventHandler;
	g_mkdirScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_mkdirScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_mkdirScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [CwdFinishedEvent] = Mkdir_CwdFinishedEventHandler;
	g_mkdirScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_mkdirScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [TypeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [RetrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [RetrProgressEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [RetrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StorPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StorProgressEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StorFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ListPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ListProgressEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ListFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [NlstProgressEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [NlstFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_mkdirScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [RntoFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [DeleFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [MkdFinishedEvent] = Mkdir_MkdFinishedEventHandler;
	g_mkdirScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [PwdFinishedEvent] = Mkdir_PwdFinishedEventHandler;
	g_mkdirScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_mkdirScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_mkdirScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_mkdirScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_mkdirScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_mkdirScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_mkdirScenario [ErrorEvent] = Mkdir_ErrorEventHandler;
	g_mkdirScenario [RequestEvent] = Common_RequestEventHandler;
	g_mkdirScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_mkdirScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_mkdirScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_mkdirScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_mkdirScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitGetdirScenario ()
{
	g_getdirScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_getdirScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StartupEvent] = Common_StartupEventHandler;
	g_getdirScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_getdirScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_getdirScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [CwdFinishedEvent] = Getdir_CwdFinishedEventHandler;
	g_getdirScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_getdirScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [TypeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [RetrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [RetrProgressEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [RetrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StorPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StorProgressEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StorFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ListPreparedEvent] = Getdir_ListPreparedEventHandler;
	g_getdirScenario [ListProgressEvent] = Getdir_ListProgressEventHandler;
	g_getdirScenario [ListFinishedEvent] = Getdir_ListFinishedEventHandler;
	g_getdirScenario [NlstPreparedEvent] = Getdir_NlstPreparedEventHandler;
	g_getdirScenario [NlstProgressEvent] = Getdir_NlstProgressEventHandler;
	g_getdirScenario [NlstFinishedEvent] = Getdir_NlstFinishedEventHandler;
	g_getdirScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_getdirScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [RntoFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [DeleFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [MkdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [PwdFinishedEvent] = Getdir_PwdFinishedEventHandler;
	g_getdirScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_getdirScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_getdirScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_getdirScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_getdirScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_getdirScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_getdirScenario [ErrorEvent] = Getdir_ErrorEventHandler;
	g_getdirScenario [RequestEvent] = Common_RequestEventHandler;
	g_getdirScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_getdirScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_getdirScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_getdirScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_getdirScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitStoreScenario ()
{
	g_storeScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_storeScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [StartupEvent] = Common_StartupEventHandler;
	g_storeScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_storeScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_storeScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [CwdFinishedEvent] = Store_CwdFinishedEventHandler;
	g_storeScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_storeScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [TypeFinishedEvent] = Store_TypeFinishedEventHandler;
	g_storeScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [RetrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [RetrProgressEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [RetrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [StorPreparedEvent] = Store_StorPreparedEventHandler;
	g_storeScenario [StorProgressEvent] = Store_StorProgressEventHandler;
	g_storeScenario [StorFinishedEvent] = Store_StorFinishedEventHandler;
	g_storeScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ListPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ListProgressEvent] = Store_ListProgressEventHandler;
	g_storeScenario [ListFinishedEvent] = Store_ListFinishedEventHandler;
	g_storeScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [NlstProgressEvent] = Store_NlstProgressEventHandler;
	g_storeScenario [NlstFinishedEvent] = Store_NlstFinishedEventHandler;
	g_storeScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_storeScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [RntoFinishedEvent] = Store_RntoFinishedEventHandler;
	g_storeScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [DeleFinishedEvent] = Store_DeleFinishedEventHandler;
	g_storeScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [MkdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [PwdFinishedEvent] = Store_PwdFinishedEventHandler;
	g_storeScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_storeScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_storeScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_storeScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_storeScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_storeScenario [ErrorEvent] = Store_ErrorEventHandler;
	g_storeScenario [RequestEvent] = Common_RequestEventHandler;
	g_storeScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_storeScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_storeScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_storeScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_storeScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitRetrieveScenario ()
{
	g_retrieveScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_retrieveScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [StartupEvent] = Common_StartupEventHandler;
	g_retrieveScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_retrieveScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_retrieveScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [CwdFinishedEvent] = Retrieve_CwdFinishedEventHandler;
	g_retrieveScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_retrieveScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [TypeFinishedEvent] = Retrieve_TypeFinishedEventHandler;
	g_retrieveScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [RetrPreparedEvent] = Retrieve_RetrPreparedEventHandler;
	g_retrieveScenario [RetrProgressEvent] = Retrieve_RetrProgressEventHandler;
	g_retrieveScenario [RetrFinishedEvent] = Retrieve_RetrFinishedEventHandler;
	g_retrieveScenario [StorPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [StorProgressEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [StorFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ListPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ListProgressEvent] = Retrieve_ListProgressEventHandler;
	g_retrieveScenario [ListFinishedEvent] = Retrieve_ListFinishedEventHandler;
	g_retrieveScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [NlstProgressEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [NlstFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_retrieveScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [RntoFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [DeleFinishedEvent] = Retrieve_DeleFinishedEventHandler;
	g_retrieveScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [MkdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [PwdFinishedEvent] = Retrieve_PwdFinishedEventHandler;
	g_retrieveScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_retrieveScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_retrieveScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_retrieveScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_retrieveScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_retrieveScenario [ErrorEvent] = Retrieve_ErrorEventHandler;
	g_retrieveScenario [RequestEvent] = Common_RequestEventHandler;
	g_retrieveScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_retrieveScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_retrieveScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_retrieveScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_retrieveScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitStoreAllScenario ()
{
	g_storeAllScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_storeAllScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [StartupEvent] = Common_StartupEventHandler;
	g_storeAllScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_storeAllScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_storeAllScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [CwdFinishedEvent] = StoreAll_CwdFinishedEventHandler;
	g_storeAllScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_storeAllScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [TypeFinishedEvent] = StoreAll_TypeFinishedEventHandler;
	g_storeAllScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [RetrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [RetrProgressEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [RetrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [StorPreparedEvent] = StoreAll_StorPreparedEventHandler;
	g_storeAllScenario [StorProgressEvent] = StoreAll_StorProgressEventHandler;
	g_storeAllScenario [StorFinishedEvent] = StoreAll_StorFinishedEventHandler;
	g_storeAllScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ListPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ListProgressEvent] = StoreAll_ListProgressEventHandler;
	g_storeAllScenario [ListFinishedEvent] = StoreAll_ListFinishedEventHandler;
	g_storeAllScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [NlstProgressEvent] = StoreAll_NlstProgressEventHandler;
	g_storeAllScenario [NlstFinishedEvent] = StoreAll_NlstFinishedEventHandler;
	g_storeAllScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_storeAllScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [RntoFinishedEvent] = StoreAll_RntoFinishedEventHandler;
	g_storeAllScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [DeleFinishedEvent] = StoreAll_DeleFinishedEventHandler;
	g_storeAllScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [MkdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [PwdFinishedEvent] = StoreAll_PwdFinishedEventHandler;
	g_storeAllScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_storeAllScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_storeAllScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_storeAllScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_storeAllScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_storeAllScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_storeAllScenario [ErrorEvent] = StoreAll_ErrorEventHandler;
	g_storeAllScenario [RequestEvent] = Common_RequestEventHandler;
	g_storeAllScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_storeAllScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_storeAllScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_storeAllScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_storeAllScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitRetrieveAllScenario ()
{
	g_retrieveAllScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_retrieveAllScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [StartupEvent] = Common_StartupEventHandler;
	g_retrieveAllScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_retrieveAllScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_retrieveAllScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [CwdFinishedEvent] = RetrieveAll_CwdFinishedEventHandler;
	g_retrieveAllScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_retrieveAllScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [TypeFinishedEvent] = RetrieveAll_TypeFinishedEventHandler;
	g_retrieveAllScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [RetrPreparedEvent] = RetrieveAll_RetrPreparedEventHandler;
	g_retrieveAllScenario [RetrProgressEvent] = RetrieveAll_RetrProgressEventHandler;
	g_retrieveAllScenario [RetrFinishedEvent] = RetrieveAll_RetrFinishedEventHandler;
	g_retrieveAllScenario [StorPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [StorProgressEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [StorFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ListPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ListProgressEvent] = RetrieveAll_ListProgressEventHandler;
	g_retrieveAllScenario [ListFinishedEvent] = RetrieveAll_ListFinishedEventHandler;
	g_retrieveAllScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [NlstProgressEvent] = RetrieveAll_NlstProgressEventHandler;
	g_retrieveAllScenario [NlstFinishedEvent] = RetrieveAll_NlstFinishedEventHandler;
	g_retrieveAllScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_retrieveAllScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [RntoFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [DeleFinishedEvent] = RetrieveAll_DeleFinishedEventHandler;
	g_retrieveAllScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [MkdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [PwdFinishedEvent] = RetrieveAll_PwdFinishedEventHandler;
	g_retrieveAllScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_retrieveAllScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_retrieveAllScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_retrieveAllScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_retrieveAllScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_retrieveAllScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_retrieveAllScenario [ErrorEvent] = RetrieveAll_ErrorEventHandler;
	g_retrieveAllScenario [RequestEvent] = Common_RequestEventHandler;
	g_retrieveAllScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_retrieveAllScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_retrieveAllScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_retrieveAllScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_retrieveAllScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitDeleteScenario ()
{
	g_deleteScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_deleteScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StartupEvent] = Common_StartupEventHandler, g_deleteScenario [ConnectedEvent] =
		Common_ConnectedEventHandler;
	g_deleteScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_deleteScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [CwdFinishedEvent] = Delete_CwdFinishedEventHandler;
	g_deleteScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_deleteScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [TypeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [RetrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [RetrProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [RetrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StorPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StorProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StorFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ListPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ListProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ListFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [NlstProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [NlstFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_deleteScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [RntoFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [DeleFinishedEvent] = Delete_DeleFinishedEventHandler;
	g_deleteScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [MkdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [PwdFinishedEvent] = Delete_PwdFinishedEventHandler;
	g_deleteScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_deleteScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_deleteScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_deleteScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_deleteScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_deleteScenario [ErrorEvent] = Delete_ErrorEventHandler;
	g_deleteScenario [RequestEvent] = Common_RequestEventHandler;
	g_deleteScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_deleteScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_deleteScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_deleteScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_deleteScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitDeleteAllScenario ()
{
	g_deleteAllScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_deleteAllScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StartupEvent] = Common_StartupEventHandler;
	g_deleteAllScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_deleteAllScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_deleteAllScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [CwdFinishedEvent] = DeleteAll_CwdFinishedEventHandler;
	g_deleteAllScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_deleteAllScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [TypeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [RetrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [RetrProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [RetrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StorPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StorProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StorFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ListPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ListProgressEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ListFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [NlstProgressEvent] = DeleteAll_NlstProgressEventHandler;
	g_deleteAllScenario [NlstFinishedEvent] = DeleteAll_NlstFinishedEventHandler;
	g_deleteAllScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_deleteAllScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [RntoFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [DeleFinishedEvent] = DeleteAll_DeleFinishedEventHandler;
	g_deleteAllScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [MkdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [PwdFinishedEvent] = DeleteAll_PwdFinishedEventHandler;
	g_deleteAllScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_deleteAllScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_deleteAllScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_deleteAllScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_deleteAllScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_deleteAllScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_deleteAllScenario [ErrorEvent] = DeleteAll_ErrorEventHandler;
	g_deleteAllScenario [RequestEvent] = Common_RequestEventHandler;
	g_deleteAllScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_deleteAllScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_deleteAllScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_deleteAllScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_deleteAllScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitMakeWorkingEnvScenario ()
{
	g_makeWorkingEnvScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_makeWorkingEnvScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StartupEvent] = Common_StartupEventHandler;
	g_makeWorkingEnvScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_makeWorkingEnvScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_makeWorkingEnvScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [CwdFinishedEvent] = MakeWorkingEnv_CwdFinishedEventHandler;
	g_makeWorkingEnvScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [CdupFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_makeWorkingEnvScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [TypeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [RetrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [RetrProgressEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [RetrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StorPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StorProgressEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StorFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ListPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ListProgressEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ListFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [NlstProgressEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [NlstFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_makeWorkingEnvScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [RntoFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [DeleFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [RmdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [MkdFinishedEvent] = MakeWorkingEnv_MkdFinishedEventHandler;
	g_makeWorkingEnvScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [PwdFinishedEvent] = MakeWorkingEnv_PwdFinishedEventHandler;
	g_makeWorkingEnvScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_makeWorkingEnvScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_makeWorkingEnvScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_makeWorkingEnvScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_makeWorkingEnvScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_makeWorkingEnvScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_makeWorkingEnvScenario [ErrorEvent] = MakeWorkingEnv_ErrorEventHandler;
	g_makeWorkingEnvScenario [RequestEvent] = Common_RequestEventHandler;
	g_makeWorkingEnvScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_makeWorkingEnvScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_makeWorkingEnvScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_makeWorkingEnvScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_makeWorkingEnvScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

void FtpClientWorker::InitCleanDirScenario ()
{
	g_cleanDirScenario = new FtpClient::FtpCallback [FtpEventCount];

	g_cleanDirScenario [FtpNullEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StartupEvent] = Common_StartupEventHandler;
	g_cleanDirScenario [ConnectedEvent] = Common_ConnectedEventHandler;
	g_cleanDirScenario [UserPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [PassPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AcctPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ClientAuthenticatedEvent] = Common_ClientAuthenticatedEventHandler;
	g_cleanDirScenario [CwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [CwdFinishedEvent] = CleanDir_CwdFinishedEventHandler;
	g_cleanDirScenario [CdupPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [CdupFinishedEvent] = CleanDir_CdupFinishedEventHandler;
	g_cleanDirScenario [SmntPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [SmntFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ReinPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ReinFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [QuitPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [QuitFinishedEvent] = Common_QuitFinishedEventHandler;
	g_cleanDirScenario [PortPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [PortFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [PasvPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [PasvFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [TypePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [TypeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StruPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StruFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ModePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ModeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [RetrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [RetrProgressEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [RetrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StorPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StorProgressEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StorFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StouPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StouProgressEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StouFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AppePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AppeProgressEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AppeFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ListPreparedEvent] = CleanDir_ListPreparedEventHandler;
	g_cleanDirScenario [ListProgressEvent] = CleanDir_ListProgressEventHandler;
	g_cleanDirScenario [ListFinishedEvent] = CleanDir_ListFinishedEventHandler;
	g_cleanDirScenario [NlstPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [NlstProgressEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [NlstFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AlloPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AlloFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [SystPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [SystFinishedEvent] = Common_SystFinishedEventHandler;
	g_cleanDirScenario [StatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [StatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [RnfrPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [RnfrFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [RntoPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [RntoFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AborPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AborFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [DelePreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [DeleFinishedEvent] = CleanDir_DeleFinishedEventHandler;
	g_cleanDirScenario [RmdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [RmdFinishedEvent] = CleanDir_RmdFinishedEventHandler;
	g_cleanDirScenario [MkdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [MkdFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [PwdPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [PwdFinishedEvent] = CleanDir_PwdFinishedEventHandler;
	g_cleanDirScenario [HelpPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [HelpFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [NoopPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [NoopFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [FeatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [FeatFinishedEvent] = Common_FeatFinishedEventHandler;
	g_cleanDirScenario [OptsPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [OptsFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AuthPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AuthFinishedEvent] = Common_AuthFinishedEventHandler;
	g_cleanDirScenario [AdatPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [AdatFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ProtPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ProtFinishedEvent] = Common_ProtFinishedEventHandler;
	g_cleanDirScenario [PbszPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [PbszFinishedEvent] = Common_PbszFinishedEventHandler;
	g_cleanDirScenario [CccPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [CccFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [MicPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [MicFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ConfPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [ConfFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [EncPreparedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [EncFinishedEvent] = (FtpClient::FtpCallback) 0;
	g_cleanDirScenario [DisposedEvent] = Common_DisposedEventHandler;
	g_cleanDirScenario [ErrorEvent] = CleanDir_ErrorEventHandler;
	g_cleanDirScenario [RequestEvent] = Common_RequestEventHandler;
	g_cleanDirScenario [ReplyEvent] = Common_ReplyEventHandler;
	g_cleanDirScenario [CtrlBusyTimerExpiredEvent] = Common_CtrlBusyTimerExpiredEventHandler;
	g_cleanDirScenario [CtrlIdleTimerExpiredEvent] = Common_CtrlIdleTimerExpiredEventHandler;
	g_cleanDirScenario [DataTimerExpiredEvent] = Common_DataTimerExpiredEventHandler;
	g_cleanDirScenario [ListenTimerExpiredEvent] = Common_ListenTimerExpiredEventHandler;
}

#if	defined (FTPQUEUEMAP)

FtpClientWorker::FtpClientWorker(u_char* addrinfo, u_int size)
{
	m_callback = NULL;
	m_args = NULL;
	m_ftpRequest = NULL;
	m_ftpConnectionInfo = NULL;
	m_emptyRequest = NULL;
	m_size = 0;
	m_newLine = false;
	m_localDir = NULL;
	m_localDirEntryPtr = NULL;
	m_currentRemoteFile = NULL;
	m_listBuffer = NULL;
	m_listPtr = NULL;
	m_listEnd = NULL;
	m_released = false;

	passiveEnabled(true);
}

#else	// FTPQUEUEMAP
FtpClientWorker::FtpClientWorker () : FtpClient (g_evnset)
{
	m_sessionId = 0;
	m_dirEnd = 0;
	m_mkdPtr = 0;
	m_cwdContext = 0;
	m_sysType = UnknownSystem;
	m_listRead = 0;
	m_requestCode = FirstRequest;
	m_listContext = 0;
	m_startIndex = 0;
	m_listStart = 0;
	m_listName = 0;
	m_dirPtr = 0;
	m_mkdEnd = 0;
	m_addrinfo = 0;
	m_request = 0;

	m_ctrlTask = 0;

	m_homeDir = ".";

	m_args = NULL;
	m_ftpRequest = NULL;
	m_ftpConnectionInfo = NULL;
	m_emptyRequest = NULL;
	m_size = 0;
	m_newLine = false;
	m_localDir = NULL;
	m_localDirEntryPtr = NULL;
	m_currentRemoteFile = NULL;
	m_listBuffer = NULL;
	m_listPtr = NULL;
	m_listEnd = NULL;
	m_released = false;
	m_ctrlProtection = false;
	m_dataProtection = false;

	passiveEnabled (true);

	m_tmpDirName = NULL;
	m_stack = 0;
	m_stackSize = 0;
	m_stackDepth = -1;
}

#endif	// FTPQUEUEMAP
FtpClientWorker::~FtpClientWorker ()
{
	Dispose ();
	if (m_listBuffer != NULL)
		free (m_listBuffer);
	m_listBuffer = NULL;
	m_listPtr = NULL;
	m_listEnd = NULL;
}

void FtpClientWorker::Dispose ()
{
	if (g_debug)
		cout << "FtpClientWorker::Dispose" << endl;

	if (m_ctrlTask != 0)
		Send (m_ctrlTask, new SftpClientStop (m_sessionId, 0));
	m_ctrlTask = 0;
	if (m_request != 0)
		xdr_free ((xdrproc_t) xdr_FtpRequest, (char*) m_request);
	m_request = 0;
	if (m_addrinfo != 0)
		freeaddrinfo (m_addrinfo);
	m_addrinfo = 0;

	m_ftpRequest = NULL;
	m_ftpConnectionInfo = NULL;
	m_requestCode = FirstRequest;
	m_emptyRequest = NULL;
	if (m_localDir != NULL)
		closedir (m_localDir);
	m_localDir = NULL;
	m_localDirEntryPtr = NULL;
	m_currentRemoteFile = NULL;
	m_listPtr = m_listBuffer;
	UnregisterFtpScenario ();
}

void FtpClientWorker::Release (int result, const char* const msg [], const char* api, int errnum)
{
	char buffer [2048];
	if (m_released)
		return;
	m_released = true;
	if (m_ctrlTask != 0)
		Send (m_ctrlTask, new SftpClientStop (m_sessionId, result));
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

void FtpClientWorker::printtrailer (bool force)
{
	if (!(g_debug || force))
		return;

	time_t now = time (0);
	struct tm lt = *localtime (&now);

	fprintf (stdout, "%04d-%02d-%02d %02d:%02d:%02d S%06ld %s: ", lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
		lt.tm_hour, lt.tm_min, lt.tm_sec, m_sessionId % (1000 * 1000),
		((this == NULL) || (m_ftpConnectionInfo == NULL) || (m_ftpConnectionInfo->hostname == NULL)) ?
			"" : m_ftpConnectionInfo->hostname);
}

void FtpClientWorker::Execute (FtpRequest* req, void* addInfo)
{
	char* reqtxt = FtpRequestInterface::GetFtpRequestText (req, '\t', '\n');
	FtpCallback* scenario;

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
		Send (m_ctrlTask, new SftpJobInfo (clientId (), m_jobCount, m_sessionId, reqtxt));
		free (reqtxt);
	}
	else
		Send (m_ctrlTask,
			new SftpJobInfo (clientId (), m_jobCount, m_sessionId, (char*) "CANNOT GENERATE FTP REQUEST DESCRIPTION"));

	Continue (scenario);
}

void FtpClientWorker::Continue (FtpCallback* scenario)
{
	if (scenario == 0)
	{
		Stop ();
//		Release (0, 0, 0, 0);
		return;
	}

	m_released = false;
	if (IsAlive () && (strcmp (m_ftpConnectionInfo->hostname, (char*) m_name.c_str ()) == 0))
	{
		RegisterFtpScenario (scenario, this);
		++m_connContinued;
		m_cwdContext = CWD_HOME_DIR;
		ChangeWorkingDirectory (m_homeDir.c_str ());
	}
	else
	{
		Stop ();
//		Release (0, 0, 0, 0);
		RegisterFtpScenario (scenario, this);
		m_name = m_ftpConnectionInfo->hostname;
		Start ();
	}
}

void FtpClientWorker::Common_NotImplementedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	cf_sc_printf (SC_SFTP, SC_ERR, "--- not implemented, context = %d, status = %d, state = %d, event = %ld", context,
		status, state, (long) args [0]);
}

void FtpClientWorker::Common_CtrlBusyTimerExpiredEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "control connection busy timer expired", 0 };

	ftp->Stop ();
	Release (FtpCtrlConnBusyTimerExpired, msgs, 0, 0);
}

void FtpClientWorker::Common_CtrlIdleTimerExpiredEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "control connection idle timer expired", 0 };

	ftp->Stop ();
	Release (FtpCtrlConnIdleTimerExpired, msgs, 0, 0);
}

void FtpClientWorker::Common_DataTimerExpiredEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "data connection timer expired", 0 };

	ftp->Stop ();
	Release (FtpDataConnTimerExpired, msgs, 0, 0);
}

void FtpClientWorker::Common_ListenTimerExpiredEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "listening connection timer expired", 0 };

	ftp->Stop ();
	Release (FtpListenConnTimerExpired, msgs, 0, 0);
}

void FtpClientWorker::Common_RequestEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	if (conversation ())
	{
		if (detailed ())
			cf_sc_printf (SC_SFTP, SC_ERR, "S%06d: <-- %s", m_sessionId % (1000 * 1000),
				((state == PassPrepared) ? "pass ******\r\n" : (char*) args [0]));
		else
			cf_sc_printf (SC_SFTP, SC_APL, "S%06d: <-- %s", m_sessionId % (1000 * 1000),
				((state == PassPrepared) ? "pass ******\r\n" : (char*) args [0]));
	}
	if (m_ctrlTask != 0)
		Send (m_ctrlTask, new SftpClientRequest (m_sessionId, (char*) args [0]));
}

void FtpClientWorker::Common_ReplyEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	if (conversation ())
	{
		if (detailed ())
			cf_sc_printf (SC_SFTP, SC_ERR, "S%06d: --> %s", m_sessionId % (1000 * 1000), (char*) args [0]);
		else
			cf_sc_printf (SC_SFTP, SC_APL, "S%06d: --> %s", m_sessionId % (1000 * 1000), (char*) args [0]);
	}
	if (m_ctrlTask != 0)
		Send (m_ctrlTask, new SftpClientReply (m_sessionId, (char*) args [0]));
}

//
//	FTP CALLBACKS COMMON TO ALL SCENARIOS
//

void FtpClientWorker::Common_StartupEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
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

	char buff [64];

	if (sockaddr4 != 0)
	{
		++m_connInited;
		cf_sc_printf (SC_SFTP, SC_APL, "--- starting A: (%s)", inet_ntop (AF_INET, &sockaddr4->sin_addr, buff, 64));

		sockaddr_in addr;
		memset (&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons (IPPORT_FTP);
		addr.sin_addr = sockaddr4->sin_addr;
		ftp->Connect (m_ftpConnectionInfo->hostname, (sockaddr*) &addr, sizeof(sockaddr_in));
	}
	else if (sockaddr6 != 0)
	{
		++m_connInited;
		cf_sc_printf (SC_SFTP, SC_APL, "--- starting AAAA: (%s)",
			inet_ntop (AF_INET6, &sockaddr6->sin6_addr, buff, 64));

		sockaddr_in6 addr;
		memset (&addr, 0, sizeof(struct sockaddr_in6));
		addr.sin6_family = AF_INET;
		addr.sin6_port = htons (IPPORT_FTP);
		addr.sin6_addr = sockaddr6->sin6_addr;
		ftp->Connect (m_ftpConnectionInfo->hostname, (sockaddr*) &addr, sizeof(sockaddr_in6));
	}
	else if ((hostAddr = inet_addr (m_ftpConnectionInfo->hostname)) != (in_addr_t) -1)
	{
		++m_connInited;
		cf_sc_printf (SC_SFTP, SC_APL, "--- starting AA: (%s)", m_ftpConnectionInfo->hostname);

		sockaddr_in addr;
		memset (&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons (IPPORT_FTP);
		addr.sin_addr.s_addr = hostAddr;
		ftp->Connect (m_ftpConnectionInfo->hostname, (sockaddr*) &addr, sizeof(sockaddr_in));
	}
	else
	{
		++m_connUnresolved;
		const char* const args [] =
		{ "unresolved hostname: ", m_ftpConnectionInfo->hostname, 0 };
		Release (FtpUnresolvedHostname, args, 0, 0);
	}
//	else	ftp->Connect(m_ftpConnectionInfo->hostname);
}

void FtpClientWorker::Common_ConnectedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	size_t pos;

	++m_connEstablished;
	cf_sc_printf (SC_SFTP, SC_APL, "--- connected: %s", m_ftpConnectionInfo->hostname);
	string authentication = m_ftpConnectionInfo->authentication;
	if ((pos = authentication.find ("ssl-version=", 0)) == string::npos)
		ftp->Login (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
	else
	{
		size_t spos = authentication.find (';', pos);
		if (spos == string::npos)
			ftp->Login (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
		else
		{
			string sslVersion = authentication.substr (pos + 12, spos);
			if (sslVersion == "none")
				ftp->Login (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
			else
			{
				if ((pos = authentication.find ("data-protection=", 0)) == string::npos)
					m_dataProtection = true;
				else
				{
					if ((spos = authentication.find (';', pos)) == string::npos)
						m_dataProtection = false;
					else
					{
						string dataProtection = authentication.substr (pos + 16, spos);
						if (dataProtection == "protect")
							m_dataProtection = true;
						else
							m_dataProtection = false;
					}
				}
				ftp->AuthMechanism (sslVersion);
				m_ctrlProtection = true;
			}
		}
	}
}

void FtpClientWorker::Common_AuthFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	++m_userAuthenticated;
	cf_sc_printf (SC_SFTP, SC_APL, "--- authentication in progress: %s", m_ftpConnectionInfo->hostname);
	ftp->Login (m_ftpConnectionInfo->user, m_ftpConnectionInfo->password);
}

void FtpClientWorker::Common_ClientAuthenticatedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	++m_userAuthenticated;
	cf_sc_printf (SC_SFTP, SC_APL, "--- authenticated: %s", m_ftpConnectionInfo->hostname);
	if (m_ctrlProtection && m_dataProtection)
		ftp->ProtectionBufferSize ("0");
	else
		ftp->System ();
}

void FtpClientWorker::Common_PbszFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	cf_sc_printf (SC_SFTP, SC_APL, "--- PBSZ (data protection buffer size - 0) OK");
	ftp->ProtectionLevel ("P");
}

void FtpClientWorker::Common_ProtFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	cf_sc_printf (SC_SFTP, SC_APL, "--- PROT (data protection level - private) OK");
	ftp->System ();
}

void FtpClientWorker::Common_SystFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	char* line = (char*) args [0];

	if (strlen (line) > 4)
		line += 4;
	char* ptr;
	if ((ptr = strpbrk (line, " \t\r\n")) != NULL)
		*ptr = 0;
	m_system = line;
	if (strncasecmp (line, "unix", 4) == 0)
		m_sysType = UnixSystem;
	else if (strncasecmp (line, "linux", 5) == 0)
		m_sysType = LinuxSystem;
	else if (strncasecmp (line, "windows", 7) == 0)
		m_sysType = WindowsSystem;
	else
		m_sysType = UnixSystem;
	ftp->PrintWorkingDirectory ();
//	ftp->Feature();
}

void FtpClientWorker::Common_FeatFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	char* line = (char*) args [0];

	cf_sc_printf (SC_SFTP, SC_APL, "--- FEATURE: %s", line);
	ftp->PrintWorkingDirectory ();
}

void FtpClientWorker::Common_QuitFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "quit", 0 };

	ftp->Stop ();
	Release (0, msgs, 0, 0);
}

void FtpClientWorker::Common_DisposedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	++m_connDisposed;
	cf_sc_printf (SC_SFTP, SC_APL, "--- disposed, context = %d, status = %d, state = %d%s", context, status, state,
		((m_released) ? ", allready released" : ""));
//	Release (0, 0, 0, 0);
}

void FtpClientWorker::ExtractHomeDir (char* line)
{
	m_homeDir = ".";
	if (strlen (line) <= 4)
		return;

	line += 4;
	char* ptr;
	if ((ptr = strchr (line, '"')) != 0)
	{
		line = ptr + 1;
		if ((ptr = strchr (line, '"')) != 0)
		{
			*ptr = 0;
			m_homeDir = line;
			*ptr = '"';
		}
	}
}

//
//	CHECK CONNECTIVITY SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::CheckConnectivity_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);

	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (".");
}

void FtpClientWorker::CheckConnectivity_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "check connection finished", 0 };

	Release (0, msgs, 0, 0);
}

void FtpClientWorker::CheckConnectivity_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	GenErrorReport (args);

	const char* const msgarr [] =
	{ (char*) args [0], 0 };
	Release (FtpCheckConnectivityFailed, msgarr, (char*) args [1], (int) (long) args [2]);
}

//
//	MAKE DIRECTORY PATH SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::Mkdir_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);

	m_cwdContext = CWD_REMOTE_DIR;
	Mkdir_CwdFinishedEventHandler (ftp, context, status, state, args);
}

void FtpClientWorker::Mkdir_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_makeDir->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		m_mkdEnd = m_mkdPtr = m_makeDir->remoteDirName;
		while (*m_mkdEnd == '/')
			*m_mkdEnd++ = 0;
		if (m_mkdEnd != m_mkdPtr)
		{
			m_cwdContext = CWD_ROOT;
			ftp->ChangeWorkingDirectory ("/");
		}
		else
		{
			while ((*m_mkdEnd != 0) && (*m_mkdEnd != '/'))
				m_mkdEnd++;
			while (*m_mkdEnd == '/')
				*m_mkdEnd++ = 0;
			m_cwdContext = CWD_MKDIR;
			ftp->ChangeWorkingDirectory (m_mkdPtr);
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
		while ((*m_mkdEnd != 0) && (*m_mkdEnd != '/'))
			m_mkdEnd++;
		while (*m_mkdEnd == '/')
			*m_mkdEnd++ = 0;
		m_cwdContext = CWD_MKDIR;
		ftp->ChangeWorkingDirectory (m_mkdPtr);
		break;
	default:
	{
		const char* const msgs [] =
		{ "mkdir failed, illegal context", 0 };

		Release (FtpIllegalCwdContext, msgs, 0, 0);
		return; // don't delete this statement
	}
		break;
	}
}

void FtpClientWorker::Mkdir_MkdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	m_cwdContext = CWD_MKDIR;
	ftp->ChangeWorkingDirectory (m_mkdPtr);
}

void FtpClientWorker::Mkdir_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	if ((state == CwdFinished) && (m_cwdContext == CWD_MKDIR))
	{
		ftp->MakeDirectory (m_mkdPtr);
		return;
	}
	if (state < ClientAuthenticated)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (FtpMkdirFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	GET DIRECTORY LISTING SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::Getdir_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);

	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_getDir->remoteDirName);
}

void FtpClientWorker::Getdir_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_getDir->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		m_listReply = "";
		if (m_ftpRequest->flags & FTP_NAME_LISTING)
			ftp->NameList ("");
		else
			ftp->List ("");
		break;
	default:
		break;
	}
}

void FtpClientWorker::Getdir_ListPreparedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
}

void FtpClientWorker::Getdir_ListProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (context != data)
		return;
	char* buff = (char*) args [0];
	cf_sc_printf (SC_SFTP, SC_APL, buff);
}

void FtpClientWorker::Getdir_ListFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "getdir(detailed list) finished", 0 };

	if (context != ctrl)
		return;
	Release (0, msgs, 0, 0);
}

void FtpClientWorker::Getdir_NlstPreparedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
}

void FtpClientWorker::Getdir_NlstProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
}

void FtpClientWorker::Getdir_NlstFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "getdir(simple list) finished", 0 };

	if (context != ctrl)
		return;
	Release (0, msgs, 0, 0);
}

void FtpClientWorker::Getdir_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (FtpGetdirFailed, msg, (char*) args [1], (int) (long) args [2]);
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
/*   its callback function is called accordingly. Information   */
/*   about this context is very helpful in all situations where */
/*   some event should appear in different situations           */
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
/*   approprietly before use                                    */
/****************************************************************/

/****************************************************************/
/* Function:    FtpClientWorker::Store_PwdFinishedEventHandler()*/
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

void FtpClientWorker::Store_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);

	string remoteDir;
	if (*m_storeFile->remoteDirName == '/')
		remoteDir = m_storeFile->remoteDirName;
	else
	{
		remoteDir = m_homeDir;
		remoteDir += '/';
		remoteDir += m_storeFile->remoteDirName;
	}
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (remoteDir);
}

/****************************************************************/
/* Function:    FtpClientWorker::Store_CwdFinishedEventHandler()*/
/* In:          common FTP parameters                           */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: CWD handler in 'store file' operation. It is    */
/****************************************************************/

void FtpClientWorker::Store_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
	{
		string remoteDir;
		if (*m_storeFile->remoteDirName == '/')
			remoteDir = m_storeFile->remoteDirName;
		else
		{
			remoteDir = m_homeDir;
			remoteDir += '/';
			remoteDir += m_storeFile->remoteDirName;
		}
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (remoteDir);
	}
		break;
	case CWD_REMOTE_DIR:
		if (m_ftpRequest->flags & FTP_PROTECT_EXISTING_FILE)
		{
			m_listReply = "";
			ftp->NameList (m_storeFile->remoteFileName);
		}
		else
			ftp->Delete (m_storeFile->remoteFileName);
		break;
	case CWD_WORKING_DIR:
		Store_InvokeRepresentationType (ftp, context, status, state, args);
		break;
	default:
		break;
	}
}

void FtpClientWorker::Store_NlstProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (context != data)
		return;
	m_listReply += (char*) args [0];
}

void FtpClientWorker::Store_NlstFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	char* line = (char*) m_listReply.c_str ();

	if (strstr (line, m_storeFile->remoteFileName) == 0)
		((FtpClientWorker*) ftp)->Store_DeleFinishedEventHandler (ftp, context, status, state, args);
	else
	{
		const char* const args [] =
		{ "store failed, remote file '", m_storeFile->remoteFileName, "' not listed", 0 };
		Release (FtpStoreNotListed, args, 0, 0);
		return; // don't delete this statement
	}
}

void FtpClientWorker::Store_DeleFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		char* workingDirName = m_storeFile->workingDirName;
		if ((workingDirName == NULL) || (*workingDirName == 0))
		{
			const char* const msgs [] =
			{ "store: cannot check unnamed working directory", 0 };

			Release (FtpStoreWorkDirEmpty, msgs, 0, 0);
			return;
		}
		string workingDir;
		if (*workingDirName == '/')
			workingDir = workingDirName;
		else
		{
			workingDir = m_homeDir;
			workingDir += "/";
			workingDir += workingDirName;
		}
		m_cwdContext = CWD_WORKING_DIR;
		ftp->ChangeWorkingDirectory (workingDir);
	}
	else
		Store_InvokeRepresentationType (ftp, context, status, state, args);
}

void FtpClientWorker::Store_InvokeRepresentationType (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_TEXT_FILE_TRANSFER)
		ftp->RepresentationType (ascii, undefinedFormat, -1);
	else
		ftp->RepresentationType (image, undefinedFormat, -1);
}

void FtpClientWorker::Store_TypeFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	string localPathName = m_storeFile->localDirName;
	localPathName += '/';
	localPathName += m_storeFile->localFileName;
	ftp->Store (localPathName, m_storeFile->remoteFileName);
}

void FtpClientWorker::Store_StorPreparedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	ftpcln->m_start = ftp->ctx ()->realTime ();
	ftpcln->m_size = 0;
}

void FtpClientWorker::Store_StorProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	long n = (long) args [1];
	ftpcln->m_size += n;
}

void FtpClientWorker::Store_StorFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	ftpcln->m_stop = ftp->ctx ()->realTime ();
	if (m_ftpRequest->flags & FTP_CHECK_FILE_SIZE)
	{
		m_listReply = "";
		ftp->List (m_storeFile->remoteFileName);
	}
	else
		Store_RenameWorkingFile (ftp, context, status, state, args);
}

void FtpClientWorker::Store_ListProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (context != data)
		return;
	m_listReply += (char*) args [0];
}

void FtpClientWorker::Store_ListFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	DirEntryInfo ent;

	if (ReadListLine ((char*) m_listReply.c_str (), ent) && (ent.size == (u_long) ftpcln->m_size))
		Store_RenameWorkingFile (ftp, context, status, state, args);
	else
	{
		const char* const args [] =
		{ "store failed, illegal list reply: ", m_listReply.c_str (), 0 };
		Release (FtpStoreIllList, args, 0, 0);
	}
}

void FtpClientWorker::Store_RenameWorkingFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		string dstFileName = m_homeDir;
		dstFileName += '/';
		dstFileName += m_storeFile->remoteDirName;
		dstFileName += '/';
		dstFileName += m_storeFile->remoteFileName;
		ftp->Rename (m_storeFile->remoteFileName, dstFileName);
	}
	else
		Store_RemoveSourceFile (ftp, context, status, state, args);
}

void FtpClientWorker::Store_RntoFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	Store_RemoveSourceFile (ftp, context, status, state, args);
}

void FtpClientWorker::Store_RemoveSourceFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
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
			{ "store failed, cannot remove local file '", localFile.c_str (), "'", 0 };
			Release (FtpStoreLocalUnlink, args, "unlink", errno);
			return; // don't delete this statement
		}
		if (conversation ())
		{
			if (detailed ())
				cf_sc_printf (SC_SFTP, SC_ERR, "S%06d: --> source file '%s' removed", m_sessionId % (1000 * 1000),
					m_storeFile->localFileName);
			else
				cf_sc_printf (SC_SFTP, SC_APL, "S%06d: --> source file '%s' removed", m_sessionId % (1000 * 1000),
					m_storeFile->localFileName);
		}
	}
	const char* const msgs [] =
	{ "store finished", 0 };

	Release (0, msgs, 0, 0);
	return; // don't delete this statement
}

void FtpClientWorker::Store_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	switch (state)
	{
	case DeleFinished:
	case NlstFinished:
		//	'DELE remoteFile' or 'NLST remoteFile' failed: it's OK - continue with next request
		((FtpClientWorker*) ftp)->Store_DeleFinishedEventHandler (ftp, context, status, state, args);
		return;
		break;
	default:
		if (state < ClientAuthenticated)
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
	Release (FtpStoreFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	RETRIEVE SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::Retrieve_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);

	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_retrieveFile->remoteDirName);
}

void FtpClientWorker::Retrieve_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_retrieveFile->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		if (m_ftpRequest->flags & FTP_PROTECT_EXISTING_FILE)
		{
			string localFile = m_retrieveFile->localDirName;
			localFile += '/';
			localFile += m_retrieveFile->localFileName;
			if (access (localFile.c_str (), F_OK) == 0)
			{
				const char* const args [] =
				{ "retrieve failed: cannot access local file '", m_retrieveFile->localFileName, "'", 0 };
				Release (FtpRetrLocalAccess, args, "access", errno);
				return;
			}
		}
		if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
		{
			char* workingDir = m_retrieveFile->workingDirName;
			if ((workingDir == NULL) || (*workingDir == 0))
			{
				const char* const msgs [] =
				{ "retrieve: empty working directory name", 0 };

				Release (FtpRetrWorkDirEmpty, msgs, 0, 0);
				return;
			}
			string workingPath = m_retrieveFile->localDirName;
			workingPath += '/';
			workingPath += m_retrieveFile->workingDirName;
			workingDir = (char*) workingPath.c_str ();
			if (access (workingDir, F_OK) != 0)
			{
				umask (0);
				if (mkdir (workingDir, 0777) != 0)
				{
					const char* const args [] =
					{ "retrieve: mkdir '", workingDir, "' failed", 0 };
					Release (FtpRetrMkWorkDir, args, "mkdir", errno);
					return;
				}
			}
		}
		Retrieve_InvokeRepresentationType (ftp, context, status, state, args);
		break;
	default:
		break;
	}
}

void FtpClientWorker::Retrieve_InvokeRepresentationType (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_TEXT_FILE_TRANSFER)
		ftp->RepresentationType (ascii, undefinedFormat, -1);
	else
		ftp->RepresentationType (image, undefinedFormat, -1);
}

void FtpClientWorker::Retrieve_TypeFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	string localPathName = m_retrieveFile->localDirName;
	localPathName += '/';
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		localPathName += m_retrieveFile->workingDirName;
		localPathName += '/';
	}
	localPathName += m_retrieveFile->localFileName;
	ftp->Retrieve (localPathName, m_retrieveFile->remoteFileName);
}

void FtpClientWorker::Retrieve_RetrPreparedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	ftpcln->m_start = ftp->ctx ()->realTime ();
}

void FtpClientWorker::Retrieve_RetrProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	long n = (long) args [1];
	ftpcln->m_size += n;
}

void FtpClientWorker::Retrieve_RetrFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	ftpcln->m_stop = ftp->ctx ()->realTime ();

	if (m_ftpRequest->flags & FTP_CHECK_FILE_SIZE)
	{
		m_listReply = "";
		ftp->List (m_retrieveFile->remoteFileName);
	}
	else
		Retrieve_MoveRetrievedFile (ftp, context, status, state, args);
}

void FtpClientWorker::Retrieve_ListProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (context != data)
		return;
	m_listReply += (char*) args [0];
	cf_sc_printf (SC_SFTP, SC_APL, (char*) m_listReply.c_str ());
}

void FtpClientWorker::Retrieve_ListFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	DirEntryInfo ent;

	if (ReadListLine ((char*) m_listReply.c_str (), ent))
		Retrieve_MoveRetrievedFile (ftp, context, status, state, args);
	else
	{
		const char* const msgs [] =
		{ "retrieve: illegal list reply", 0 };

		Release (FtpRetrIllList, msgs, 0, 0);
	}
}

void FtpClientWorker::Retrieve_MoveRetrievedFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
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
			cf_sc_printf (SC_SFTP, SC_ERR, "--- rename (%s, %s) failed, errno = %d", workingPath.c_str (),
				localPath.c_str (), errno);
			const char* const args [] =
			{ "rename (", workingPath.c_str (), ", ", localPath.c_str (), ") failed", 0 };
			Release (FtpRetrRename, args, "rename", errno);
			return;
		}
	}
	if (m_ftpRequest->flags & FTP_REMOVE_SOURCE_FILE)
	{
		ftp->Delete (m_retrieveFile->remoteFileName);
	}
	else
	{
		const char* const msgs [] =
		{ "retrieve finished", 0 };

		Release (0, msgs, 0, 0);
		return; // don't delete this statement
	}
}

void FtpClientWorker::Retrieve_DeleFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "retrieve finished", 0 };

	Release (0, msgs, 0, 0);
	return; // don't delete this statement
}

void FtpClientWorker::Retrieve_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	if (state < ClientAuthenticated)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (FtpRetrFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	STORE ALL SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::StoreAll_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);

	string remoteDir;
	if (*m_storeAllFiles->remoteDirName == '/')
		remoteDir = m_storeAllFiles->remoteDirName;
	else
	{
		remoteDir = m_homeDir;
		remoteDir += '/';
		remoteDir += m_storeAllFiles->remoteDirName;
	}
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (remoteDir);
}

void FtpClientWorker::StoreAll_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
	{
		string remoteDir;
		if (*m_storeAllFiles->remoteDirName == '/')
			remoteDir = m_storeAllFiles->remoteDirName;
		else
		{
			remoteDir = m_homeDir;
			remoteDir += '/';
			remoteDir += m_storeAllFiles->remoteDirName;
		}
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (remoteDir);
	}
		m_localDir = NULL;
		break;
	case CWD_REMOTE_DIR:
		if (m_localDir == NULL)
		{
			if ((m_localDir = opendir (m_storeAllFiles->localDirName)) == NULL)
			{
				const char* const args [] =
				{ "storeall: opendir (", m_storeAllFiles->localDirName, ") failed", 0 };
				Release (FtpStoreAllLocDirOpen, args, "opendir", errno);
				return;
			}
			StoreAll_StoreNextFile (ftp, context, status, state, args);
		}
		else
			StoreAll_RemoveSourceFile (ftp, context, status, state, args);
		break;
	case CWD_WORKING_DIR:
		StoreAll_InvokeRepresentationType (ftp, context, status, state, args);
		break;
	default:
		break;
	}
}

void FtpClientWorker::StoreAll_StoreNextFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	ExceptionFileList* exceptions = m_storeAllFiles->exceptions;

	while (true)
	{
		if (readdir_r (m_localDir, &m_localDirEntry, &m_localDirEntryPtr) != 0)
		{
			const char* const args [] =
			{ "storeall: readdir (", m_storeAllFiles->localDirName, ") failed", 0 };
			Release (FtpStoreAllLocDirRead, args, "readdir_r", errno);
			return;
		}
		if (m_localDirEntryPtr == NULL)
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
			Release (FtpStoreAllStat, args, "stat", errno);
			return;
		}
		if (sbuff.st_mode & __S_IFREG)
			break;
	}
	if (m_ftpRequest->flags & FTP_PROTECT_EXISTING_FILE)
	{
		m_listReply = "";
		ftp->NameList (m_localDirEntry.d_name);
	}
	else
		ftp->Delete (m_localDirEntry.d_name);
}

void FtpClientWorker::StoreAll_NlstProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (context != data)
		return;
	m_listReply += (char*) args [0];
}

void FtpClientWorker::StoreAll_NlstFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	char* line = (char*) m_listReply.c_str ();

	if (strstr (line, m_localDirEntry.d_name) == 0)
		((FtpClientWorker*) ftp)->StoreAll_DeleFinishedEventHandler (ftp, context, status, state, args);
	else
	{
		const char* const msgs [] =
		{ "storeall: entry exist ", m_localDirEntry.d_name, 0 };

		Release (FtpStoreAllListReply, msgs, 0, 0);
		return; // don't delete this statement
	}
}

void FtpClientWorker::StoreAll_InvokeRepresentationType (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_TEXT_FILE_TRANSFER)
		ftp->RepresentationType (ascii, undefinedFormat, -1);
	else
		ftp->RepresentationType (image, undefinedFormat, -1);
}

void FtpClientWorker::StoreAll_TypeFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	string localPathName = m_storeAllFiles->localDirName;
	localPathName += '/';
	localPathName += m_localDirEntry.d_name;
	ftp->Store (localPathName, m_localDirEntry.d_name);
}

void FtpClientWorker::StoreAll_StorPreparedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	ftpcln->m_start = ftp->ctx ()->realTime ();
	ftpcln->m_size = 0;
}

void FtpClientWorker::StoreAll_StorProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	long n = (long) args [1];
	ftpcln->m_size += n;
}

void FtpClientWorker::StoreAll_StorFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	ftpcln->m_stop = ftp->ctx ()->realTime ();
	if (m_ftpRequest->flags & FTP_CHECK_FILE_SIZE)
	{
		m_listReply = "";
		ftp->List (m_localDirEntry.d_name);
	}
	else
		StoreAll_RenameWorkingFile (ftp, context, status, state, args);
}

void FtpClientWorker::StoreAll_ListProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (context != data)
		return;
	m_listReply += (char*) args [0];
	cf_sc_printf (SC_SFTP, SC_APL, "list reply = %s", (char*) m_listReply.c_str ());
}

void FtpClientWorker::StoreAll_ListFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	DirEntryInfo ent;

	if (ReadListLine ((char*) m_listReply.c_str (), ent) && (ent.size == (u_long) ftpcln->m_size))
		StoreAll_RenameWorkingFile (ftp, context, status, state, args);
	else
	{
		const char* const msgs [] =
		{ "storeall: entry not found ", (char*) m_listReply.c_str (), 0 };

		Release (FtpStoreAllListReply, msgs, 0, 0);
	}
}

void FtpClientWorker::StoreAll_RenameWorkingFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		string dstFileName = m_homeDir;
		dstFileName += '/';
		dstFileName += m_storeAllFiles->remoteDirName;
		dstFileName += '/';
		dstFileName += m_localDirEntry.d_name;
		ftp->Rename (m_localDirEntry.d_name, dstFileName);
	}
	else
		StoreAll_RemoveSourceFile (ftp, context, status, state, args);
}

void FtpClientWorker::StoreAll_DeleFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		char* workingDirName = m_storeAllFiles->workingDirName;
		if ((workingDirName == NULL) || (*workingDirName == 0))
		{
			const char* const msgs [] =
			{ "store: cannot check unnamed working directory", 0 };

			Release (FtpStoreAllWorkDirEmpty, msgs, 0, 0);
			return;
		}
		string workingDir;
		if (*workingDirName == '/')
			workingDir = workingDirName;
		else
		{
			workingDir = m_homeDir;
			workingDir += "/";
			workingDir += workingDirName;
		}
		m_cwdContext = CWD_WORKING_DIR;
		m_listReply = "";
		ftp->ChangeWorkingDirectory (workingDir);
	}
	else
		StoreAll_InvokeRepresentationType (ftp, context, status, state, args);
}

void FtpClientWorker::StoreAll_RntoFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	string remoteDir;
	if (*m_storeAllFiles->remoteDirName == '/')
		remoteDir = m_storeAllFiles->remoteDirName;
	else
	{
		remoteDir = m_homeDir;
		remoteDir += '/';
		remoteDir += m_storeAllFiles->remoteDirName;
	}
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (remoteDir);
}

void FtpClientWorker::StoreAll_RemoveSourceFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
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
			Release (FtpStoreAllSrcRemove, args, "unlink", errno);
			return;
		}
		if (conversation ())
		{
			if (detailed ())
				cf_sc_printf (SC_SFTP, SC_ERR, "S%06d: --> source file '%s' removed", m_sessionId % (1000 * 1000),
					m_localDirEntry.d_name);
			else
				cf_sc_printf (SC_SFTP, SC_APL, "S%06d: --> source file '%s' removed", m_sessionId % (1000 * 1000),
					m_localDirEntry.d_name);
		}
	}
	StoreAll_StoreNextFile (ftp, context, status, state, args);
}

void FtpClientWorker::StoreAll_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	switch (state)
	{
	case DeleFinished:
	case NlstFinished:
		//	'DELE remoteFile' or 'NLST remoteFile' failed: it's OK - continue with next request
		((FtpClientWorker*) ftp)->StoreAll_DeleFinishedEventHandler (ftp, context, status, state, args);
		return;
		break;
	default:
		if (state < ClientAuthenticated)
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
	Release (FtpStoreAllFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	RETRIEVE SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::RetrieveAll_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);

	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_retrieveAllFiles->remoteDirName);
}

void FtpClientWorker::RetrieveAll_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_retrieveAllFiles->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		m_listReply = "";
		ftp->NameList ("");
		break;
	default:
	{
		const char* const msgs [] =
		{ "retrieveall: illegal context", 0 };

		Release (FtpRetrAllCwdContext, msgs, 0, 0);
		return; // don't delete this statement
	}
		break;
	}
}

void FtpClientWorker::RetrieveAll_NlstProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	char* buff = (char*) args [0];
	int n = (int) (long) args [1];

	if (n == 0)
	{
		const char* const msgs [] =
		{ "empty name list", 0 };

		Release (0, msgs, 0, 0);
		return;
	}

	int freeSpace = m_listEnd - m_listPtr;
	if (freeSpace < n)
	{
		int usedSpace = m_listPtr - m_listBuffer;
		int needSpace = usedSpace + n;
		needSpace >>= 12;
		needSpace++;
		needSpace <<= 12;
		char* buffer = (char*) malloc (needSpace);
		if (buffer == 0)
		{
			const char* const msgs [] =
			{ "malloc failed", 0 };

			Release (FtpRetrAllMallocFailed, msgs, 0, 0);
			return;
		}
		if (m_listBuffer != 0)
		{
			memcpy (buffer, m_listBuffer, usedSpace);
			free (m_listBuffer);
		}
		m_listBuffer = buffer;
		m_listPtr = buffer + usedSpace;
		m_listEnd = buffer + needSpace;
	}
	memcpy (m_listPtr, buff, n);
	m_listPtr += n;
}

void FtpClientWorker::RetrieveAll_NlstFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_PROTECT_EXISTING_FILE)
	{
		string localFile = m_retrieveAllFiles->localDirName;
		localFile += '/';
		localFile += m_currentRemoteFile;
		if (access (localFile.c_str (), F_OK) == 0)
		{
			const char* const args [] =
			{ "retrieveall: access (", (char*) localFile.c_str (), ") failed", 0 };
			Release (FtpRetrAllLocalAccess, args, "access", errno);
			return;
		}
	}
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		char* workingDir = m_retrieveAllFiles->workingDirName;
		if ((workingDir == NULL) || (*workingDir == 0))
		{
			const char* const msgs [] =
			{ "retrieveall: empty working dir name", 0 };

			Release (FtpRetrAllWrkDirEmpty, msgs, 0, 0);
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
				Release (FtpRetrAllWrkDirMake, args, "mkdir", errno);
				return;
			}
		}
	}
	m_listRead = m_listBuffer;
	RetrieveAll_InvokeRepresentationType (ftp, context, status, state, args);
}

void FtpClientWorker::RetrieveAll_InvokeRepresentationType (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_TEXT_FILE_TRANSFER)
		ftp->RepresentationType (ascii, undefinedFormat, -1);
	else
		ftp->RepresentationType (image, undefinedFormat, -1);
}

void FtpClientWorker::RetrieveAll_TypeFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	RetrieveAll_RetrieveNextFile (ftp, context, status, state, args);
}

void FtpClientWorker::RetrieveAll_RetrieveNextFile (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExceptionFileList* exceptions = m_retrieveAllFiles->exceptions;

	while (true)
	{
		if ((m_listRead == NULL) || (*m_listRead == 0))
		{
			const char* const msgs [] =
			{ "retrieveall finished", 0 };

			Release (0, msgs, 0, 0);
			return;
		}

		char* ptr = strstr (m_listRead, "\r\n");
		if (ptr != NULL)
			*ptr = 0;

		m_currentRemoteFile = m_listRead;
		if (ptr != NULL)
			m_listRead = ptr + 2;
		else
			m_listRead = NULL;

		ExceptionFileList* exPtr;
		bool exception;
		for (exception = false, exPtr = exceptions; exPtr != 0; exPtr = exPtr->next)
		{
			regex_t reg;
			if (regcomp (&reg, exPtr->fileNameTemplate, REG_EXTENDED) == 0)
			{
				regmatch_t match;
				memset (&match, 0, sizeof(regmatch_t));
				if (regexec (&reg, m_currentRemoteFile, 1, &match, 0) == 0)
					if (exPtr->action == ExceptionDeny)
						exception = true;
				regfree (&reg);
			}
		}

		if (exception)
			continue;

		string localPathName = m_retrieveAllFiles->localDirName;
		localPathName += '/';
		if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
		{
			localPathName += m_retrieveAllFiles->workingDirName;
			localPathName += '/';
		}
		localPathName += m_currentRemoteFile;
		ftp->Retrieve (localPathName, m_currentRemoteFile);
		break;
	}
}

void FtpClientWorker::RetrieveAll_RetrPreparedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	ftpcln->m_start = ftp->ctx ()->realTime ();
}

void FtpClientWorker::RetrieveAll_RetrProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	long n = (long) args [1];
	ftpcln->m_size += n;
}

void FtpClientWorker::RetrieveAll_RetrFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	FtpClientWorker* ftpcln = (FtpClientWorker*) ftp;
	ftpcln->m_stop = ftp->ctx ()->realTime ();

	if (m_ftpRequest->flags & FTP_CHECK_FILE_SIZE)
	{
		m_listReply = "";
		ftp->List (m_currentRemoteFile);
	}
	else
		RetrieveAll_MoveRetrievedFile (ftp, context, status, state, args);
}

void FtpClientWorker::RetrieveAll_ListProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (context != data)
		return;
	m_listReply += (char*) args [0];
}

void FtpClientWorker::RetrieveAll_ListFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	DirEntryInfo ent;

	if (ReadListLine ((char*) m_listReply.c_str (), ent))
		RetrieveAll_MoveRetrievedFile (ftp, context, status, state, args);
	else
	{
		const char* const msgs [] =
		{ "retrieveall: illegal list reply", 0 };

		Release (FtpRetrAllIllListReply, msgs, 0, 0);
	}
}

void FtpClientWorker::RetrieveAll_MoveRetrievedFile (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_USE_WORKING_DIR)
	{
		string workingPath = m_retrieveAllFiles->localDirName;
		workingPath += '/';
		workingPath += m_retrieveAllFiles->workingDirName;
		workingPath += '/';
		workingPath += m_currentRemoteFile;
		string localPath = m_retrieveAllFiles->localDirName;
		localPath += '/';
		localPath += m_currentRemoteFile;
		if (rename (workingPath.c_str (), localPath.c_str ()) != 0)
		{
			const char* const args [] =
			{ "retrieveall: rename (", (char*) workingPath.c_str (), ", ", (char*) localPath.c_str (), ") failed", 0 };
			Release (FtpRetrAllLocalRename, args, "rename", errno);
			return;
		}
	}
	if (m_ftpRequest->flags & FTP_REMOVE_SOURCE_FILE)
		ftp->Delete (m_currentRemoteFile);
	else
		RetrieveAll_RetrieveNextFile (ftp, context, status, state, args);
}

void FtpClientWorker::RetrieveAll_DeleFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	RetrieveAll_RetrieveNextFile (ftp, context, status, state, args);
}

void FtpClientWorker::RetrieveAll_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (state == RetrFinished)
	{
		RetrieveAll_RetrieveNextFile (ftp, context, status, state, args);
		return;
	}
	if (state < ClientAuthenticated)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (FtpRetrAllFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	DELETE SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::Delete_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);

	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_deleteFile->remoteDirName);
}

void FtpClientWorker::Delete_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_deleteFile->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		ftp->Delete (m_deleteFile->remoteFileName);
		break;
	default:
	{
		const char* const msgs [] =
		{ "delete: illegal context", 0 };

		Release (FtpDeleteContext, msgs, 0, 0);
		return; // don't delete this statement
	}
		break;
	}
}

void FtpClientWorker::Delete_DeleFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	const char* const msgs [] =
	{ "delete finished", 0 };

	Release (0, msgs, 0, 0);
	return; // don't delete this statement
}

void FtpClientWorker::Delete_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	if (state < ClientAuthenticated)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (FtpDeleteFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	DELETE ALL SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::DeleteAll_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);

	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_deleteAllFiles->remoteDirName);
}

void FtpClientWorker::DeleteAll_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_deleteAllFiles->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		m_listReply = "";
		ftp->NameList ("");
		break;
	default:
	{
		const char* const msgs [] =
		{ "deleteall: illegal context", 0 };

		Release (FtpDeleteAllContext, msgs, 0, 0);
		return; // don't delete this statement
	}
		break;
	}
}

void FtpClientWorker::DeleteAll_NlstProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	char* buff = (char*) args [0];
	int n = (int) (long) args [1];

	if (n == 0)
	{
		const char* const msgs [] =
		{ "empty name list", 0 };

		Release (0, msgs, 0, 0);
		return;
	}

	int freeSpace = m_listEnd - m_listPtr;
	if (freeSpace < n)
	{
		int usedSpace = m_listPtr - m_listBuffer;
		int needSpace = usedSpace + n;
		needSpace >>= 12;
		needSpace++;
		needSpace <<= 12;
		char* buffer = (char*) malloc (needSpace);
		if (buffer == 0)
		{
			const char* const msgs [] =
			{ "malloc failed", 0 };

			Release (FtpDeleteAllMallocFailed, msgs, 0, 0);
			return;
		}
		if (m_listBuffer != 0)
		{
			memcpy (buffer, m_listBuffer, usedSpace);
			free (m_listBuffer);
		}
		m_listBuffer = buffer;
		m_listPtr = buffer + usedSpace;
		m_listEnd = buffer + needSpace;

	}
	memcpy (m_listPtr, buff, n);
	m_listPtr += n;
}

void FtpClientWorker::DeleteAll_NlstFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	m_listRead = m_listBuffer;
	DeleteAll_DeleteNextFile (ftp, context, status, state, args);
}

void FtpClientWorker::DeleteAll_DeleteNextFile (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	ExceptionFileList* exceptions = m_deleteAllFiles->exceptions;

	while (true)
	{
		if ((m_listRead == NULL) || (*m_listRead == 0))
		{
			const char* const msgs [] =
			{ "deleteall finished", 0 };

			Release (0, msgs, 0, 0);
			return;
		}

		char* ptr = strstr (m_listRead, "\r\n");
		if (ptr != NULL)
			*ptr = 0;

		m_currentRemoteFile = m_listRead;
		if (ptr != NULL)
			m_listRead = ptr + 2;
		else
			m_listRead = NULL;

		ExceptionFileList* exPtr;
		bool exception;
		for (exception = false, exPtr = exceptions; exPtr != 0; exPtr = exPtr->next)
		{
			regex_t reg;
			if (regcomp (&reg, exPtr->fileNameTemplate, REG_EXTENDED) == 0)
			{
				regmatch_t match;
				memset (&match, 0, sizeof(regmatch_t));
				if (regexec (&reg, m_currentRemoteFile, 1, &match, 0) == 0)
					if (exPtr->action == ExceptionDeny)
						exception = true;
				regfree (&reg);
			}
		}

		if (exception)
			continue;

		ftp->Delete (m_currentRemoteFile);
		break;
	}
}

void FtpClientWorker::DeleteAll_DeleFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	DeleteAll_DeleteNextFile (ftp, context, status, state, args);
}

void FtpClientWorker::DeleteAll_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	if (state == DeleFinished)
	{
		DeleteAll_DeleteNextFile (ftp, context, status, state, args);
		return;
	}
	if (state < ClientAuthenticated)
		Stop ();

	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (FtpDeleteAllFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

//
//	MAKE WORKING ENVIRONMENT SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::MakeWorkingEnv_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (state)
	{
	case CwdFinished:
	{
		switch (m_cwdContext)
		{
		case CWD_REMOTE_DIR:
		case CWD_WORKING_DIR:
			//	'CWD working' failed: it's OK - make directory and try again
			ftp->MakeDirectory (m_dirPtr);
			return;
			break;
		default:
			break;
		}
	}
		break;
	default:
		if (state < ClientAuthenticated)
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
	Release (FtpMakeWorkingEnvFailed, msg, (char*) args [1], (int) (long) args [2]);
	return; // don't delete this statement
}

void FtpClientWorker::MakeWorkingEnv_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);
	MakeWorkingEnv_CwdFirstRemoteComponent (ftp);
}

void FtpClientWorker::MakeWorkingEnv_CwdFirstRemoteComponent (FtpClient *ftp)
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

void FtpClientWorker::MakeWorkingEnv_CwdNextRemoteComponent (FtpClient *ftp)
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

void FtpClientWorker::MakeWorkingEnv_CwdLastRemoteComponent (FtpClient *ftp)
{
	m_cwdContext = CWD_HOME_DIR_AGAIN;
	ftp->ChangeWorkingDirectory (m_homeDir);
}

void FtpClientWorker::MakeWorkingEnv_CwdFirstWorkingComponent (FtpClient *ftp)
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

void FtpClientWorker::MakeWorkingEnv_CwdNextWorkingComponent (FtpClient *ftp)
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

void FtpClientWorker::MakeWorkingEnv_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
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

void FtpClientWorker::MakeWorkingEnv_MkdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ftp->ChangeWorkingDirectory (m_dirPtr);
}

void FtpClientWorker::MakeWorkingEnv_Finished (FtpClient *ftp)
{
	const char* const msgs [] =
	{ "make working environment finished", 0 };

	Release (0, msgs, 0, 0);
}

//
//	CLEAN DIRECTORY SCENARIO FTP CALLBACKS AND OTHER UTILITIES
//

void FtpClientWorker::CleanDir_ErrorEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status, FtpState state,
	void* args [])
{
	GenErrorReport (args);
	const char* const msg [] =
	{ (char*) args [0], 0 };
	Release (FtpCleanDirFailed, msg, (char*) args [1], (int) (long) args [2]);
}

void FtpClientWorker::CleanDir_CwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	switch (m_cwdContext)
	{
	case CWD_HOME_DIR:
		m_stackDepth = -1;
		m_cwdContext = CWD_REMOTE_DIR;
		ftp->ChangeWorkingDirectory (m_cleanDir->remoteDirName);
		break;
	case CWD_REMOTE_DIR:
		m_listReply = "";
		ftp->List ("");
		break;
	}
}

void FtpClientWorker::CleanDir_PwdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	ExtractHomeDir ((char*) args [0]);
	m_stackDepth = -1;
	m_cwdContext = CWD_REMOTE_DIR;
	ftp->ChangeWorkingDirectory (m_cleanDir->remoteDirName);
}

void FtpClientWorker::CleanDir_ListPreparedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	m_startIndex = m_listPtr - m_listBuffer;
}

void FtpClientWorker::CleanDir_ListProgressEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	char* buff = (char*) args [0];
	int n = (int) (long) args [1];

	if (n == 0)
	{
		const char* const msgs [] =
		{ "empty name list", 0 };

		Release (0, msgs, 0, 0);
		return;
	}

	int freeSpace = m_listEnd - m_listPtr;
	if (freeSpace < n)
	{
		int usedSpace = m_listPtr - m_listBuffer;
		int needSpace = usedSpace + n;
		needSpace >>= 12;
		needSpace++;
		needSpace <<= 12;
		char* buffer = (char*) malloc (needSpace);
		if (buffer == 0)
		{
			const char* const msgs [] =
			{ "malloc failed", 0 };

			Release (FtpCleanDirMallocFailed, msgs, 0, 0);
			return;
		}
		if (m_listBuffer != 0)
		{
			memcpy (buffer, m_listBuffer, usedSpace);
			free (m_listBuffer);
		}
		m_listBuffer = buffer;
		m_listPtr = buffer + usedSpace;
		m_listEnd = buffer + needSpace;

	}
	memcpy (m_listPtr, buff, n);
	m_listPtr += n;
}

void FtpClientWorker::CleanDir_ListFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	m_listStart = m_listRead = m_listBuffer + m_startIndex;
	CleanDir_DeleteNextComponent (ftp, context, status, state, args);
}

void FtpClientWorker::CleanDir_CdupFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	if (m_ftpRequest->flags & FTP_DESTRUCTIVE_OPERATION)
		ftp->RemoveDirectory (m_listName);
	else
		CleanDir_DeleteNextComponent (ftp, context, status, state, args);
}

void FtpClientWorker::CleanDir_RmdFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	CleanDir_DeleteNextComponent (ftp, context, status, state, args);
}

void FtpClientWorker::CleanDir_DeleFinishedEventHandler (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	CleanDir_DeleteNextComponent (ftp, context, status, state, args);
}

void FtpClientWorker::CleanDir_DeleteNextComponent (FtpClient *ftp, FtpContext context, FtpStatus status,
	FtpState state, void* args [])
{
	while (m_listRead < m_listPtr)
	{
		char* line = m_listRead;
		char* ptr = strstr (m_listRead, "\r\n");
		if (ptr != NULL)
		{
			*ptr = 0;
			m_listRead = ptr + 2;
		}
		else
			m_listRead = m_listPtr;

		DirEntryInfo ent;

		if (!ReadListLine (line, ent))
			continue;

		if (ent.type == DT_DIR)
		{
			if (!(m_ftpRequest->flags & FTP_RECURSIVE_OPERATION))
				continue;
			CleanDir_IncStack ();
			ftp->ChangeWorkingDirectory (ent.name);
		}
		else if (ent.type == DT_REG)
			ftp->Delete (ent.name);
		else
			continue;

		return;
	}
	if (m_listRead >= m_listPtr)
	{
		if (m_stackDepth < 0)
		{
			const char* const msgs [] =
			{ "clean directory finished finished", 0 };

			Release (0, msgs, 0, 0);
		}
		else
		{
			CleanDir_DecStack ();
			ftp->ChangeToParentDirectory ();
		}
	}
}

void FtpClientWorker::CleanDir_IncStack ()
{
	if (m_stackDepth + 1 >= m_stackSize)
	{
		int stackSize = m_stackSize;
		stackSize >>= 4;
		stackSize++;
		stackSize <<= 4;
		int* stack = (int*) malloc (2 * stackSize * sizeof(int));
		if (stack == 0)
		{
			const char* const msgs [] =
			{ "malloc failed", 0 };

			Release (FtpCleanDirMallocFailed, msgs, 0, 0);
			return;
		}
		if (m_stack != 0)
		{
			memcpy (stack, m_stack, 3 * (m_stackDepth + 1) * sizeof(int));
			free (m_stack);
		}
		m_stack = stack;
		m_stackSize = stackSize;
	}
	m_stackDepth++;
	m_stack [3 * m_stackDepth] = m_listStart - m_listBuffer;
	m_stack [3 * m_stackDepth + 1] = m_listRead - m_listBuffer;
	m_stack [3 * m_stackDepth + 2] = m_listName - m_listBuffer;
}

void FtpClientWorker::CleanDir_DecStack ()
{
	if (m_stackDepth < 0)
		return;
	m_listPtr = m_listStart;
	m_listStart = m_listBuffer + m_stack [3 * m_stackDepth];
	m_listRead = m_listBuffer + m_stack [3 * m_stackDepth + 1];
	m_listName = m_listBuffer + m_stack [3 * m_stackDepth + 2];
	m_stackDepth--;
}

void FtpClientWorker::GenErrorReport (void*args [])
{
	char* msg = (char*) args [0];
	char* sys = (char*) args [1];
	int err = (int) (long) args [2];
	char* reqtxt = FtpRequestInterface::GetFtpRequestText (m_ftpRequest, '\t', '\n');

	if (reqtxt != 0)
	{
		cf_sc_printf (SC_SFTP, SC_ERR, "FTP job %d.%d.%d (S%06d) --- FTP failed:\n\t%s\n\t%s%s, %s%d\n\t%s",
			clientId (), m_jobCount, m_sessionId, m_sessionId % (1000 * 1000), msg, sys, ((err != 0) ? " failed" : ""),
			((err != 0) ? "errno = " : ""), err, reqtxt);
		free (reqtxt);
	}
	else
	{
		reqtxt = (char*) "CANNOT CREATE FTP REQUEST TEXT";
		cf_sc_printf (SC_SFTP, SC_ERR, "FTP job %d.%d.%d (S%06d) --- FTP failed:\n\t%s\n\t%s%s, %s%d\n\t%s",
			clientId (), m_jobCount, m_sessionId, m_sessionId % (1000 * 1000), msg, sys, ((err != 0) ? " failed" : ""),
			((err != 0) ? "errno = " : ""), err, reqtxt);
	}
}

bool FtpClientWorker::ReadListLine (char* line, DirEntryInfo &ent)
{
	char* parts [16];
	int count;

	bool param;
	for (param = false, count = 0; (count < 16) && (*line != 0); ++line)
	{
		if ((*line == ' ') || (*line == '\t') || (*line == '\r') || (*line == '\n'))
		{
			param = false;
			*line = 0;
			continue;
		}
		if (param)
			continue;
		parts [count++] = line;
		param = true;
	}

	memset (&ent, 0, sizeof(DirEntryInfo));
	switch (m_sysType)
	{
	case UnixSystem:
		return ReadUnixListLine (parts, count, ent);
		break;
	case LinuxSystem:
		return ReadLinuxListLine (parts, count, ent);
		break;
	case WindowsSystem:
		return ReadWindowsListLine (parts, count, ent);
		break;
	default:
		return false;
		break;
	}
	return false;
}

bool FtpClientWorker::ReadUnixListLine (char* parts [], int count, DirEntryInfo &ent)
{
	/* Examples of unix-like list replies:
	 drwxr-xr-x  28 rvod       rdsm          8192 Feb  9  2013 test
	 -rw-r-----   1 rvod       rdsm          1812 May  5  2009 sort.txt
	 lrwxrwxr-x   1 rvod       rdsm             7 Feb  9  2013 common -> /common
	 -rw-rw-rw-   1 rvod       rdsm          4161 Oct 22 09:31 BKQGXX03
	 */
	if (count < 9)
		return false;

	ent.size = atol (parts [4]);
	m_listName = parts [8];
	ent.name = parts [8];
	switch (*parts [0])
	{
	case 'd':
		ent.type = DT_DIR;
		break;
	case '-':
		ent.type = DT_REG;
		break;
	default:
		ent.type = 0;
		break;
	}
	return true;
}

bool FtpClientWorker::ReadLinuxListLine (char* parts [], int count, DirEntryInfo &ent)
{
	if (count < 9)
		return false;

	ent.size = atol (parts [4]);
	m_listName = parts [8];
	memset (&ent, 0, sizeof(DirEntryInfo));
	ent.name = parts [8];
	switch (*parts [0])
	{
	case 'd':
		ent.type = DT_DIR;
		break;
	case '-':
		ent.type = DT_REG;
		break;
	default:
		ent.type = 0;
		break;
	}
	return true;
}

bool FtpClientWorker::ReadWindowsListLine (char* parts [], int count, DirEntryInfo &ent)
{
	if (count < 4)
		return false;

	ent.size = 0;
	m_listName = parts [3];
	memset (&ent, 0, sizeof(DirEntryInfo));
	ent.name = parts [3];
	if (strstr (parts [2], "<DIR>") != 0)
		ent.type = DT_DIR;
	else
	{
		ent.size = atol (parts [2]);
		ent.type = DT_REG;
	}
	return true;
}

void FtpClientWorker::StartTask ()
{
	ctx (((MpxTaskMultiplexer*) mpx ())->ctx ());
}

void FtpClientWorker::StopTask ()
{
	Dispose ();
}

void FtpClientWorker::HandleInviteRequestEvent (MpxEventBase* event)
{
	SftpInviteRequest* inviteRequest = dynamic_cast <SftpInviteRequest*> (event);
	if (inviteRequest == 0)
		return;

	if (m_ctrlTask != 0)
		Send ((MpxTaskBase*) event->src (), new SftpInviteReply (false));
	else
	{
		if (m_request != 0)
			xdr_free ((xdrproc_t) xdr_FtpRequest, (char*) m_request);
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

void FtpClientWorker::HandleJobFinishedEvent (MpxEventBase* event)
{
	MpxJobFinishedEvent* jobFinishedEvent = dynamic_cast <MpxJobFinishedEvent*> (event);
	if (jobFinishedEvent == 0)
		return;

	MpxJobGetAddrInfo* jobGetAddrInfo = dynamic_cast <MpxJobGetAddrInfo*> (jobFinishedEvent->job ());
	if (jobGetAddrInfo == 0)
		return;

	m_addrinfo = jobGetAddrInfo->results ();
	delete jobGetAddrInfo;
	Send (m_ctrlTask, new SftpClientStart ());
	Execute (m_request);
}

} /* namespace sftp */
