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
#include <mpx-tasks/MpxLocalEndPointProxyTask.h>

namespace mpx
{

MpxLocalEndPointProxyTask::MpxLocalEndPointProxyTask (MpxTaskBase* task, MpxLocalEndPoint* localEndPoint) :
	MpxProxyTask (task, localEndPoint)
{
	localEndPoint->task (this);
	RegisterEventHandler (AnyState, LocalEndPointEvent, HandleLocalEndPointEvent, this);
}

MpxLocalEndPointProxyTask::~MpxLocalEndPointProxyTask ()
{
}

void MpxLocalEndPointProxyTask::HandleLocalEndPointEvent (MpxEventBase *event)
{
	MpxLocalEndPointEvent* localEndPointEvent = dynamic_cast <MpxLocalEndPointEvent*> (event);
	if (localEndPointEvent == 0)
		return;

	MpxLocalEndPoint* localEndPoint =
		dynamic_cast <MpxLocalEndPoint*> ((MpxLocalEndPoint*) localEndPointEvent->endPoint ());
	if (localEndPoint == 0)
		return;

	switch (localEndPointEvent->flags ())
	{
	case EPOLLIN:
		if ((localEndPointEvent->error () == 0) && (localEndPointEvent->size () != 0))
		{
			MpxEventStruct* req = new MpxEventStruct;
			memset (req, 0, sizeof(MpxEventStruct));
			if (localEndPoint->ReadXdrRequest ((xdrproc_t) xdr_MpxEventStruct, (void*) req) < 0)
			{
				delete req;
				if (false)
					cout << "proxy " << this << " event: local end point input error" << endl;
			}
			else
			{
				if (false)
					cout << "proxy " << this << " event: local end point input ok" << endl;
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
			cout << "proxy " << this << " event: local end point output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: local end point default" << endl;
		break;
	}
}

} // namespace mpx
