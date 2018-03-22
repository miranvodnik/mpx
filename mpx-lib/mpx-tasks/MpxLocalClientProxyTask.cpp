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
#include <mpx-working-threads/MpxWorkingQueue.h>
#include <mpx-jobs/MpxJobs.h>

namespace mpx
{

MpxLocalClientProxyTask::MpxLocalClientProxyTask (MpxTaskBase* task, const char* encdeclib, MpxLocalClient* localClient) :
	MpxProxyTask (task, localClient, encdeclib, true)
{
	localClient->task (this);
}

MpxLocalClientProxyTask::~MpxLocalClientProxyTask ()
{
}

void MpxLocalClientProxyTask::HandleSocketEvent (MpxEventBase *event)
{
	MpxLocalClientEvent* localClientEvent = dynamic_cast <MpxLocalClientEvent*> (event);
	if (localClientEvent == 0)
		return;

	MpxLocalClient* localClient = reinterpret_cast <MpxLocalClient*> (localClientEvent->endPoint ());
	if (localClient == 0)
		return;

	switch (localClientEvent->flags ())
	{
	case EPOLLIN:
		if ((localClientEvent->error () == 0) && (localClientEvent->size () != 0))
		{
			MpxEventBase* event;
			while ((event = localClient->DecodeEvent(m_eventXDR)) != 0)
				Send (m_task, event, false);
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
