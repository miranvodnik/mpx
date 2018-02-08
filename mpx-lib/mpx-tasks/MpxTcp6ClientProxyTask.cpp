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

#include <mpx-tasks/MpxTcp6ClientProxyTask.h>

namespace mpx
{

MpxTcp6ClientProxyTask::MpxTcp6ClientProxyTask (MpxTaskBase* task, MpxTcp6Client* tcp6Client) :
	MpxProxyTask (task, tcp6Client)
{
	tcp6Client->task (this);
	RegisterEventHandler (AnyState, Tcp6ClientEvent, HandleTcp6ClientEvent, this);
}

MpxTcp6ClientProxyTask::~MpxTcp6ClientProxyTask ()
{
}

void MpxTcp6ClientProxyTask::HandleTcp6ClientEvent (MpxEventBase *event)
{
	MpxTcp6ClientEvent* tcp6ClientEvent = dynamic_cast <MpxTcp6ClientEvent*> (event);
	if (tcp6ClientEvent == 0)
		return;

	MpxTcp6Client* tcp6Client = dynamic_cast <MpxTcp6Client*> ((MpxTcp6Client*) tcp6ClientEvent->endPoint ());
	if (tcp6Client == 0)
		return;

	switch (tcp6ClientEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp6ClientEvent->error () == 0) && (tcp6ClientEvent->size () != 0))
		{
			MpxEventStruct* req = new MpxEventStruct;
			memset (req, 0, sizeof(MpxEventStruct));
			if (tcp6Client->ReadXdrRequest ((xdrproc_t) xdr_MpxEventStruct, (void*) req) < 0)
			{
				delete req;
				if (false)
					cout << "proxy " << this << " event: tcp6 client input error" << endl;
			}
			else
			{
				if (false)
					cout << "proxy " << this << " event: tcp6 client input ok" << endl;
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
			cout << "proxy " << this << " event: tcp6 client output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp6 client default" << endl;
		break;
	}
}

} // namespace mpx
