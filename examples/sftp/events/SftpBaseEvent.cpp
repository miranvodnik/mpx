//
// SftpBaseEvent.cpp
//
//  Created on: Jan 31, 2018
//      Author: miran
//

#include <events/SftpBaseEvent.h>

namespace sftp
{

SftpBaseEvent::SftpBaseEvent (u_int baseCode, SftpEventIndex event, SftpContext context, SftpStatus status,
	SftpState state) :
	MpxEventBase (baseCode), m_event (event), m_context (context), m_status (status), m_state (state)
{
}

SftpBaseEvent::~SftpBaseEvent ()
{
}

} // namespace sftp
