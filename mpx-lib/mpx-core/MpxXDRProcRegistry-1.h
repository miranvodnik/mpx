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

#include <pthread.h>
#include <rpc/rpc.h>
#include <mpx-events/MpxEventBase.h>

#include <map>
using namespace std;

namespace mpx
{

typedef void* (*xdralloc_t) ();
typedef MpxEventBase* (*evnalloc_t) ();

typedef pair <xdralloc_t, evnalloc_t> allocator_t;
typedef pair <xdrproc_t, allocator_t> xdrpair_t;
typedef map <int, xdrpair_t> xdrset;

class MpxXDRProcRegistry
{
private:
	MpxXDRProcRegistry ();
	virtual ~MpxXDRProcRegistry ();
public:
	static inline int Register (int id, xdrproc_t proc, xdralloc_t xdralloc, evnalloc_t evnalloc)
	{
		return g_XDRProcRegistry->_Register (id, proc, xdralloc, evnalloc);
	}
	static inline xdrpair_t Retrieve (int id)
	{
		return g_XDRProcRegistry->_Retrieve (id);
	}
private:
	int _Register (int id, xdrproc_t proc, xdralloc_t xdralloc, evnalloc_t evnalloc);
	xdrpair_t _Retrieve (int id);
private:
	static MpxXDRProcRegistry* g_XDRProcRegistry;
	pthread_rwlock_t m_rwlock;
	xdrset m_xdrset;
};

} // namespace mpx
