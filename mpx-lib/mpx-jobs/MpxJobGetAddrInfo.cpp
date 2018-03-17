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

#include <mpx-jobs/MpxJobGetAddrInfo.h>

namespace mpx
{

MpxJobGetAddrInfo::MpxJobGetAddrInfo (MpxTaskBase* task, const char* node, const char* service,
	const struct addrinfo* hints) :
	MpxJob (task), m_node (node), m_service (service)
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

MpxJobGetAddrInfo::~MpxJobGetAddrInfo ()
{
	freeaddrinfo (m_hints);
	freeaddrinfo (m_results);
}

int MpxJobGetAddrInfo::Execute ()
{
	getaddrinfo (m_node.c_str (), m_service.c_str (), m_hints, &m_results);
	return 0;
}

} /* namespace mpx */
