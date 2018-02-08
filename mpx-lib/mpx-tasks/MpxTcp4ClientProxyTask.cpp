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

#include <mpx-tasks/MpxTcp4ClientProxyTask.h>

namespace mpx
{

MpxTcp4ClientProxyTask::MpxTcp4ClientProxyTask (MpxTaskBase* task, MpxTcp4Client* tcp4Client) :
	MpxProxyTask (task, tcp4Client)
{
	tcp4Client->task (this);
	RegisterEventHandler (AnyState, Tcp4ClientEvent, HandleTcp4ClientEvent, this);
}

MpxTcp4ClientProxyTask::~MpxTcp4ClientProxyTask ()
{
}

void MpxTcp4ClientProxyTask::HandleTcp4ClientEvent (MpxEventBase *event)
{
	MpxTcp4ClientEvent* tcp4ClientEvent = dynamic_cast <MpxTcp4ClientEvent*> (event);
	if (tcp4ClientEvent == 0)
		return;

	MpxTcp4Client* tcp4Client = dynamic_cast <MpxTcp4Client*> ((MpxTcp4Client*) tcp4ClientEvent->endPoint ());
	if (tcp4Client == 0)
		return;

	switch (tcp4ClientEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp4ClientEvent->error () == 0) && (tcp4ClientEvent->size () != 0))
		{
			MpxEventStruct* req = new MpxEventStruct;
			memset (req, 0, sizeof(MpxEventStruct));
			if (tcp4Client->ReadXdrRequest ((xdrproc_t) xdr_MpxEventStruct, (void*) req) < 0)
			{
				delete req;
				if (false)
					cout << "proxy " << this << " event: tcp4 client input error" << endl;
			}
			else
			{
				if (false)
					cout << "proxy " << this << " event: tcp4 client input ok" << endl;
				xdrpair_t xdrPair = MpxXDRProcRegistry::Retrieve (req->m_code);
				evnalloc_t evnAlloc = xdrPair.second.second;
				if (evnAlloc == 0)
				{
					if (false)
						cout << "proxy " << this << " missing event allocator" << endl;
				}
				else
				{
					MpxEventBase* event = evnAlloc ();
					event->Decode (req);
					Send (m_task, event, false);
				}
				xdr_free ((xdrproc_t) xdr_int, (char*) req);
			}
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: tcp4 client output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp4 client default" << endl;
		break;
	}
}

} // namespace mpx
