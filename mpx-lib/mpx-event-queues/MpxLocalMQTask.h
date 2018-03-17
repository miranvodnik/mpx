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

#pragma once

#include <mpx-core/MpxUtilities.h>
#include <mpx-event-queues/MpxMQTaskI.h>
#include <mpx-events/MpxLocalClientEvent.h>
#include <mpx-events/MpxLocalEndPointEvent.h>
#include <mpx-events/MpxLocalListenerEvent.h>
#include <mpx-events/MpxStartEvent.h>
#include <mpx-events/MpxStopEvent.h>
#include <mpx-sockets/MpxLocalClient.h>
#include <mpx-sockets/MpxLocalEndPoint.h>
#include <mpx-sockets/MpxLocalListener.h>
using namespace mpx;

#include <map>
using namespace std;

namespace mpx
{

/*! waiting queue implementation using local (UNIX domain) sockets
 *
 * All waiting queue used in MPX environment implement similar
 * functionality using different transport mechanisms. This class
 * implements it using UNIX domain sockets. Every task multiplexer has
 * its own waiting queue. This waiting queue is used to convey events
 * between tasks belonging to the same task multiplexer, between tasks
 * belonging to different task multiplexers or even crossing the process
 * and machine boundary. To achieve this qoal, waiting queues need to be
 * connected. Connections between them are unix domain sockets. They
 * represent queues implemented by operating system. Every waiting queue
 * should establish many sockets. This means that every implementation
 * of waiting queue contains many 'queues' provided by operating system.
 *
 * Waiting queues must therefore provide these functionalities:
 * - they must provide queuing mechanisms to convey events between tasks
 * - they must provide connection mechanisms to connect them together
 *
 * Since waiting queues need to communicate with each other, and since
 * MPX environment is an environment which is provided exactly for that
 * matter (communication between different tasks), waiting queues are
 * themselves implemented as special MPX tasks which enable sending events
 * between all tasks in the system.
 *
 * Waiting queues are thus implemented so that they implement MpxMQTaskI
 * interface which in turn implements basic task interface MpxTaskBase.
 *
 * Since it subclasses MpxTaskBase, this class implements StartTask() and StopTask()
 * methods in a nontrivial way.
 *
 * Since it subclasses MpxMQTaskI interface, it implements additional functions
 * Copy() and MQSend() required by that interface
 *
 * There are some important things to remember about waiting queues implemented
 * with this class:
 * - the central part of the queue is connection point of the queue in
 * the form of local UNIX domain listening socket, which is implemented by
 * MpxLocalListener. Queue's name is stored in MpxEventQueueRepository. It
 * is accessible through the key equal to reference of task multiplexer
 * owning this queue. Starting routine of task implementing this queue
 * creates UNIX domain socket listening on that connection point. Every
 * entity which want to send events to tasks belonging to task multiplexer
 * owning that queue, should first retrieve the name of this connection
 * point from MpxEventQueueRepository using reference to task multiplexer
 * as a search key, then posting connection request to this connection
 * point, and finally send desired event through communication channel
 * created by successful connection attempt. This holds for every task,
 * even if one want to send an event between tasks belonging to the same
 * task multiplexer.
 * - every waiting queue contains two sets of communication channels
 * created by many different successful connection attempts between queues.
 * Every successful connection attempt between two waiting queues, creates
 * two communication channels. One is created in the queue which posts
 * successful connection attempt. This communication channel is an instance
 * of MpxLocalEndPoint and is saved in the end-point set of originating
 * waiting queue. Another one is created on the terminating side by the
 * listening UNIX domain socket, which accepts connection attempt. It is
 * implemented by an instance of MpxLocalClient and is saved in the client
 * set of the terminating waiting queue.
 * - communication channels created by successful connection attempt are peers
 * of full-duplex unix domain sockets, Every one of them should be used to
 * send and receive events. But this implementation does not exploit this
 * possibility. End-point peer is always used to send events, client peer
 * is always used to only receive them. So, if wee want that two queues
 * communicate bidirectionally, two UNIX domain sockets must be created, one
 * conveying events in one direction, other conveying them in other direction.
 * - in summary: if two tasks belonging to task multiplexer owning waiting
 * queue A want to send events between them, one unix domain socket is needed
 * to do that, since communication diagram looks like that: A --> A, which
 * is equivalent to A <-- A. If tasks belong to different task multiplexers,
 * owning queues, say A and B, two sockets are needed, one for conveying
 * events from A to B (A --> B) and other to convey events from B to A
 * (B --> A), although it would be quite enough to create just one socket,
 * since sockets are full-duplex (A <--> B).
 */
class MpxLocalMQTask: public mpx::MpxMQTaskI
{
public:
	typedef set <MpxLocalEndPoint*> epset;
	typedef map <void*, MpxLocalClient*> clnset;
	MpxLocalMQTask ();
	virtual ~MpxLocalMQTask ();
	virtual void StartTask ();
	virtual void StopTask ();
	virtual MpxMQTaskI* Copy (tskmpx_t mpx);
	virtual int MQSend (tskmpx_t mpx, MpxEventBase* event);
	virtual void UnlinkMQ (tskmpx_t mpx);
private:
	virtual MpxLocalClient* Connect (tskmpx_t mpx);
	void Release ();
private:
	/*! event handler used to establish communication link between waiting queues
	 *
	 * this handler intercepts connection requests made by other waiting queues
	 */
	mpx_event_handler (LocalListenerEventHandler, MpxLocalMQTask)

	/*! event handler used to send events
	 *
	 * events sent to task outside of task multiplexer owning waiting queue task,
	 * are handled by this callback
	 */
	mpx_event_handler (LocalEndPointEventHandler, MpxLocalMQTask)

	/*! event handler used to receive events
	 *
	 * events sent by tasks from other task multiplexers are handled by
	 * this callback
	 */
	mpx_event_handler (LocalClientEventHandler, MpxLocalMQTask)

private:
	/*!
	 *  event registration table for callback functions defined by this class
	 */
	static EventDescriptor g_evntab[];
	static evnset g_evnset;

	/*!
	 *  directory containing UNIX domain sockets used to construct full path names
	 *  of UNIX domain sockets used by an instance of this class
	 */
	static const char* g_localPath;

	/*! listening UNIX domain socket
	 */
	MpxLocalListener* m_listener;

	/*! set of UNIX domain end-point objects (instances of MpxLocalEndPoint)
	 *
	 * this set is composed of socket peers created due to successful connection
	 * attempts to other waiting queues in the system made by this waiting queue
	 */
	epset m_epset;

	/*! set of UNIX domain client objects (instances of MpxLocalClient)
	 *
	 * this set is composed of socket peers created due to successful connection
	 * attempts to this waiting queue made by other waiting queues in the system
	 */
	clnset m_clnset;
};

} /* namespace mpx */
