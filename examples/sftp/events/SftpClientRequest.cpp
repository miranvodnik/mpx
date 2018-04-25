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
// SftpClientRequest.cpp
//
//  Created on: Feb 7, 2018
//      Author: miran
//

#include <events/SftpClientRequest.h>
#include <sstream>
using namespace std;

namespace sftp
{

SftpClientRequest::SftpClientRequest (u_long sessionId, char*args []) :
	MpxEventBase (SftpClientRequest::EventCode), m_sessionId (sessionId)
{
	int size;
	char** ptr;

	for (ptr = args, size = 0; *ptr != 0; ++ptr, ++size)
		;
	m_args = new char* [size + 1];
	for (ptr = args, size = 0; *ptr != 0; ++ptr, ++size)
		m_args [size] = strdup (*ptr);
	m_args [size] = 0;
}

SftpClientRequest::SftpClientRequest (u_long sessionId, char*args) :
	MpxEventBase (SftpClientRequest::EventCode), m_sessionId (sessionId)
{
	m_args = new char* [2];
	m_args [0] = strdup (args);
	m_args [1] = 0;
}

SftpClientRequest::~SftpClientRequest ()
{
	for (char** ptr = m_args; *ptr != 0; ++ptr)
		free (*ptr);
	delete [] m_args;
}

} // namespace sftp
