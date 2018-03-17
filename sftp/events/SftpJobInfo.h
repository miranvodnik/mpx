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
// SftpJobInfo.h
//
//  Created on: Feb 8, 2018
//      Author: miran
//

#pragma once

#include <mpx-events/MpxEventBase.h>

namespace sftp
{

class SftpJobInfo: public mpx::MpxEventBase
{
public:
	SftpJobInfo (u_int clientId, u_int jobCount, u_int sessionId, char* msg);
	virtual ~SftpJobInfo ();
	virtual const char* Name ()
	{
		return "Invite Reply";
	}
	virtual MpxEventBase* Copy ()
	{
		return new SftpJobInfo (*this);
	}
	inline u_int clientId ()
	{
		return m_clientId;
	}
	inline u_int jobCount ()
	{
		return m_jobCount;
	}
	inline u_int sessionId ()
	{
		return m_sessionId;
	}
	inline char* msg ()
	{
		return m_msg;
	}
public:
	static const int EventCode = 1006;
private:
	u_int m_clientId;
	u_int m_jobCount;
	u_int m_sessionId;
	char* m_msg;
};

} // namespace sftp
