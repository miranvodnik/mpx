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

#include <mpx-events/MpxEventBase.h>

namespace sftp
{

class SftpInviteReply: public mpx::MpxEventBase
{
public:
	SftpInviteReply (bool invite);
	virtual ~SftpInviteReply ();
	virtual const char* Name ()
	{
		return "Invite Reply";
	}
	virtual MpxEventBase* Copy ()
	{
		return new SftpInviteReply (*this);
	}
	virtual int Encode (xdrproc_t& proc, xdrdata_t& data)
	{
		return 0;
	}
	virtual int Decode (MpxEventStruct* eventStruct)
	{
		return 0;
	}
	inline bool invite ()
	{
		return m_invite;
	}
public:
	static const int InviteReplyEvent = 1001;
private:
	bool m_invite;
};

} // namespace sftp
