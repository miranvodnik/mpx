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

#include "FtpClientInterface.h"

namespace sftp
{

u_long FtpClientInterface::g_clientId = 0;
u_long FtpClientInterface::g_sessionId = 0;
bool FtpClientInterface::g_debug = false;
bool FtpClientInterface::g_conversation = false;
bool FtpClientInterface::g_detailed = false;

} /* namespace sftp */
