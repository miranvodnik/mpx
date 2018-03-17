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

#include <mpx-core/MpxXDRProcRegistry.h>

namespace mpx
{

MpxXDRProcRegistry* MpxXDRProcRegistry::g_XDRProcRegistry = new MpxXDRProcRegistry ();

MpxXDRProcRegistry::MpxXDRProcRegistry ()
{
	pthread_rwlock_init (&m_rwlock, 0);
}

MpxXDRProcRegistry::~MpxXDRProcRegistry ()
{
	pthread_rwlock_destroy (&m_rwlock);
}

int MpxXDRProcRegistry::_Register (int id, xdrproc_t proc, xdralloc_t xdralloc, evnalloc_t evnalloc)
{
	xdrset::iterator it = m_xdrset.find (id);
	if (it != m_xdrset.end ())
		return it->first;
	m_xdrset [id] = xdrpair_t (proc, allocator_t (xdralloc, evnalloc));
	return id;
}

xdrpair_t MpxXDRProcRegistry::_Retrieve (int id)
{
	xdrset::iterator it = m_xdrset.find (id);
	if (it == m_xdrset.end ())
		return xdrpair_t (0, allocator_t (0, 0));
	return it->second;
}

} // namespace mpx
