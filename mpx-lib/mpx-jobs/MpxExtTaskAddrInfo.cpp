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
// MpxExtTaskAddrInfo.cpp
//
//  Created on: Mar 8, 2018
//      Author: miran
//

#include <mpx-jobs/MpxExtTaskAddrInfo.h>

namespace mpx
{

MpxExtTaskAddrInfo::MpxExtTaskAddrInfo (MpxTaskBase* sender, const char* hostName, MpxEventBase* query, int family, const addrinfo* hints) :
	MpxJob (sender), m_hostName (hostName), m_query (query), m_family (family)
{
	m_hints = 0;
	m_results = 0;
	struct addrinfo** info = &m_hints;
	for (; hints != 0; hints = hints->ai_next)
	{
		*info = (struct addrinfo*) malloc (sizeof(struct addrinfo));
		if (*info == 0)
			break;
		**info = *hints;
		info = &((*info)->ai_next = 0);
	}
}

MpxExtTaskAddrInfo::~MpxExtTaskAddrInfo ()
{
	delete m_query;
	freeaddrinfo (m_hints);
	freeaddrinfo (m_results);
}

int MpxExtTaskAddrInfo::Execute ()
{
	getaddrinfo (m_hostName.c_str (), 0, m_hints, &m_results);
	return 0;
}

} // namespace mpx
