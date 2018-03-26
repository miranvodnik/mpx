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
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

MpxTcp6EndPointProxyTask::MpxTcp6EndPointProxyTask (MpxTaskBase* task, const char* encdeclib,
	MpxTcp6EndPoint* tcp6EndPoint) :
	MpxProxyTask (task, tcp6EndPoint, encdeclib, false)
{
	tcp6EndPoint->task (this);
}

MpxTcp6EndPointProxyTask::~MpxTcp6EndPointProxyTask ()
{
}

void MpxTcp6EndPointProxyTask::HandleSocketEvent (MpxEventBase *event)
{
	MpxTcp6EndPointEvent* tcp6EndPointEvent = dynamic_cast <MpxTcp6EndPointEvent*> (event);
	if (tcp6EndPointEvent == 0)
		return;

	MpxTcp6EndPoint* tcp6EndPoint = reinterpret_cast <MpxTcp6EndPoint*> (tcp6EndPointEvent->endPoint ());
	if (tcp6EndPoint == 0)
		return;

	switch (tcp6EndPointEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp6EndPointEvent->error () == 0) && (tcp6EndPointEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = tcp6EndPoint->DecodeEvent (m_eventXDR)) != 0)
				Send (m_task, event, true);
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: tcp6 end point output" << endl;
		if ((tcp6EndPointEvent->error () == 0) && (tcp6EndPointEvent->size () != 0))
			return;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp6 end point default" << endl;
		break;
	}
	cout << "CONNECTION BROKEN" << endl;
	Send (m_task, new MpxProxyTaskEvent (MpxProxyTaskEvent::ReasonConnectionBroken), true);
	Dispose (false);
}

} // namespace mpx
