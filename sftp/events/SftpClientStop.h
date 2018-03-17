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
// SftpClientStop.h
//
//  Created on: Feb 7, 2018
//      Author: miran
//

#pragma once

#include <mpx-events/MpxEventBase.h>

namespace sftp
{

class SftpClientStop: public mpx::MpxEventBase
{
public:
	SftpClientStop ();
	virtual ~SftpClientStop ();
	virtual const char* Name ()
	{
		return "FTP/SFTP Client Stopped";
	}
	virtual MpxEventBase* Copy ()
	{
		return new SftpClientStop (*this);
	}
public:
	static const int EventCode = 1003;
};

} // namespace sftp
