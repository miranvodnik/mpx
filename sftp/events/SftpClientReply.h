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
// SftpClientReply.h
//
//  Created on: Feb 7, 2018
//      Author: miran
//

#pragma once

#include <mpx-events/MpxEventBase.h>

namespace sftp
{

class SftpClientReply: public mpx::MpxEventBase
{
public:
	SftpClientReply (u_long sessionId, char*args[]);
	SftpClientReply (u_long sessionId, char*args);
	virtual ~SftpClientReply ();
	virtual const char* Name ()
	{
		return "FTP/SFTP Client Reply";
	}
	virtual MpxEventBase* Copy ()
	{
		return new SftpClientReply (*this);
	}
	virtual int Encode (xdrproc_t& proc, xdrdata_t& data)
	{
		return 0;
	}
	virtual int Decode (MpxEventStruct* eventStruct)
	{
		return 0;
	}
	inline u_long sessionId () { return m_sessionId; }
	inline char** args () { return m_args; }
public:
	static const int ClientReplyEvent = 1005;
private:
	u_long m_sessionId;
	char** m_args;
};

} // namespace sftp
