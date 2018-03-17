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

#include "mpx-core/MpxUtilities.h"
#include <mpx-events/MpxEventBase.h>
#include <mpx-tasks/mpx-messages.h>

namespace mpx
{
typedef void* mpx_appdt_t;	//!< general pointer representing additional application data
typedef void* mpx_evndes_t;	//!< general pointer representing I/O handler associated with I/O call-back function

/*! Event handler macro
 *
 * Events, triggered by MPX environment, must be handled in some way. Event handlers are
 * functions defined by objects which are interested to handle them. Preferred way to
 * define these functions is to use this macro. This macro defines two functions which
 * should be used to handle events of some type. First one is static member function
 * and is implemented by this macro. Its goal is to call other non-static member function,
 * which is just declared by this macro. This function must implement event handler for
 * events of some specific type.
 *
 * Since this macro defines member functions of some class it should be used only inside
 * class declarations.
 *
 * Macro has these parameters:
 * @param x - function name for both functions, which have different prototypes
 * @param y - class name of subclass implementing MpxTaskBase interface. If this class
 * does not implement MpxTaskBase interface, these functions should not be used in event
 * registration procedures
 *
 * Static member function is obviously used in event handler registration procedures and
 * is rarely used explicitly, since it is called by MPX environment which triggers events.
 * Phrase 'triggering event' is synonym for the act of calling function registered to
 * handle that event. Events which have no registered event handlers are never triggered.
 *
 * Function has two parameters, both provided by MPX environment:
 * @param event - reference to object implementing MpxEventBase. Concrete subclass of
 * that object should be deduced by dynamically casting this reference
 * @param appdata - this data is provided by application in the call of registration
 * procedure for specific event. Since this parameter is dynamically casted by non-static
 * member function defiined by this macro to subclass implementing MpxTaskBase, it must
 * be reference to the object implementing this same interface. This reference is used
 * to call the second non-static member function defined by this macro.
 *
 * The second, nonstatic member function, has only one parameter:
 * @param event - reference to object implementing MpyEventBase interface. This reference
 * is provided by its static member function counterpart when called bay MPX environment, which
 * is almost always the case.
 */
#define	mpx_event_handler(x,y) \
inline	static	void	x (MpxEventBase *event, mpx_appdt_t appdata) \
{ \
	(dynamic_cast < y* > ((MpxTaskBase*) appdata))->x (event); \
} \
void	x (MpxEventBase *event);

/*! Event registration key
 *
 * Event handling functions or event call-back functions (named so because they are
 * provided by application and later called back bay MPX framework) are registered
 * using structured keys. These keys are used to store and retrieve them to or from
 * the special dictionary used by registration object. This dictionary can be
 * observed like some kind of matrix with state numbers in columns and event numbers
 * in rows (or vice versa). Fields of this matrix are filled with call-back function
 * references. Every function reference can be found given (state, event) combination.
 * This combination is thus some kind of key which enables store and retrieve operations
 * on that matrix. Since there can be large number of empty fields in this matrix
 * (especially when there are many states and events and just a few valid (state,
 * event) combinations), this dictionary is represented with special STL object containing
 * just as many fields as there are valid (state, event) combinations.
 *
 * These keys are composed of two integers representing (state, event) combination:
 * @param state - an integer representing state in which an event should trigger
 * @param event - an integer representing event code of event which should trigger
 * in given state
 */
typedef pair <u_int, u_int> evnkey;

/*! class representing 'less then' relation on the set of event call-back function keys
 *
 * function, defined in this class, defines 'total order' on the set of keys used to
 * manipulate with event call-back functions. Function is defined so that for any two
 * different keys k1 and k2 keys from that set one of these relations hold: k1 < k2 or
 * k2 < k1. This is not so obvious as it sounds, since there are sets which are not
 * totaly ordered: they contain different elements k1 and k2 for which both k1 < k2
 * and k2 < k1 does not hold.
 *
 * 'less than' relation implemented by this class is defined as follows: given two keys
 * x and y, x is 'less than' y (x < y) if and only if: integer representing x's state
 * code is less than an integer representing  y's state code or, when these integers
 * are equal, if integer representing x's event code is less than integer representing
 * y's event code.
 *
 * This relation is used by dictionary to insert and retrieve call-back function
 * references
 */
class evncmp: public less <evnkey>
{
public:
	bool operator() (const evnkey& __x, const evnkey& __y) const
	{
		return (__x.first < __y.first) || ((__x.first == __y.first) && (__x.second < __y.second));
	}
};

typedef void* tskmpx_t; //< general type reference for task multiplexer objects

/*! Common state codes
 *
 * this enumeration defines some common codes used to represent states
 * occuring in all tasks:
 *
 * InvalidState: 0 coded state
 * StartState: initial state of any task
 * AnyState: used to fire an event regardless of task state
 */
enum MpxTaskState
{
	InvalidState, StartState = (u_int) -1, AnyState = (u_int) -2,
};

/*! event callback function prototype
 *
 * @param event - reference to an event object being fired. Concrete event reference
 * should be find with dynamically casting this reference
 * @param appdata - reference to object representing task which had been delivered
 * event referenced in first parameter
 */
typedef void (*evnfunc) (MpxEventBase *event, mpx_appdt_t appdata);

/*! structure representing triple (state, event, callback)
 *
 *  this triple is useful to create large lists of just that triples, which are
 *  further used to register large number of event callback functions for given
 *  task with just one function call
 *
 *  Every triple consists of:
 *  @param stateCode - an integer representing state (should also be common state code)
 *  @param eventCode - an integer representing event
 *  @param f - reference to callback function which should be called (triggered) by MPX
 *  environment when an event with event code equal to eventCode has been detected in
 *  state with state code equal to stateCode for task which registered this callback
 *  reference for (stateCode, eventCode) combination
 */
struct EventDescriptor
{
	unsigned int stateCode; //< state code
	unsigned int eventCode; //< event code
	evnfunc f; //< callback function reference
};

/*! superclass for all MPX task classes
 *
 *  MPX task is object with basically very simple functionality. It implicitly
 *  iterates through an infinite loop doing nothing but invoking event callback
 *  functions. Period! This simple functionality can, however implement very
 *  complex goal defined with combinations of states, events and callback functions
 *  registered to achieve this goal. Every task begins this goal's implementation
 *  by handling start event in start state. This event is always trigerred by MPX
 *  environment and need not be registered to handle it. To handle it, class
 *  implementing some MPX task, must implement StartTask() method required by this
 *  superclass. This method is in fact start event callback function and must
 *  implement initial processing to achieve desired goal.
 *
 *  Every MPX task should use any number of states and events. It can register
 *  call-back functions for just any combination of these states and events. Every
 *  MPX task should also implement other (non callback) functions, but these functions
 *  should be used only as subroutines called within callback functions.
 *
 *  Every callback function (also special handler StartTask()method) should take
 *  responsibility for the following facts:
 *  - it must implement the piece of functionality provided for current combination
 *  of state and event
 *  - it should send any number of events to itself or to another tasks in
 *  the system. Tasks which do not send events tend to cause the death or deadlock
 *  of the system.
 *  - it should change the state of task. Some tasks or even the whole system are
 *  sometimes stateless so that they always stay in start state. They just register
 *  events for state AnyState and care only for delivered events.
 *
 *  Sending events makes implicit iterations of event callback function invocations
 *  mentioned in first paragraph. There is no while or some other loop to make explicit
 *  iterations of this functions. That is why it is so important that tasks generate
 *  events in callback functions. Every iteration (callback function) must prepare
 *  invocation (send event) of another iteration (callback function invocation in
 *  self or another task)
 *
 *  There is also another event which need not be registered but is sent to every task
 *  by MPX environment. This event is stop event. After that event is sent (not triggered)
 *  no other event can be triggered any more, even events which were sent before stop
 *  event but have not yet been delivered. Class implementing MPX task must implement
 *  function StopTask() to handle this event.
 *
 *  Functions StartTask() and StopTask() are only functions which must be implemented by
 *  class implementing MPX task. This class is in fact nothing but a long list of
 *  callback functions which implement MPX task's goal.
 *
 *  In summary, functionalities exposed by this class are:
 *  - event callback functions registration
 *  - event callback functions retrieval
 *  - sending events to any task in the system and beyond
 *  - retrieving useful informations or references to important objects in the system
 *  like: current state of the task, reference to the owning task multiplexer, task
 *  name, geting current time
 *  - one important functionality, not mentioned yet, are timers. Any task can start
 *  any number of them.
 *  - another important functionality is retrieval of tasks acros the system boundary:
 *  task residing in another process in the same machine or even on another machine
 *  in the network
 *
 */
class MpxTaskBase
{
public:
	typedef map <evnkey, evnfunc, evncmp> evnset;
public:
	MpxTaskBase (evnset& e, const char* name = 0);
	MpxTaskBase (const char* name = 0);
	virtual ~MpxTaskBase ();
	void Dispose (bool release);

