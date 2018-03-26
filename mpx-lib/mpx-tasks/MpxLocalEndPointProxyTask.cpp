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

#include <mpx-tasks/MpxLocalEndPointProxyTask.h>
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

MpxLocalEndPointProxyTask::MpxLocalEndPointProxyTask (MpxTaskBase* task, const char* encdeclib,
	MpxLocalEndPoint* localEndPoint) :
	MpxProxyTask (task, localEndPoint, encdeclib, false)
{
	localEndPoint->task (this);
}

MpxLocalEndPointProxyTask::~MpxLocalEndPointProxyTask ()
{
}

void MpxLocalEndPointProxyTask::HandleSocketEvent (MpxEventBase *event)
{
	MpxLocalEndPointEvent* localEndPointEvent = dynamic_cast <MpxLocalEndPointEvent*> (event);
	if (localEndPointEvent == 0)
		return;

	MpxLocalEndPoint* localEndPoint = reinterpret_cast <MpxLocalEndPoint*> (localEndPointEvent->endPoint ());
	if (localEndPoint == 0)
		return;

	switch (localEndPointEvent->flags ())
	{
	case EPOLLIN:
		if ((localEndPointEvent->error () == 0) && (localEndPointEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = localEndPoint->DecodeEvent (m_eventXDR)) != 0)
				Send (m_task, event, true);
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: local end point output" << endl;
		if ((localEndPointEvent->error () == 0) && (localEndPointEvent->size () != 0))
			return;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: local end point default" << endl;
		break;
	}
	cout << "CONNECTION BROKEN" << endl;
	Send (m_task, new MpxProxyTaskEvent (MpxProxyTaskEvent::ReasonConnectionBroken), true);
	Dispose (false);
}

} // namespace mpx
