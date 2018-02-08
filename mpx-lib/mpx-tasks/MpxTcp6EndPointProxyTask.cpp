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

#include <mpx-tasks/MpxTcp6EndPointProxyTask.h>

namespace mpx
{

MpxTcp6EndPointProxyTask::MpxTcp6EndPointProxyTask (MpxTaskBase* task, MpxTcp6EndPoint* tcp6EndPoint) :
	MpxProxyTask (task, tcp6EndPoint)
{
	tcp6EndPoint->task (this);
	RegisterEventHandler (AnyState, Tcp6EndPointEvent, HandleTcp6EndPointEvent, this);
}

MpxTcp6EndPointProxyTask::~MpxTcp6EndPointProxyTask ()
{
}

void MpxTcp6EndPointProxyTask::HandleTcp6EndPointEvent (MpxEventBase *event)
{
	MpxTcp6EndPointEvent* tcp6EndPointEvent = dynamic_cast <MpxTcp6EndPointEvent*> (event);
	if (tcp6EndPointEvent == 0)
		return;

	MpxTcp6EndPoint* tcp6EndPoint = dynamic_cast <MpxTcp6EndPoint*> ((MpxTcp6EndPoint*) tcp6EndPointEvent->endPoint ());
	if (tcp6EndPoint == 0)
		return;

	switch (tcp6EndPointEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp6EndPointEvent->error () == 0) && (tcp6EndPointEvent->size () != 0))
		{
			MpxEventStruct* req = new MpxEventStruct;
			memset (req, 0, sizeof(MpxEventStruct));
			if (tcp6EndPoint->ReadXdrRequest ((xdrproc_t) xdr_MpxEventStruct, (void*) req) < 0)
			{
				delete req;
				if (false)
					cout << "proxy " << this << " event: tcp6 end point input error" << endl;
			}
			else
			{
				if (false)
					cout << "proxy " << this << " event: tcp6 end point input ok" << endl;
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
			cout << "proxy " << this << " event: tcp6 end point output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp6 end point default" << endl;
		break;
	}
}

} // namespace mpx
