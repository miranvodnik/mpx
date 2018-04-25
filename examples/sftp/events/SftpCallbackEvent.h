//
// SftpCallbackEvent.h
//
//  Created on: Jan 31, 2018
//      Author: miran
//

#pragma once

#include <events/SftpBaseEvent.h>

namespace sftp
{

class SftpCallbackEvent: public SftpBaseEvent
{
public:
	SftpCallbackEvent (SftpEventIndex index, SftpContext context, SftpStatus status, SftpState state);
	virtual ~SftpCallbackEvent ();
	virtual const char* Name () { return "Callback"; }
	virtual MpxEventBase* Copy () { return new SftpCallbackEvent (*this); }
public:
	static const unsigned int EventCode = (unsigned int) CallbackEvent;
};

} // namespace sftp
