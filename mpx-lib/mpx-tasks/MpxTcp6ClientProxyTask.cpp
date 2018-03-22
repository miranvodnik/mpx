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
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

MpxTcp6ClientProxyTask::MpxTcp6ClientProxyTask (MpxTaskBase* task, const char* encdeclib, MpxTcp6Client* tcp6Client) :
	MpxProxyTask (task, tcp6Client, encdeclib, true)
{
	tcp6Client->task (this);
}

MpxTcp6ClientProxyTask::~MpxTcp6ClientProxyTask ()
{
}

void MpxTcp6ClientProxyTask::HandleSocketEvent (MpxEventBase *event)
{
	MpxTcp6ClientEvent* tcp6ClientEvent = dynamic_cast <MpxTcp6ClientEvent*> (event);
	if (tcp6ClientEvent == 0)
		return;

	MpxTcp6Client* tcp6Client = reinterpret_cast <MpxTcp6Client*> (tcp6ClientEvent->endPoint ());
	if (tcp6Client == 0)
		return;

	switch (tcp6ClientEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp6ClientEvent->error () == 0) && (tcp6ClientEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = tcp6Client->DecodeEvent(m_eventXDR)) != 0)
				Send (m_task, event, false);
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
