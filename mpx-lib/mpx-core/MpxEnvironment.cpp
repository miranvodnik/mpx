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

#include <libxml/parser.h>

#include <mpx-core/MpxEnvironment.h>
#include <mpx-event-queues/MpxLocalMQTask.h>
#include <mpx-working-threads/MpxWorkerTask.h>
#include <mpx-events/MpxEvents.h>

namespace mpx
{

/*! @brief mpx-lib one and only one environment object
 *
 */

MpxEnvironment* MpxEnvironment::g_mpxEnvironment = new MpxEnvironment ();

/*! @brief mpx-lib environment object constructor
 *
 * this constructor initializes m_lock mutex used
 * to synchronize concurrent access to environment
 * data structures
 */

MpxEnvironment::MpxEnvironment ()
{
	pthread_mutexattr_t attr;

	pthread_mutexattr_init (&attr);
	pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&m_lock, &attr);

	m_barrierActive = false;
}

/*! @brief destructor of mpx-lib environment object
 *
 */

MpxEnvironment::~MpxEnvironment ()
{
}

/*!
 *
 * @param configFile
 * @return
 */
int MpxEnvironment::_CreateEnvironment (const char* configFile)
{
	xmlDocPtr xmlDoc = xmlReadFile (configFile, 0, 0);
	if (xmlDoc == 0)
		return -1;

	int status = ParseConfigDoc (reinterpret_cast <void*> (xmlDoc));

	xmlFreeDoc (xmlDoc);
	return status;
}

