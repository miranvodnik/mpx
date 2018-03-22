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
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

MpxTcp4EndPointProxyTask::MpxTcp4EndPointProxyTask (MpxTaskBase* task, const char* encdeclib, MpxTcp4EndPoint* tcp4EndPoint) :
	MpxProxyTask (task, tcp4EndPoint, encdeclib, false)
{
	tcp4EndPoint->task (this);
}

MpxTcp4EndPointProxyTask::~MpxTcp4EndPointProxyTask ()
{
}

void MpxTcp4EndPointProxyTask::HandleSocketEvent (MpxEventBase *event)
{
	MpxTcp4EndPointEvent* tcp4EndPointEvent = dynamic_cast <MpxTcp4EndPointEvent*> (event);
	if (tcp4EndPointEvent == 0)
		return;

	MpxTcp4EndPoint* tcp4EndPoint = reinterpret_cast <MpxTcp4EndPoint*> (tcp4EndPointEvent->endPoint ());
	if (tcp4EndPoint == 0)
		return;

	switch (tcp4EndPointEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp4EndPointEvent->error () == 0) && (tcp4EndPointEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = tcp4EndPoint->DecodeEvent(m_eventXDR)) != 0)
				Send (m_task, event, false);
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