	static evnset& CreateEventSet (EventDescriptor eventTab[]);
	void RegisterEventHandlers (EventDescriptor evntab[]);
	evnfunc RegisterEventHandler (unsigned int stateCode, unsigned int eventCode, evnfunc f);
	evnfunc RetrieveEventHandler (u_int state, u_int event);
	int Send (MpxTaskBase* task, MpxEventBase* event, bool invoke = false);
	virtual int HandleEvent (MpxEventBase* event);
	int RetrieveExternalTask (const char* connString, const char* encdeclib = "mpx-ed-lib");

	/*! event handler for start event
	 *
	 * this function must be implemented by the subclass in the same manner as other
	 * event callback functions are. The only difference between this and other event
	 * handlers is, that this function does not have parameter event, since it is not
	 * needed, because it would be equal to start event instance.
	 */
	virtual void StartTask () = 0;

	/*! event handler for stop event
	 *
	 * this function must be implemented by the subclass in the same manner as other
	 * event callback functions are. The only difference between this and other event
	 * handlers is, that this function does not have parameter event, since it is not
	 * needed, because it would be equal to stop event instance.
	 */
	virtual void StopTask () = 0;

	/*! retrieve current state of the task
	 *
	 * @return **state** - an integer representing current state of the task
	 */
	inline MpxTaskState state ()
	{
		return m_state;
	}

