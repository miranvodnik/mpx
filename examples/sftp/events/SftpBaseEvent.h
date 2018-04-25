//
// SftpBaseEvent.h
//
//  Created on: Jan 31, 2018
//      Author: miran
//

#pragma once

#include <mpx-events/MpxEventBase.h>
#include <ftpcln/SftpClientEnums.h>

namespace sftp
{

enum SftpProtocolEvent
{
	CallbackEvent = 1000,
	InviteRequestEvent = 1001,
	InviteResponseEvent = 1002,
	ConnectRequestEvent = 1003,
	HandshakeRequestEvent = 1004,
};

class SftpBaseEvent: public mpx::MpxEventBase
{
public:
	SftpBaseEvent (u_int baseCode, SftpEventIndex event, SftpContext context, SftpStatus status, SftpState state);
	virtual ~SftpBaseEvent ();

	inline SftpEventIndex event () { return m_event; }
	inline SftpContext context () { return m_context; }
	inline SftpStatus status () { return m_status; }
	inline SftpState state () { return m_state; }
private:
	SftpEventIndex m_event;
	SftpContext m_context;
	SftpStatus m_status;
	SftpState m_state;
};

} // namespace sftp
