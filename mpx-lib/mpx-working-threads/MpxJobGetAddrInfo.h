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
#include <sys/socket.h>
#include <netdb.h>
#include <mpx-working-threads/MpxJob.h>

#include <string>
using namespace std;

namespace mpx
{

class MpxJobGetAddrInfo: public mpx::MpxJob
{
public:
	MpxJobGetAddrInfo (MpxTaskBase* task, const char* node, const char* service, const struct addrinfo* hints);
	virtual ~MpxJobGetAddrInfo ();
	virtual int Execute ();
	inline struct addrinfo* results ()
	{
		addrinfo* results = m_results;
		m_results = 0;
		return results;
	}
private:
	string m_node;
	string m_service;
	struct addrinfo* m_hints;
	struct addrinfo* m_results;
};

} /* namespace mpx */