int MpxEnvironment::ParseConfigDoc (void* xmlDoc)
{
	for (xmlNode* child = (reinterpret_cast <xmlDocPtr> (xmlDoc))->children; child != 0; child = child->next)
	{
		if (strcmp (reinterpret_cast <const char*> (child->name), "mpx") == 0)
		{
			if (ParseMpxNode (reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "text") == 0)
			continue;
		else
			return -1;
	}
	return 0;
}

int MpxEnvironment::ParseMpxNode (void* node)
{
	for (xmlNode* child = (reinterpret_cast <xmlNode*> (node))->children; child != 0; child = child->next)
	{
		if (strcmp (reinterpret_cast <const char*> (child->name), "common-settings") == 0)
		{
			if (ParseCommonSettingsNode (reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "specific-settings") == 0)
		{
			if (ParseSpecificSettingsNode (reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "text") == 0)
			continue;
		else
			return -1;
	}
	return 0;
}

int MpxEnvironment::ParseCommonSettingsNode (void* node)
{
	for (xmlNode* child = (reinterpret_cast <xmlNode*> (node))->children; child != 0; child = child->next)
	{
		if (strcmp (reinterpret_cast <const char*> (child->name), "environment") == 0)
		{
			if (ParseEnvironmentNode (reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "text") == 0)
			continue;
		else
			return -1;
	}
	return 0;
}

int MpxEnvironment::ParseSpecificSettingsNode (void* node)
{
	for (xmlNode* child = (reinterpret_cast <xmlNode*> (node))->children; child != 0; child = child->next)
	{
		if (strcmp (reinterpret_cast <const char*> (child->name), "task-multiplexers") == 0)
		{
			if (ParseTaskMultiplexersNode (reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "text") == 0)
			continue;
		else
			return -1;
	}
	return 0;
}

int MpxEnvironment::ParseEnvironmentNode (void* node)
{
	int mpxCount = -1;
	int workerCount = -1;

	for (xmlAttr* property = (reinterpret_cast <xmlNode*> (node))->properties; property != 0; property = property->next)
	{
		if (strcmp (reinterpret_cast <const char*> (property->name), "mpx-count") == 0)
		{
			mpxCount = atoi (reinterpret_cast <const char*> (property->children->content));
		}
		else if (strcmp (reinterpret_cast <const char*> (property->name), "worker-count") == 0)
		{
			workerCount = atoi (reinterpret_cast <const char*> (property->children->content));
		}
		else
			continue;
	}
	return 0;
}

int MpxEnvironment::ParseTaskMultiplexersNode (void* node)
{
	for (xmlNode* child = (reinterpret_cast <xmlNode*> (node))->children; child != 0; child = child->next)
	{
		if (strcmp (reinterpret_cast <const char*> (child->name), "task-multiplexer") == 0)
		{
			mpxStruct mpxStr;
			if (ParseTaskMultiplexerNode (mpxStr, reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "text") == 0)
			continue;
		else
			return -1;
	}
	return 0;
}

int MpxEnvironment::ParseTaskMultiplexerNode (mpxStruct& mpxStr, void* node)
{
	int index = -1;
	string name = "";
	stringstream connStr ("");
	int threadAffinity = -1;
	connectionString connStruct;

	for (xmlAttr* property = (reinterpret_cast <xmlNode*> (node))->properties; property != 0; property = property->next)
	{
		if (strcmp (reinterpret_cast <const char*> (property->name), "index") == 0)
		{
			index = atoi (reinterpret_cast <const char*> (property->children->content));
		}
		else if (strcmp (reinterpret_cast <const char*> (property->name), "name") == 0)
		{
			name = reinterpret_cast <const char*> (strdup (reinterpret_cast <const char*> (property->children->content)));
		}
		else if (strcmp (reinterpret_cast <const char*> (property->name), "connection-string") == 0)
		{
			connStr << reinterpret_cast <const char*> (strdup (reinterpret_cast <const char*> (property->children->content)));
		}
		else if (strcmp (reinterpret_cast <const char*> (property->name), "thread-affinity") == 0)
		{
			threadAffinity = atoi (reinterpret_cast <const char*> (property->children->content));
		}
		else
			return -1;
	}

	for (xmlNode* child = (reinterpret_cast <xmlNode*> (node))->children; child != 0; child = child->next)
	{
		if (strcmp (reinterpret_cast <const char*> (child->name), "connection-string") == 0)
		{
			string cStr;
			if (ParseConnectionStringNode (connStruct, reinterpret_cast <void*> (child)) != 0)
				return -1;
			if ((cStr = connStruct.CreateConnectionString()).empty())
				return -1;
			connStr << cStr;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "text") == 0)
			continue;
		else
			return -1;
	}

	connStr << ends;

	mpxStr.index = index;
	mpxStr.name = name;
	mpxStr.connStr = connStr.str();
	mpxStr.threadAffinity = threadAffinity;

	return 0;
}

int MpxEnvironment::ParseConnectionStringNode (connectionString& connStruct, void* node)
{
//	memset (&connStruct, 0, sizeof connStruct);
//	connStruct.protocol = "";
//	connStruct.address = "";
//	connStruct.path = "";

	for (xmlAttr* property = (reinterpret_cast <xmlNode*> (node))->properties; property != 0; property = property->next)
	{
		if (strcmp (reinterpret_cast <const char*> (property->name), "protocol") == 0)
		{
			connStruct.protocol = reinterpret_cast <const char*> (strdup (reinterpret_cast <const char*> (property->children->content)));
		}
		else if (strcmp (reinterpret_cast <const char*> (property->name), "address") == 0)
		{
			connStruct.address = reinterpret_cast <const char*> (strdup (reinterpret_cast <const char*> (property->children->content)));
		}
		else if (strcmp (reinterpret_cast <const char*> (property->name), "path") == 0)
		{
			connStruct.path = reinterpret_cast <const char*> (strdup (reinterpret_cast <const char*> (property->children->content)));
		}
		else if (strcmp (reinterpret_cast <const char*> (property->name), "port") == 0)
		{
			connStruct.port = atoi (reinterpret_cast <const char*> (property->children->content));
		}
		else
			return -1;
	}

	for (xmlNode* child = (reinterpret_cast <xmlNode*> (node))->children; child != 0; child = child->next)
	{
		if (strcmp (reinterpret_cast <const char*> (child->name), "protocol") == 0)
		{
			if (ParseProtocolNode (connStruct.protocol, reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "address") == 0)
		{
			if (ParseAddressNode (connStruct.address, reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "path") == 0)
		{
			if (ParsePathNode (connStruct.path, reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "port") == 0)
		{
			if (ParsePortNode (connStruct.port, reinterpret_cast <void*> (child)) != 0)
				return -1;
		}
		else if (strcmp (reinterpret_cast <const char*> (child->name), "text") == 0)
			continue;
		else
			return -1;
	}
	return 0;
}

int MpxEnvironment::ParseProtocolNode (string& protocol, void* node)
{
	protocol = "";

	for (xmlAttr* property = (reinterpret_cast <xmlNode*> (node))->properties; property != 0; property = property->next)
	{
		if (strcmp (reinterpret_cast <const char*> (property->name), "value") == 0)
		{
			protocol = reinterpret_cast <const char*> (strdup (reinterpret_cast <const char*> (property->children->content)));
		}
		else
			return -1;
	}
	return 0;
}

int MpxEnvironment::ParseAddressNode (string& address, void* node)
{
	address = "";

	for (xmlAttr* property = (reinterpret_cast <xmlNode*> (node))->properties; property != 0; property = property->next)
	{
		if (strcmp (reinterpret_cast <const char*> (property->name), "value") == 0)
		{
			address = reinterpret_cast <const char*> (strdup (reinterpret_cast <const char*> (property->children->content)));
		}
		else
			return -1;
	}
	return 0;
}

int MpxEnvironment::ParsePathNode (string& path, void* node)
{
	path = "";

	for (xmlAttr* property = (reinterpret_cast <xmlNode*> (node))->properties; property != 0; property = property->next)
	{
		if (strcmp (reinterpret_cast <const char*> (property->name), "value") == 0)
		{
			path = reinterpret_cast <const char*> (strdup (reinterpret_cast <const char*> (property->children->content)));
		}
		else
			return -1;
	}
	return 0;
}

int MpxEnvironment::ParsePortNode (int& port, void* node)
{
	port = -1;

	for (xmlAttr* property = (reinterpret_cast <xmlNode*> (node))->properties; property != 0; property = property->next)
	{
		if (strcmp (reinterpret_cast <const char*> (property->name), "value") == 0)
		{
			port = atoi (reinterpret_cast <const char*> (property->children->content));
		}
		else
			return -1;
	}
	return 0;
}

string MpxEnvironment::connectionString::CreateConnectionString()
{
	stringstream connStr("");

	if (protocol.empty())
		return "";
	if (protocol.compare("local") == 0)
	{
		if (path.empty())
			return 0;
		connStr << "protocol:local,path:" << path << ";" << ends;
		return connStr.str();
	}
	if ((protocol.compare("tcp4") == 0) || (protocol.compare("tcp6") == 0))
	{
		if (address.empty())
			return 0;
		if (port < 0)
			return 0;
		connStr << "protocol:" << protocol << ",address:" << address << ",port:" << port << ";" << ends;
		return connStr.str();
	}
	return "";
}

/*! @brief create new task multiplexer
 *
 * this function creates new instance of MpxTaskMultiplexer class. its
 * reference is saved into internal data structure m_tskmpxset, set of
 * task multipexing objects. Access to this set is synchronized to
 * prevent concurrent write operations on it. Every instance of task
 * multiplexer is associated with unique pipe object used to properly
 * terminate its execution. Execution termination is further described
 * in description of MpxTaskMultiplexer class itself
 *
 * @param connStr connection string parameter which is transfered to
 * MpxTaskMultiplexer unchanged and it is not examined by this function
 * @param isWorker working thread indicator transferred to MpxTaskMultiplexer
 * constructor
 * @return **0** - multiplexer cannot be created: cannot create associated pipe or
 * cannot create new object
 * @return **other** - reference to new [MpxTaskMultiplexer] object
 */

MpxTaskMultiplexer* MpxEnvironment::_CreateTaskMultiplexer (const char* connStr, bool isWorker)
{
	int fd [2];
	MpxTaskMultiplexer * mpx = 0;

	pthread_mutex_lock (&m_lock);
	if (pipe (fd) == 0)
	{
		mpx = new MpxTaskMultiplexer (fd, connStr, isWorker);
		if (mpx != 0)
			m_tskmpxset.insert (mpx);
	}
	pthread_mutex_unlock (&m_lock);

	return mpx;
}

/*! @brief starting routine of mpx-lib environment
 *
 * The function works following these steps:
 * - first, create some working threads that perform havy and long-lasting jobs
 * - it then sets up two barriers that prevent the registered tasks to start too erly
 * - finally , all the multiplexers that were registered in a given environment are started.
 * Each of them is associated with a waiting queue task (parameter mqTask), which will allow
 * the flow of events between all tasks they contain
 *
 * When all this is done, it waits for all barriers to be dropped and quits.
 *
 * @param mqTask reference to task implementing waiting queue for sending events.
 * This task is essentially the same as usual one, but it has a little more widespread
 * functionality: it must know how to send events and it has to know how to clone itself.
 * This parameter is really just a reference. Each multiplexer creates its own clone of
 * waiting queue using this reference.
 * @return **0** - return code is not important
 */
int MpxEnvironment::_Start (MpxMQTaskI* mqTask)
{
	for (int i = 0; i < 4; ++i)
	{
		MpxTaskMultiplexer* mpx = MpxEnvironment::CreateTaskMultiplexer (0, true);
		MpxWorkerTask* task = new MpxWorkerTask ();
		mpx->RegisterTask (task);
	}

	pthread_mutex_lock (&m_lock);
	m_barrierActive = true;
	pthread_barrier_init (&m_barrier, 0, m_tskmpxset.size () + 1);
	pthread_barrier_init (&m_tbarrier, 0, m_tskmpxset.size () + 1);

	for (tskmpxset::iterator it = m_tskmpxset.begin (); it != m_tskmpxset.end (); ++it)
	{
		(*it)->mqImplTask (mqTask);
		(*it)->Start ();
		m_thrset.insert ((*it)->getHandle ());
	}

	pthread_mutex_unlock (&m_lock);
	pthread_barrier_wait (&m_barrier);
	pthread_barrier_wait (&m_tbarrier);
	m_barrierActive = false;
	return 0;
}

/*! lowering the barrier for the operation of multiplexers
 *
 * when the multiplexer is initialized, it must lower the barrier for their operation.
 * Only when the barrier is completely lowered, each of them can start to function genuinely.
 * @return **0** - return value is not important
 */
int MpxEnvironment::_WaitMultiplexerBarrier ()
{
	pthread_mutex_lock (&m_lock);
	if (!m_barrierActive)
	{
		pthread_mutex_unlock (&m_lock);
		return -1;
	}
	pthread_mutex_unlock (&m_lock);
	pthread_barrier_wait (&m_barrier);
	return 0;
}

/*! lowering the barrier for the operation of tasks
 *
 * when the multiplexer is initialized, it must also lower another the barrier to
 * enable its operation. This barrier sholud be lowered when all tasks are started.
 * Only when the barrier is completely lowered, each of them can start to function genuinely.
 * @return **0** - return value is not important
 */
int MpxEnvironment::_WaitTasksBarrier ()
{
	pthread_mutex_lock (&m_lock);
	if (!m_barrierActive)
	{
		pthread_mutex_unlock (&m_lock);
		return -1;
	}
	pthread_mutex_unlock (&m_lock);
	pthread_barrier_wait (&m_tbarrier);
	return 0;
}

/*! stop executing multiplexing environment
 *
 * The essence of this function is how to stop all the multiplexers. The procedure
 * is as follows:
 * - first, sending of events is disabled
 * - Then, the Stop () function of each of them is executed
 * - Immediately thereafter, it is waited for each of them to complete execution.
 * - When this is completed, each of them is destroyed.
 *
 * All together, it is running synchronously with other requirements for accessing
 * data that relate to the environment in which multiplexers operate.
 */
void MpxEnvironment::_Stop ()
{
	pthread_mutex_lock (&m_lock);

	MpxTaskBase::DisableSend ();

	for (tskmpxset::iterator it = m_tskmpxset.begin (); it != m_tskmpxset.end (); ++it)
		(*it)->Stop ();

	for (thrset::iterator it = m_thrset.begin (); it != m_thrset.end (); ++it)
		pthread_join (*it, 0);

	for (tskmpxset::iterator it = m_tskmpxset.begin (); it != m_tskmpxset.end (); ++it)
		delete (*it);

	m_tskmpxset.clear ();

	pthread_mutex_unlock (&m_lock);
}

/*! Wait all task multiplexers to terminate
 *
 * function iterates through set of task multiplexers and waiting one by one to terminate
 * using system function pthread_join()
 */
void MpxEnvironment::_Wait ()
{
	for (thrset::iterator it = m_thrset.begin (); it != m_thrset.end (); ++it)
		pthread_join (*it, 0);
}

/** Deliver an event to all external tasks in MPX environment
 *
 * Every task multiplexer (MpxTaskMultiplexer instance) has exactly one external task
 * (MpxExternalTask instance). This function broadcasts any event (its copy) to just
 * these tasks. This feature is usually used when it is necessary to find a task
 * which is located in the environment. Because we do not know in which multiplexer
 * it is registered, the search request is multiplicated and sent to every 'external' task.
 *
 * @param task sender task reference
 * @param event event to be sent
 * @return **count** - number of events broadcasted to 'external' tasks
 */
int MpxEnvironment::_BroadcastExternalEvent (MpxTaskBase* task, MpxEventBase* event)
{
	int count = 0;

	pthread_mutex_lock (&m_lock);

	for (tskmpxset::iterator it = m_tskmpxset.begin (); it != m_tskmpxset.end (); ++it, ++count)
	{
		task->Send ((*it)->externalTask (), event->Copy ());
	}
	delete event;

	pthread_mutex_unlock (&m_lock);

	return count;
}

} /* namespace mpx */
