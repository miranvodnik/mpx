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
// SftpClientEnums.h
//
//  Created on: Mar 29, 2018
//      Author: miran
//

#pragma once

namespace sftp
{

enum SftpContext
{
	sftp_unknown, sftp_ctrl, sftp_data, sftp_request, sftp_reply, sftp_start, sftp_stop, sftp_progress
};

enum SftpStatus
{
	sftp_success, sftp_failure, sftp_error
};

typedef enum SftpState
{
	//	initial state (value = 0)
	SNull,

	SStartup,
	SConnectPrepared,
	SConnected,
	SHandshakePrepared,
	SHandshakeFinished,
	SHashPrepared,
	Sha1Hash,
	Md5Hash,
	SAuthPrepared,
	SAuthFinished,
	SSftpPrepared,
	SSftpFinished,
	SPwdPrepared,
	SPwdFinished,
	SCwdPrepared,
	SCwdFinished,
	SCdupPrepared,
	SCdupFinished,
	SLPwdPrepared,
	SLPwdFinished,
	SLCwdPrepared,
	SLCwdFinished,
	SLCdupPrepared,
	SLCdupFinished,
	SMkdirPrepared,
	SMkdirFinished,
	SRmdirPrepared,
	SRmdirFinished,
	SListDirPrepared,
	SListDirProgress,
	SListDirFinished,
	SPutFilePrepared,
	SPutFileProgress,
	SPutFileFinished,
	SGetFilePrepared,
	SGetFileProgress,
	SGetFileFinished,
	SRenamePrepared,
	SRenameFinished,
	SRemovePrepared,
	SRemoveFinished,
	SListPrepared,
	SListFinished,
	SDisposed,

	SFtpStateCount
} SftpState;

typedef enum SftpEventIndex
{
	//	initial event (value = 0)
	SftpNullEvent,

	//	pure protocol events
	SStartupEvent,
	SConnectPreparedEvent,
	SConnectedEvent,
	SHandshakePreparedEvent,
	SHandshakeFinishedEvent,
	SHashPreparedEvent,
	Sha1HashEvent,
	Md5HashEvent,
	SAuthPreparedEvent,
	SAuthFinishedEvent,
	SSftpPreparedEvent,
	SSftpFinishedEvent,
	SPwdPreparedEvent,
	SPwdFinishedEvent,
	SCwdPreparedEvent,
	SCwdFinishedEvent,
	SCdupPreparedEvent,
	SCdupFinishedEvent,
	SLPwdPreparedEvent,
	SLPwdFinishedEvent,
	SLCwdPreparedEvent,
	SLCwdFinishedEvent,
	SLCdupPreparedEvent,
	SLCdupFinishedEvent,
	SMkdirPreparedEvent,
	SMkdirFinishedEvent,
	SRmdirPreparedEvent,
	SRmdirFinishedEvent,
	SListDirPreparedEvent,
	SListDirProgressEvent,
	SListDirFinishedEvent,
	SPutFilePreparedEvent,
	SPutFileProgressEvent,
	SPutFileFinishedEvent,
	SGetFilePreparedEvent,
	SGetFileProgressEvent,
	SGetFileFinishedEvent,
	SRenamePreparedEvent,
	SRenameFinishedEvent,
	SRemovePreparedEvent,
	SRemoveFinishedEvent,
	SListPreparedEvent,
	SListFinishedEvent,
	SDisposedEvent,

	//	utility events
	SErrorEvent,
	SRequestEvent,
	SReplyEvent,

	//	control events
	SCtrlBusyTimerExpiredEvent,
	SCtrlIdleTimerExpiredEvent,

	SftpEventCount
} SftpEventIndex;

};
