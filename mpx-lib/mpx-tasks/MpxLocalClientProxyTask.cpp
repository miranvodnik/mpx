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

#include <mpx-tasks/MpxLocalClientProxyTask.h>

namespace mpx
{

MpxLocalClientProxyTask::MpxLocalClientProxyTask (MpxTaskBase* task, MpxLocalClient* localClient) :
	MpxProxyTask (task, localClient)
{
	localClient->task (this);
	RegisterEventHandler (AnyState, LocalClientEvent, HandleLocalClientEvent, this);
}

MpxLocalClientProxyTask::~MpxLocalClientProxyTask ()
{
}

void MpxLocalClientProxyTask::HandleLocalClientEvent (MpxEventBase *event)
{
	MpxLocalClientEvent* localClientEvent = dynamic_cast <MpxLocalClientEvent*> (event);
	if (localClientEvent == 0)
		return;

	MpxLocalClient* localClient = dynamic_cast <MpxLocalClient*> ((MpxLocalClient*) localClientEvent->endPoint ());
	if (localClient == 0)
		return;

	switch (localClientEvent->flags ())
	{
	case EPOLLIN:
		if ((localClientEvent->error () == 0) && (localClientEvent->size () != 0))
		{
			MpxEventStruct* req = new MpxEventStruct;
			memset (req, 0, sizeof(MpxEventStruct));
			if (localClient->ReadXdrRequest ((xdrproc_t) xdr_MpxEventStruct, (void*) req) < 0)
			{
				delete req;
				if (false)
					cout << "proxy " << this << " event: local client input error" << endl;
			}
			else
			{
				if (false)
					cout << "proxy " << this << " event: local client input ok" << endl;
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
			cout << "proxy " << this << " event: local client output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: local client default" << endl;
		break;
	}
}

} // namespace mpx
