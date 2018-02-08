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

#include <ftpmsg/FtpRequestInterface.h>
#include <mpx-tasks/MpxTaskBase.h>
using namespace mpx;

namespace sftp
{

#define	CWD_HOME_DIR	1
#define	CWD_HOME_DIR_AGAIN	2
#define	CWD_MKDIR	3
#define	CWD_REMOTE_DIR	4
#define	CWD_REMOTE_DIR_AGAIN	5
#define	CWD_ROOT	6
#define	CWD_WORKING_DIR	7

#define	LIST_PROTECT_EXISTING_FILE	1
#define	LIST_CHECK_FILE_SIZE	2

typedef enum
{
	UnknownProtocol, Ssh2Protocol, FtpProtocol
} FtpClientProtocol;

class FtpClientInterface : public MpxTaskBase
{
public:
	FtpClientInterface (FtpClientProtocol protocol) :
		m_protocol (protocol)
	{
		m_clientId = ++g_clientId;
		m_jobCount = 0;
	}
	virtual ~FtpClientInterface ()
	{
	}

public:
	virtual bool IsAlive () = 0;
	virtual void Start () = 0;
	virtual void Stop () = 0;
	virtual void Execute (FtpRequest* req, void* addInfo = 0) = 0;
	inline FtpClientProtocol protocol ()
	{
		return m_protocol;
	}
	inline static void debug (bool debug)
	{
		g_debug = debug;
	}
	inline static bool debug (void)
	{
		return g_debug;
	}
	inline static void conversation (bool conversation)
	{
		g_conversation = conversation;
	}
	inline static bool conversation (void)
	{
		return g_conversation;
	}
	inline static void detailed (bool detailed)
	{
		g_detailed = detailed;
	}
	inline static bool detailed (void)
	{
		return g_detailed;
	}
	inline u_long clientId ()
	{
		return m_clientId;
	}
protected:
	static u_long g_clientId;
	static u_long g_sessionId;
	static bool g_debug;
	static bool g_conversation;
	static bool g_detailed;
	u_long m_jobCount;

private:
	u_long m_clientId;
	FtpClientProtocol m_protocol;
};

} /* namespace sftp */
