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

#include <pthread.h>
#include <map>
using namespace std;

#include <mpx-core/MpxTaskMultiplexer.h>
#include <mpx-core/MpxUtilities.h>
#include <mpx-event-queues/MpxMQTaskI.h>

namespace mpx
{

/*! MPX environment
 *
 * This class represents environment in which all the multiplexers for
 * event-controlled tasks are located. Instances of this class can not be
 * created in the normal way because their constructor is private. However,
 * a global instance is defined in this class, created by the running
 * environment.The functionality implemented by this class is accessed
 * through this instance. This is:
 * - creating environment: creating environment data structures and task
 * multiplexers
 * - starting MPX environment: starting all task multiplexers
 * - stoping MPX environment: stoping task multiplexers
 * - broadcasting 'external task' events
 *
 * These are data structures used to implement MPX environment functionality:
 * - The most important among them is a collection of multiplexers. All other
 * data serve to make this collection easier to handle.
 * - set of thread handles associated with multiplexers
 * - synchronization mechanisms: locks and barriers
 */
class MpxEnvironment
{
private:
	typedef set <MpxTaskMultiplexer*> tskmpxset;
	typedef set <pthread_t> thrset;
	MpxEnvironment ();
	virtual ~MpxEnvironment ();
public:

	inline static int CreateEnvironment (const char* configFile)
	{
		return g_mpxEnvironment->_CreateEnvironment (configFile);
	}
	/*! Create new instance of task multiplexer
	 *
	 * this function mimics [_CreateTaskMultiplexer](@ref _CreateTaskMultiplexer)
	 *
	 * @param connStr connection string parameter - its default value is 0: this
	 * multiplexer will not listen to external events
	 * @param isWorker working thread indicator - its default value is false: this
	 * multiplexer is not working thread invoking havy and long-lasting jobs
	 * @return **null** - task multiplexer cannot be created
	 * @return **other** - reference to new task multiplexer
	 */
	inline static MpxTaskMultiplexer* CreateTaskMultiplexer (const char* connStr = 0, bool isWorker = false)
	{
		return g_mpxEnvironment->_CreateTaskMultiplexer (connStr, isWorker);
	}

	/*! Start MPX environment
	 *
	 * it mimics [_Start function](@ref _Start)
	 *
	 * @param mqTask reference to waiting queue task which will be multipicated
	 * by every instance of task multiplexer started within this environment
	 * @return **0** - return code is not important
	 */
	inline static int Start (MpxMQTaskI* mqTask)
	{
		return g_mpxEnvironment->_Start (mqTask);
	}

	/*! Lower multiplexer barrier by one unit
	 *
	 * function mimics [_WaitMultiplexerBarrier](@ref _WaitMultiplexerBarrier)
	 *
	 * @return **0** - return code is not important
	 */
	inline static int WaitMultiplexerBarrier ()
	{
		return g_mpxEnvironment->_WaitMultiplexerBarrier ();
	}

	/*! Lower task barrier by one unit
	 *
	 * Function mimics [_WaitTasksBarrier](@ref _WaitTasksBarrier)
	 * @return **0** - return code is not important
	 */
	inline static int WaitTasksBarrier ()
	{
		return g_mpxEnvironment->_WaitTasksBarrier ();
	}

	/*! Stop executing all multiplexers
	 *
	 * All multiplexers (task multiplexers or havy job working threads) are
	 * sent termination request (broken pipe or cancellation trgger). Function
	 * mimics [_Stop](@ref _Stop)
	 *
	 */
	inline static void Stop ()
	{
		g_mpxEnvironment->_Stop ();
	}

	/*! Wait MPX environment until it terminates
	 *
	 * Function waits until all multiplexers within MPX environment terminate
	 *
	 */
	inline static void Wait ()
	{
		g_mpxEnvironment->_Wait ();
	}

	/*! Broadcast 'external event'
	 *
	 * Function broadcasts copies of any event to all 'external tasks' in
	 * task multiplexers.
	 *
	 * @param task sending task reference
	 * @param event event reference
	 * @return **0** - return code is not important
	 */
	inline static int BroadcastExternalEvent (MpxTaskBase* task, MpxEventBase* event)
	{
		return g_mpxEnvironment->_BroadcastExternalEvent (task, event);
	}
private:
	typedef struct
	{
		string protocol;
		string address;
		string path;
		int port;
		string CreateConnectionString ();
	} connectionString;
	typedef struct
	{
		int index;
		string name;
		string connStr;
		int threadAffinity;
	} mpxStruct;
private:
	int _CreateEnvironment (const char* configFile);
	int ParseConfigDoc (void* doc);
	int ParseMpxNode (void* node);
	int ParseCommonSettingsNode (void* node);
	int ParseSpecificSettingsNode (void* node);
	int ParseEnvironmentNode (void* node);
	int ParseTaskMultiplexersNode (void* node);
	int ParseTaskMultiplexerNode (mpxStruct& mpxStr, void* node);
	int ParseConnectionStringNode (connectionString& connStr, void* node);
	int ParseProtocolNode (string& protocol, void* node);
	int ParseAddressNode (string& address, void* node);
	int ParsePathNode (string& path, void* node);
	int ParsePortNode (int& port, void* node);
	MpxTaskMultiplexer* _CreateTaskMultiplexer (const char* connStr, bool isWorker);
	int _Start (MpxMQTaskI* mqTask);
	int _WaitMultiplexerBarrier ();
	int _WaitTasksBarrier ();
	void _Stop ();
	void _Wait ();
	int _BroadcastExternalEvent (MpxTaskBase* task, MpxEventBase* event);
private:
	static MpxEnvironment* g_mpxEnvironment; //!< reference to one and only one MPX environment object
	tskmpxset m_tskmpxset; //!< set of all multiplexers within MPX environment
	thrset m_thrset; //!< set of physical threads associated wizh multiplexers
	pthread_mutex_t m_lock; //!< locking mutex for access synchronization
	pthread_barrier_t m_barrier; //!< multiplexers barrier
	pthread_barrier_t m_tbarrier; //!< tasks barrier
	bool m_barrierActive;
};

} /* namespace mpx */
