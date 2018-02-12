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

#include <mpx-tasks/MpxTcp4EndPointProxyTask.h>

namespace mpx
{

EventDescriptor MpxTcp4EndPointProxyTask::g_evntab[] =
{
	{ AnyState, Tcp4EndPointEvent, HandleTcp4EndPointEvent, 0 },
	{ 0, 0, 0, 0 }
};

MpxTcp4EndPointProxyTask::MpxTcp4EndPointProxyTask (MpxTaskBase* task, MpxTcp4EndPoint* tcp4EndPoint) :
	MpxProxyTask (task, tcp4EndPoint)
{
	tcp4EndPoint->task (this);
	RegisterEventHandlers (g_evntab);
}

MpxTcp4EndPointProxyTask::~MpxTcp4EndPointProxyTask ()
{
}

void MpxTcp4EndPointProxyTask::HandleTcp4EndPointEvent (MpxEventBase *event)
{
	MpxTcp4EndPointEvent* tcp4EndPointEvent = dynamic_cast <MpxTcp4EndPointEvent*> (event);
	if (tcp4EndPointEvent == 0)
		return;

	MpxTcp4EndPoint* tcp4EndPoint = dynamic_cast <MpxTcp4EndPoint*> ((MpxTcp4EndPoint*) tcp4EndPointEvent->endPoint ());
	if (tcp4EndPoint == 0)
		return;

	switch (tcp4EndPointEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp4EndPointEvent->error () == 0) && (tcp4EndPointEvent->size () != 0))
		{
			MpxEventStruct* req = new MpxEventStruct;
			memset (req, 0, sizeof(MpxEventStruct));
			if (tcp4EndPoint->ReadXdrRequest ((xdrproc_t) xdr_MpxEventStruct, (void*) req) < 0)
			{
				delete req;
				if (false)
					cout << "proxy " << this << " event: tcp4 end point input error" << endl;
			}
			else
			{
				if (false)
					cout << "proxy " << this << " event: tcp4 end point input ok" << endl;
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
			cout << "proxy " << this << " event: tcp4 end point output" << endl;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp4 end point default" << endl;
		break;
	}
}

} // namespace mpx