	/*! set new state of the task
	 *
	 * @param state an integer representing new state of the task
	 */
	inline void state (MpxTaskState state)
	{
		m_state = state;
	}

	/*! retrieve reference of owning task multiplexer
	 *
	 * This reference should be used to exploit the functionality of the task multiplexer
	 * which owns this task. It should be also used to further retrieve a reference of
	 * object which implements I/O multiplexing for owning task multiplexer, to exploit
	 * lower level I/O multiplexing of the system
	 *
	 * @return opaque reference of task multiplexer, owner of this task
	 */
	inline tskmpx_t mpx ()
	{
		return m_mpx;
	}

	/*! set task multiplexer reference
	 *
	 * @param mpx - reference to task multiplexer, which will become owner of the task
	 */
	inline void mpx (tskmpx_t mpx)
	{
		m_mpx = mpx;
	}

	/*! retrieve name of the task
	 *
	 * @return string reference to task name
	 */
	inline const char* name ()
	{
		return m_name.c_str ();
	}
	struct timespec GetCurrentTime ();
	void* StartTimer (struct timespec timerStamp, void* ctx = 0);
	void StopTimer (void* timer);

	/*! enable event sending
	 *
	 */
	static inline void EnableSend ()
	{
		g_enableSend = true;
	}

	/*! disable event sending
	 *
	 */
	static inline void DisableSend ()
	{
		g_enableSend = false;
	}
	static inline int sentCount () { return g_sentCount; }

private:
	int RetrieveExternalTaskLocal (const char* connString, const char* encdeclib);
	int RetrieveExternalTaskTcp4 (const char* connString, const char* encdeclib);
	int RetrieveExternalTaskTcp6 (const char* connString, const char* encdeclib);

	mpx_event_handler(StartEventHandler, MpxTaskBase)
	mpx_event_handler(StopEventHandler, MpxTaskBase)

private:
	static const char* g_protocolField; //!< parsing field: protocol
	static const char* g_protocolLocal; //!< parsing field: local
	static const char* g_protocolTcp4; //!< parsing field: tcp4
	static const char* g_protocolTcp6; //!< parsing field: tcp6
	static const char* g_pathField; //!< parsing field: path
	static const char* g_portField; //!< parsing field: port
	static const char* g_nameField; //!< parsing field: name
	static const char* g_hostnameField; //!< parsing field: hostname

private:
	static EventDescriptor g_evntab[]; //!< table containing descriptions of start and stop events
	static int g_sentCount; //!< number of events sent by all tasks in the system
	static bool g_enableSend; //!< enables sending of events
	tskmpx_t m_mpx; //!< reference to owning task multiplexer
	string m_name; //!< task name
	MpxTaskState m_state; //!< current state of the task
	evnset& m_evnset; //!< reference to event callback dictionary
	evnset m_locevnset; //!< internal event callback dictionary
	bool m_extset; //! callback dictionary provided by subclass
};

} /* namespace mpx */
