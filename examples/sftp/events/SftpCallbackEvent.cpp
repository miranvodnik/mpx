//
// SftpCallbackEvent.cpp
//
//  Created on: Jan 31, 2018
//      Author: miran
//

#include <events/SftpCallbackEvent.h>

namespace sftp
{

SftpCallbackEvent::SftpCallbackEvent (SftpEventIndex event, SftpContext context, SftpStatus status, SftpState state) :
	SftpBaseEvent (EventCode, event, context, status, state)
{
}

SftpCallbackEvent::~SftpCallbackEvent ()
{
}

} // namespace sftp
