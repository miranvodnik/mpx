//    Event Driven Task Multiplexing Library
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

#include <sys/types.h>
#include <stdlib.h>
#include <mpx-tasks/mpx-messages.h>

#include <iostream>
#include <map>
using namespace std;

namespace mpx
{

#define	_MPX_STR2(line) #line
#define	_MPX_STR(line) _MPX_STR2(line)
#define	_mpx_src_info __FILE__ ":" _MPX_STR(__LINE__)

class MpxError
{
public:
	MpxError (u_int errorCode, const char* errorMessage, const char* sourceInfo = 0) :
		m_errorCode (errorCode), m_errorMessage (errorMessage), m_sourceInfo (sourceInfo)
	{
	}
	inline u_int errorCode ()
	{
		return m_errorCode;
	}
	inline const char* errorMessage ()
	{
		return m_errorMessage.c_str ();
	}
	inline const char* sourceInfo ()
	{
		return m_sourceInfo.c_str ();
	}
private:
	u_int m_errorCode;
	string m_errorMessage;
	string m_sourceInfo;
};

class MpxUtilities
{
private:
	MpxUtilities ()
	{
		std::runtime_error ("Error");
	}
	virtual ~MpxUtilities ()
	{
	}
public:
	static void memcpy (void* dest, const void* src, size_t n);
};

} /* namespace mpx */
