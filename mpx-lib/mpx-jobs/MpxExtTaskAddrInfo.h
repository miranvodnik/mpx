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
// MpxExtTaskAddrInfo.h
//
//  Created on: Mar 8, 2018
//      Author: miran
//

#pragma once

#include <mpx-jobs/MpxJob.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
using namespace std;

namespace mpx
{

//
//
//
class MpxExtTaskAddrInfo: public MpxJob
{
public:
	MpxExtTaskAddrInfo (MpxTaskBase* sender, const char* hostName, MpxEventBase* query, int family,
		const addrinfo* hints);
	virtual ~MpxExtTaskAddrInfo ();
	virtual int Execute ();
	inline const char* hostName ()
	{
		return m_hostName.c_str ();
	}
	inline MpxEventBase* query ()
	{
		return m_query;
	}
	inline int family ()
	{
		return m_family;
	}
	inline addrinfo* results ()
	{
		addrinfo* results = m_results;
		m_results = 0;
		return results;
	}
private:
	string m_hostName;
	MpxEventBase* m_query;
	int m_family;
	addrinfo* m_hints;
	addrinfo* m_results;
};

} // namespace mpx
