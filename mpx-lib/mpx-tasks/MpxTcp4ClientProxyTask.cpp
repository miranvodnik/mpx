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
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

MpxTcp4ClientProxyTask::MpxTcp4ClientProxyTask (MpxTaskBase* task, const char* encdeclib, MpxTcp4Client* tcp4Client) :
	MpxProxyTask (task, tcp4Client, encdeclib, true)
{
	tcp4Client->task (this);
}

MpxTcp4ClientProxyTask::~MpxTcp4ClientProxyTask ()
{
}

void MpxTcp4ClientProxyTask::HandleSocketEvent (MpxEventBase *event)
{
	MpxTcp4ClientEvent* tcp4ClientEvent = dynamic_cast <MpxTcp4ClientEvent*> (event);
	if (tcp4ClientEvent == 0)
		return;

	MpxTcp4Client* tcp4Client = reinterpret_cast <MpxTcp4Client*> (tcp4ClientEvent->endPoint ());
	if (tcp4Client == 0)
		return;

	switch (tcp4ClientEvent->flags ())
	{
	case EPOLLIN:
		if ((tcp4ClientEvent->error () == 0) && (tcp4ClientEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = tcp4Client->DecodeEvent (m_eventXDR)) != 0)
				Send (m_task, event, true);
			return;
		}
		break;
	case EPOLLOUT:
		if (false)
			cout << "proxy " << this << " event: tcp4 client output" << endl;
		if ((tcp4ClientEvent->error () == 0) && (tcp4ClientEvent->size () != 0))
			return;
		break;
	default:
		if (false)
			cout << "proxy " << this << " event: tcp4 client default" << endl;
		break;
	}
	cout << "CONNECTION BROKEN" << endl;
	Send (m_task, new MpxProxyTaskEvent (MpxProxyTaskEvent::ReasonConnectionBroken), true);
	Dispose (false);
}

} // namespace mpx
