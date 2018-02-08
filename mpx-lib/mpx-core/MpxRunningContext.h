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

//
//       Header files
//

#include <sys/ioctl.h>	// select() API declarations
#include <sys/types.h>	// select() API declarations
#include <sys/syscall.h>
#include <sys/stat.h>	// select() API declarations
#include <sys/epoll.h>		// epoll() API declarations
#include <sys/select.h>	// select() API declarations
#include <fcntl.h>	// file control flags
#include <mqueue.h>	// posix message queues
#include <time.h>	// time declarations
#include <signal.h>	// signal handling
#include <unistd.h>	// std library
#include <stdlib.h>	// std library
#include <uuid/uuid.h>	// UUIDs (for unique MQ names)
#include <errno.h>	// error codes
#include <stdio.h>	// standard I/O functions
#include <string.h>	//
#include <mpx-core/MpxUtilities.h>

#include <string>	// STL strings
#include <map>		// STL keyed sets
#include <set>		// STL self keyed sets
#include <vector>	// STL vectors
#include <functional>	// function objects used by algorithms
#include <algorithm>	// algorithms for predicate functions
#include <iostream>	// stdio I/O operations
#include <sstream>	// stdio I/O operations

using namespace std;
// std:: is default namespace

namespace mpx
{

typedef void* ctx_appdt_t;	//!< general pointer representing additional application data
typedef void* ctx_fddes_t;	//!< general pointer representing I/O handler associated with I/O call-back function
typedef void* ctx_timer_t;	//!< general pointer representing timer handler associated with timer call-back function
typedef void* ctx_sig_t;	//!< general pointer representing signal handler associated with signal call-back function

#define	SEC_TO_NSEC	(1000L * 1000L * 1000L)	//!< number of nanoseconds in one second
/*! @brief display file name and line number
 *
 *  macro defines ctx_info string reference in the form
 *  of file-name : line-number when in debug mode or
 *  null pointer when not in debug mode
 */
#if (CONFIGURATION == Debug)
#define	_STR2(line) #line
#define	_STR(line) _STR2(line)
#define	ctx_info __FILE__ ":" _STR(__LINE__)
#else
#define	ctx_info 0
#endif

/*! @brief I/O multiplexer starter function
 *
 *  definition of two functions representing starting function
 *  for I/O multiplexer. First is static member function and
 *  does nothing but invokes its non-static member partner.
 *  Static member function is defined with this macro, while its
 *  non-static member partner is only declared by it.
 *
 *  Both functions have this parameter in common:
 *
 *  - ctx - reference to an instance of I/O multiplexer (or its
 *  subclass) implementation. Its class should implement this
 *  starter function
 *
 *  Static partner has an additional parameter:
 *
 *  - appdata - any reasonable value for this parameter is a
 *  reference to an instance of class implementing this function
 *  since I/O multiplexer provides it in the call of this function
 *  at the beginning of its main loop
 *
 *  @param x name of both functions
 *  @param y class implementing functions x(). It must be
 *  the name of class defining these functions otherwise the
 *  call of non-static member of function x() in the definition
 *  of its static partner won't work
 *
 */
#define	ctx_starter(x,y) \
inline	static	void	x (MpxRunningContext *ctx, ctx_appdt_t appdata) \
{ \
	((y*)appdata)->x (ctx); \
} \
void	x (MpxRunningContext *ctx);

/*! @brief I/O multiplexer end function
 *
 *  definition of two functions representing end function
 *  for I/O multiplexer. First is static member function and
 *  does nothing but invokes its non-static member partner.
 *  Static member function is defined with this macro, while its
 *  non-static member partner is only declared by it.
 *
 *  Both functions have this parameter in common:
 *
 *  - ctx - reference to an instance of I/O multiplexer (or its
 *  subclass) implementation. Its class should implement this
 *  end function
 *
 *  Static partner has an additional parameter:
 *
 *  - appdata - any reasonable value for this parameter is a
 *  reference to an instance of class implementing this function
 *  since I/O multiplexer provides it in the call of this function
 *  at the end of its main loop
 *
 *  @param x name of both functions
 *  @param y class implementing functions x(). It must be
 *  the name of class defining these functions otherwise the
 *  call of non-static member of function x() in the definition
 *  of its static partner won't work
 *
 */
#define	ctx_finisher(x,y) \
inline	static	void	x (MpxRunningContext *ctx, ctx_appdt_t appdata) \
{ \
	((y*)appdata)->x (ctx); \
} \
void	x (MpxRunningContext *ctx);

/*! @brief I/O multiplexer time hook function
 *
 *  definition of two functions representing time hook function
 *  for I/O multiplexer. First is static member function and
 *  does nothing but invokes its non-static member partner.
 *  Static member function is defined with this macro, while its
 *  non-static member partner is only declared by it.
 *
 *  Both functions have these parameters in common:
 *
 *  - ctx - reference to an instance of I/O multiplexer (or its
 *  subclass) implementation. Its class should implement this
 *  time hook function
 *
 *  - oldTime - time stamp of system time before time changed
 *
 *  - newTime - time stamp of system time after time changed
 *
 *  - timeDiff - difference between time stamps above expressed
 *  in nanoseconds
 *
 *  Static partner has an additional parameter:
 *
 *  - appdata - any reasonable value for this parameter is a
 *  reference to an instance of class implementing this function
 *  since I/O multiplexer provides it in the call of this function
 *  whenever time changes
 *
 *  @param x name of both functions
 *  @param y class implementing functions x(). It must be
 *  the name of class defining these functions otherwise the
 *  call of non-static member of function x() in the definition
 *  of its static partner won't work
 *
 */
#define	ctx_timehook(x,y) \
inline	static	void	x (MpxRunningContext *ctx, struct timespec oldTime, struct timespec newTime, long long timeDiff, ctx_appdt_t appdata) \
{ \
	((y*)appdata)->x (ctx, oldTime, newTime, timeDiff); \
} \
void	x (MpxRunningContext *ctx, struct timespec oldTime, struct timespec newTime, long long timeDiff);

/*! brief timer call-back function
 *
 *  definition of two functions representing timer call-back
 *  function. First is static member function and does nothing
 *  but invokes its non-static member partner. Static member
 *  function is defined with this macro, while its non-static
 *  member partner is only declared by it.
 *
 *  Both functions have these parameters in common:
 *
 *  - ctx - reference to an instance of I/O multiplexer (or its
 *  subclass) implementation. Its class should implement this
 *  time hook function
 *
 *  - handler - reference to timer handler - an instance of
 *  CTimer class associated with this call-back function
 *
 *  - t - time stamp of expiration time of this timer
 *
 *  Static partner has an additional parameter:
 *
 *  - appdata - any reasonable value for this parameter is a
 *  reference to an instance of class implementing this function
 *  since I/O multiplexer provides it in the call of this function
 *  at every timer expiration
 *
 *  @param x name of both functions
 *  @param y class implementing functions x(). It must be
 *  the name of class defining these functions otherwise the
 *  call of non-static member of function x() in the definition
 *  of its static partner won't work
 *
 */
#define	timer_handler(x,y) \
inline	static	void	x (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t, ctx_appdt_t appdata) \
{ \
	((y*) appdata)->x (ctx, handler, t); \
} \
void	x (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t);

/*! brief I/O call-back function
 *
 *  definition of two functions representing I/O call-back
 *  function. First is static member function and does nothing
 *  but invokes its non-static member partner. Static member
 *  function is defined with this macro, while its non-static
 *  member partner is only declared by it.
 *
 *  Both functions have these parameters in common:
 *
 *  - ctx - reference to an instance of I/O multiplexer (or its
 *  subclass) implementation. Its class should implement this
 *  time hook function
 *
 *  -flags - EPOLL flags. Two of them are used most often:
 *  EPOLLIN (reading input) EPOLLOUT (writing output) but other
 *  values should not be ignored especially EPOLLHUP which
 *  indicates broken connection
 *
 *  - handler - reference to I/O handler - an instance of
 *  CDescriptor class associated with this call-back function
 *
 *  - fd - file descriptor associated with I/O activity
 *
 *  Static partner has an additional parameter:
 *
 *  - appdata - any reasonable value for this parameter is a
 *  reference to an instance of class implementing this function
 *  since I/O multiplexer provides it in the call of this function
 *  at every I/O activity for given fd.
 *
 *  @param x name of both functions
 *  @param y class implementing functions x(). It must be
 *  the name of class defining these functions otherwise the
 *  call of non-static member of function x() in the definition
 *  of its static partner won't work
 *
 */
#define	fd_handler(x,y) \
inline	static	void	x (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd, ctx_appdt_t appdata) \
{ \
	((y*)appdata)->x (ctx, flags, handler, fd); \
} \
void	x (MpxRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd);

/*! brief signal call-back function
 *
 *  definition of two functions representing signal call-back
 *  function. First is static member function and does nothing
 *  but invokes its non-static member partner. Static member
 *  function is defined with this macro, while its non-static
 *  member partner is only declared by it.
 *
 *  Both functions have these parameters in common:
 *
 *  - ctx - reference to an instance of I/O multiplexer (or its
 *  subclass) implementation. Its class should implement this
 *  time hook function
 *
 *  - handler - reference to signal handler - an instance of
 *  CSignalContext class associated with this call-back function
 *
 *  - info - reference to siginfo_t structure provided by OS when
 *  some signal triggers
 *
 *  Static partner has an additional parameter:
 *
 *  - appdata - any reasonable value for this parameter is a
 *  reference to an instance of class implementing this function
 *  since I/O multiplexer provides it in the call of this function
 *  at every signal delivery by OS
 *
 *  @param x name of both functions
 *  @param y class implementing functions x(). It must be
 *  the name of class defining these functions otherwise the
 *  call of non-static member of function x() in the definition
 *  of its static partner won't work
 *
 */
#define	sig_handler(x,y) \
inline	static	void	x (MpxRunningContext *ctx, ctx_sig_t handler, siginfo_t* info, ctx_appdt_t appdata) \
{ \
	((y*)appdata)->x (ctx, handler, info); \
} \
void	x (MpxRunningContext *ctx, ctx_fddes_t handler, siginfo_t* info);

/*! @brief I/O multiplexer superclass
 *
 *  it implements basic I/O multiplexing functionality and should
 *  be subclassed by every subclass which want to use thi functionality.
 *
 *  Main purpose of this class is to deal I/O activity of any number
 *  of open file descriptors (disk files, network sockets, POSIX message
 *  queues, etc. - in general every entity which should be expressed with file
 *  descriptor) in such a way that it look like they work at the same
 *  time. At the same time it can manage any number of timers and signal
 *  handlers. All of them (I/O activities, timers and signals) are actually
 *  serialized, they does not interrupt each other, so that we do not need
 *  to synchronize their activity. Access to data structures made by them
 *  should not be guarded by mutexes, semaphores and other synchronization
 *  mechanisms.
 *
 */

class MpxRunningContext
{
public:

	typedef void (*initfunc) (MpxRunningContext *ctx, ctx_appdt_t appdata);	//!< prototype for starting function of I/O multiplexer
	typedef void (*exitfunc) (MpxRunningContext *ctx, ctx_appdt_t appdata);	//!< prototype for end function of I/O multiplexer
	typedef void (*timehookfunc) (MpxRunningContext *ctx, struct timespec oldTime, struct timespec newTime,
		long long timeDiff, ctx_appdt_t appdata);	//!< prototype for time-hook function of I/O multiplexer
	typedef void (*cbfunc) (MpxRunningContext *ctx, unsigned int flags, ctx_fddes_t handler, int n,
		ctx_appdt_t appdata);	//!< I/O call-back function prototype
	typedef void (*tmrfunc) (MpxRunningContext *ctx, ctx_timer_t handler, struct timespec t, ctx_appdt_t appdata);//!< timer call-back function prototype
	typedef void (*sigfunc) (MpxRunningContext *ctx, ctx_sig_t handler, siginfo_t *info, ctx_appdt_t appdata);//!< signal call-back function prototype

private:
	/*! @brief I/O handler class
	 *
	 *  this class is associated with call-back functions used to
	 *  deal with I/O activity of open file descriptors. Instances
	 *  of this class are named I/O handlers by convenience.
	 *
	 *  Every I/O descriptor can be marked active or/and deleted:
	 *
	 *  - I/O activity for I/O descriptors marked deleted (m_deleted =
	 *  true) will not be observed any more. Such descriptor will be
	 *  deleted as soon as possible, never at the middle of I/O
	 *  multiplexer main loop, but always at the beginning of next
	 *  iteration
	 *
	 *  - I/O activity for I/O descriptors marked as inactive (m_active =
	 *  false) will be temporary suspended and resumed when activated
	 *  again.
	 *
	 */
	class MpxDescriptor
	{
	public:
		/*! @brief create an instance of I/O handler
		 *
		 *  it saves these information within an instance of I/O handler:
		 *
		 *  - flags - EPOLL flags: EPOLLIN for input activity, EPOLLOUT
		 *  for output activity. There should be other EPOLL flags too,
		 *  especially EPOLLHUP which indicates broken connection, etc.
		 *
		 *  - fd - open file descriptor
		 *
		 *  - f - call-back function, which will deal with I/O activity
		 *  on fd
		 *
		 *  - appdata - general pointer provided by application and is
		 *  thus interpreted by application itself when call-back function
		 *  gets called
		 *
		 *  - info - additional information provided by application which
		 *  should be written to console by stat()
		 *
		 *  Above fd (file descriptor) and f (call-back function) are
		 *  frequently said to be associated with I/O handler containing
		 *  them and they are occasionally used interchangeably
		 *
		 */
		MpxDescriptor (uint flags, int fd, cbfunc f, ctx_appdt_t appdata, const char* info = 0) :
			m_flags (flags), m_fd (fd), m_f (f), m_data (appdata), m_active (true), m_deleted (false), m_info (info)
		{
		}
		inline void flags (uint flags)	//!< redefines I/O handler flags
		{
			m_flags = flags;
		}
		inline uint flags (void)	//!< gets I/O handler flags
		{
			return m_flags;
		}
		inline int fd (void)	//!< gets file descriptor associated with I/O handler
		{
			return m_fd;
		}
		inline cbfunc f (void)	//!< gets call-back function associated with I/O handler
		{
			return m_f;
		}
		inline void f (cbfunc p_f)	//!< redefines call-back function associated with I/O handler
		{
			m_f = p_f;
		}
		inline ctx_appdt_t data (void)	//!< gets application data
		{
			return m_data;
		}
		inline void data (ctx_appdt_t p_data)	//!< redefines application data
		{
			m_data = p_data;
		}
		inline const char* info (void)	//!< gets additional info provided by application
		{
			return m_info;
		}
		inline bool active (void)	//!< gets activity flag
		{
			return m_active;
		}
		inline void active (bool p_active)	//!< sets activity flag
		{
			m_active = p_active;
		}
		inline bool deleted (void)	//! gets deleted flag
		{
			return m_deleted;
		}
		inline void deleted (bool p_deleted)	//!< sets deleted flag
		{
			m_deleted = p_deleted;
		}
	private:
		uint m_flags;	//!< EPOLL flags for file descriptor
		int m_fd;	//!< I/O file descriptor number
		cbfunc m_f;	//!< I/O call-back function reference
		ctx_appdt_t m_data;	//!< reference to application data
		bool m_active;	//!< activity flag of file descriptor
		bool m_deleted;	//!< deleted flag of file descriptor
		const char* m_info;	//!< additional info provided by application
	};
	typedef multimap <int, MpxDescriptor*> cbset;	//!< type representing set of I/O handlers
	typedef vector <MpxDescriptor*> cblist;	//!< type representing list of prepared I/O handlers

	/*! @brief timer class
	 *
	 *  this class is associated with call-back functions which will be
	 *  called by I/O multiplexer whenever timer expires
	 *
	 *  Timers can be deactivated setting m_active to false preventing
	 *  them to expire and thus disabling associated call-back function
	 *  call
	 */
	class MpxTimer
	{
	public:
		/*! @brief create an instance of timer
		 *
		 *  constructor saves these informations:
		 *
		 *  - t - time stamp of expiration time
		 *
		 *  - f - call-back function which will be invoked by I/O multiplexer when
		 *  timer expires
		 *
		 *  - appdata - general reference to data provided by application, I/O multiplexer
		 *  does not interpret it
		 *
		 *  - info - additional reference to some string provided by application
		 *  and displayed to console with stat()
		 *
		 *  Above f (call-back function) is frequently said to be associated
		 *  with timer containing it and they are occasionally used interchangeably
		 *
		 */
		MpxTimer (struct timespec t, tmrfunc f, ctx_appdt_t appdata, const char* info = 0) :
			m_t (t), m_f (f), m_data (appdata), m_active (true), m_info (info)
		{
		}
		inline struct timespec t (void)	//!< get timer time stamp
		{
			return m_t;
		}
		inline tmrfunc f (void)	//!< get timer call-back function reference
		{
			return m_f;
		}
		inline ctx_appdt_t data (void)	//!< get application data reference
		{
			return m_data;
		}
		inline const char* info (void)	//!< get reference to additional info
		{
			return m_info;
		}
		inline bool active (void)	//!< get active flag
		{
			return m_active;
		}
		inline void active (bool p_active)	//!< set active flag
		{
			m_active = p_active;
		}
		/*! @brief adjust timer expiration
		 *
		 *  when I/O multiplexer detects system time changes it adjusts
		 *  timer time-stamp causing it to extend or shorten its expiration
		 *
		 *  @param - advance indicates direction of adjustment: true means
		 *  to extend it, false to shorten it
		 *  @param - diff adjustment timespec structure. Depending to advance
		 *  flag, this value is added to or subtracted from timer time-stamp
		 *  value.
		 *
		 */
		void compensate (bool advance, struct timespec diff)
		{
			if (advance)
			{
				if ((m_t.tv_nsec += diff.tv_nsec) >= SEC_TO_NSEC)
				{
					m_t.tv_nsec -= SEC_TO_NSEC;
					m_t.tv_sec += 1;
				}
				m_t.tv_sec += diff.tv_sec;
			}
			else
			{
				if ((m_t.tv_nsec -= diff.tv_nsec) < 0)
				{
					m_t.tv_nsec += SEC_TO_NSEC;
					m_t.tv_sec -= 1;
				}
				m_t.tv_sec -= diff.tv_sec;
			}
		}
	private:
		struct timespec m_t;	//!< expiration time
		tmrfunc m_f;	//!< timer call-back function reference
		ctx_appdt_t m_data;	//!< reference to application data
		bool m_active;	//!< is timer active ?
		const char* m_info;	//!< additional string to be displayed by stat()
	};

	/*! @brief compare two time-out values
	 *
	 *  function represents 'less then' relation for timer expiration time-stamps
	 *  and is used as a predicate function in sets containing timers.
	 *
	 */
	typedef struct timecmp: public less <struct timespec>
	{
		/*! @brief 'less than' relation on timer values
		 *
		 *  Algorithm is very simple: timer x is 'less than' timer y if its tv_sec
		 *  is less than timer y's tv_sec or if they are equal and x's tv_nsec is
		 *  less then y's tv_nsec. Since it is implemented in such a way that both
		 *  relations x < y and y < x cannot hold simultaneously, this relation
		 *  can be used to implement total order on the set of timers and is useful
		 *  to construct sets of timers
		 *
		 *  @param x first timer
		 *  @param y second timer
		 *
		 *  @return bool value of 'less than' relation
		 */
		bool operator () (const struct timespec& x, const struct timespec& y) const
		{
			return ((x.tv_sec < y.tv_sec) || ((x.tv_sec == y.tv_sec) && (x.tv_nsec < y.tv_nsec)));
		}
	} timecmp;
	typedef multimap <struct timespec, MpxTimer*, timecmp> tmrset;	//!< ordered set of timers
	typedef vector <MpxTimer*> tmrlist;	//!< unordered list of timers

	/*! @brief signal handler
	 *
	 *  class represents signal handler for signals delivered by OS.
	 *  signal handling functionality of this class is based on
	 *  sigaction mechanism. See 'man 7 signal' for additional info
	 *  about sigaction.
	 */
	class MpxSignalContext
	{
	public:
		/*! @brief create signal handler
		 *
		 *  function saves following informations in newly created instance
		 *  of signal handler:
		 *
		 *  - ctx - reference to instance of I/O multiplexer which created
		 *  signal handler. It will be used by signal delivery to select
		 *  correct I/O multiplexer when redirecting signals
		 *  - signo - number of signal to be handled by this handler
		 *  - f - call-back function to be used when handling signal
		 *  - appdata - general reference to data provided by application
		 *
		 */
		MpxSignalContext (MpxRunningContext* ctx, int signo, sigfunc f, ctx_appdt_t appdata) :
			m_ctx (ctx), m_signo (signo), m_f (f), m_data (appdata)
		{
			m_next = 0;
			memset (&m_sigAction, 0, sizeof(struct sigaction));
			memset (&m_oldAction, 0, sizeof(struct sigaction));
		}
		inline MpxSignalContext* next ()	//!< next signal handler in global list
		{
			return m_next;
		}
		inline MpxSignalContext* link (MpxSignalContext* sig)	//!< link signal handler into global list
		{
			m_next = sig;
			return this;
		}
		inline MpxRunningContext* ctx ()	//!< owner I/O multiplexer instance
		{
			return m_ctx;
		}
		inline int signo ()	//!< signal number
		{
			return m_signo;
		}
		inline sigfunc f (void)	//!< call-back function handling signal
		{
			return m_f;
		}
		inline ctx_appdt_t data (void)	//!< application data reference
		{
			return m_data;
		}
		inline struct sigaction* sigAction ()	//!< current sigaction structure
		{
			return &m_sigAction;
		}
		inline struct sigaction* oldAction ()	//!< previous sigaction structure
		{
			return &m_oldAction;
		}
		/*! @brief remove signal handler from linked list
		 *
		 *  remove signal handler from linked list of signal handlers
		 *  and then delete it
		 *
		 *  @param list reference to list of signal handlers
		 *  @param sig signal handler to be removed
		 *
		 */
		static void removeFromList (MpxSignalContext** list, MpxSignalContext* sig)
		{
			for (; (*list != 0) && (*list != sig); list = &((*list)->m_next))
				;
			if (*list != 0)
				*list = (*list)->m_next;
			delete sig;
		}
	private:
		MpxSignalContext* m_next;	//!< reference to next signal handler in list
		MpxRunningContext* m_ctx;	//!< reference to owing I/O multiplexer
		int m_signo;	//!< signal number to be handled by this handler
		sigfunc m_f; //!< signal handler call-back function
		ctx_appdt_t m_data; //!< application data associated with call-back function
		struct sigaction m_sigAction;	//!< sigaction for current handler
		struct sigaction m_oldAction;	//!< sigaction for previous handler
	};

public:

	/*! @brief reference to next signal handler
	 *
	 *  @param ctx generalized reference to signal handler
	 *
	 *  @return 0 no handler follows this one
	 *  @return other reference to signal handler following this one
	 *
	 */
	inline MpxSignalContext* signext (ctx_sig_t ctx)
	{
		return ((MpxSignalContext*) ctx)->next ();
	}
	/*! @brief link signal handler
	 *
	 *  signal handler is always linked at the beginning of
	 *  the list of signal handlers
	 *
	 *  @param ctx generalized reference to signal handler list (the
	 *  first signal handler within it)
	 *  @param sig signal handler to be linked
	 *
	 *  @return CSignalContext* reference to signal handler at the beginning
	 *  of list, actually the reference to handler being linked
	 *
	 */
	inline MpxSignalContext* siglink (ctx_sig_t ctx, MpxSignalContext* sig)
	{
		return ((MpxSignalContext*) ctx)->link (sig);
	}
	/*! @brief reference to owning I/O multiplexer
	 *
	 *  @param ctx generalized reference to signal handler
	 *
	 *  @return CSignalContext* reference to I/O multiplexer owning this handler
	 *
	 */
	inline MpxRunningContext* sigctx (ctx_sig_t ctx)
	{
		return ((MpxSignalContext*) ctx)->ctx ();
	}
	/*! @brief signal number of handler
	 *
	 *  @param ctx generalized reference to signal handler
	 *
	 *  @return int number of signal handled by this handler
	 *
	 */
	inline int sigsigno (ctx_sig_t ctx)
	{
		return ((MpxSignalContext*) ctx)->signo ();
	}
	/*! @brief call-back function of signal handler
	 *
	 *  @param ctx generalized reference to signal handler
	 *
	 *  @return sigfunc call-back function of signal handler
	 *
	 */
	inline sigfunc sigf (ctx_sig_t ctx)
	{
		return ((MpxSignalContext*) ctx)->f ();
	}
	/*! @brief application data of signal handler
	 *
	 *  @param ctx generalized reference to signal handler
	 *
	 *  @return ctx_appdt_t generalized reference to application data of signal handler
	 *
	 */
	inline ctx_appdt_t sigdata (ctx_sig_t ctx)
	{
		return ((MpxSignalContext*) ctx)->data ();
	}
	/*! @brief sigaction of current signal handler
	 *
	 *  @param ctx generalized reference to signal handler
	 *
	 *  @return sigaction* generalized reference to sigaction structure of current signal handler
	 *
	 */
	inline struct sigaction* sigsigAction (ctx_sig_t ctx)
	{
		return ((MpxSignalContext*) ctx)->sigAction ();
	}
	/*! @brief sigaction of previous signal handler
	 *
	 *  @param ctx generalized reference to signal handler
	 *
	 *  @return sigaction* generalized reference to sigaction structure of previous signal handler
	 *
	 */
	inline struct sigaction* sigoldAction (ctx_sig_t ctx)
	{
		return ((MpxSignalContext*) ctx)->oldAction ();
	}

private:
	MpxRunningContext (bool initialize);
public:
	MpxRunningContext (initfunc f, exitfunc g, timehookfunc h, ctx_appdt_t appdata, const char* name = 0);
	virtual ~MpxRunningContext ();
	inline pid_t tid ()
	{
		return m_tid;
	}
private:
	void CheckTimeValidity ();
	void HandleDescriptors (int count);
	void ActivateDescriptors (void);
	struct timespec* HandleTimers (void);
	void CompensateTimers (long long timeDiff);
	static void HandleSignal (int sig, siginfo_t *info, void *data);
	static void HandleAllSignals (MpxRunningContext* ctx);

public:
	int MainLoop (void);
	inline void Quit ()
	{
		m_quit = true;
	}

	ctx_fddes_t RegisterDescriptor (uint flags, int fd, cbfunc f, ctx_appdt_t appdata, const char* info = 0);
	/*! @brief change I/O call-back function of I/O handler
	 *
	 *  @param des generalized reference to I/O handler
	 *  @param f reference to new call-back function
	 *
	 */
	inline void ChangeDescriptorHandler (ctx_fddes_t des, cbfunc f)
	{
		if (des)
			((MpxDescriptor*) des)->f (f);
	}
	/*! @brief change application data reference of I/O handler
	 *
	 *  @param des generalized reference to I/O handler
	 *  @param appdata new application data reference
	 *
	 */
	inline void ChangeDescriptorAppData (ctx_fddes_t des, ctx_appdt_t appdata)
	{
		if (des)
			((MpxDescriptor*) des)->data (appdata);
	}
	void EnableDescriptor (ctx_fddes_t hdlr, uint flags);
	void DisableDescriptor (ctx_fddes_t hdlr, uint flags);
	void RemoveDescriptor (ctx_fddes_t hdlr);
	void ReplyDescriptor (ctx_fddes_t hdlr, unsigned int flags, int fd);

	ctx_timer_t RegisterTimer (struct timespec timer, tmrfunc f, ctx_appdt_t appdata, const char* info = 0);
	/*! @brief enable timer
	 *
	 *  function activates previously inactive timer
	 *
	 *  @param tmr generalized reference to timer handler
	 */
	inline void EnableTimer (ctx_timer_t tmr)
	{
		if (tmr)
			((MpxTimer*) tmr)->active (true);
	}
	/*! @brief disable timer
	 *
	 *  function deactivates previously active timer
	 *
	 *  @param tmr generalized reference to timer handler
	 */
	inline void DisableTimer (ctx_timer_t tmr)
	{
		if (tmr)
			((MpxTimer*) tmr)->active (false);
	}
	/*! @brief real time stamp
	 *
	 *  @return timespec function returns real time stamp
	 *  of of current iteration of I/O multiplexer. Within this
	 *  step time does not change. All call-back functions invoked
	 *  in single step of I/O multiplexer have same real time stamp
	 *  especially timer call-back functions. Using this function
	 *  is preferred way to compute real time stamps within call-back
	 *  functions because it compensates possible time delays caused
	 *  by very busy iterations
	 *
	 */
	inline struct timespec realTime (void)
	{
		return m_realTime;
	}

	ctx_sig_t RegisterSignal (int sig, sigfunc f, ctx_appdt_t appdata);
	void RemoveSignal (ctx_sig_t hdlr);

private:
	static inline void sigLock ()
	{
		pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);
		pthread_mutex_lock (&g_lock);
	}
	static inline void sigUnlock ()
	{
		pthread_mutex_unlock (&g_lock);
		pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
	}

public:
	void stat (const char* msg);
	int round ()
	{
		return m_round;
	}
	struct timespec timer ()
	{
		return m_timer;
	}

private:
	static const int g_epollSize = 1024;	//!< initial size of EPOLL set
	pid_t m_tid;
	int m_epollSetId;	//!< ID of EPOLL set used by I/O multiplexer
	epoll_event m_epollSet [g_epollSize];	//!< EPOLL set

	initfunc m_initfunc;	//!< 'initial state' function
	exitfunc m_exitfunc;	//!< 'final state' function
	timehookfunc m_timehookfunc;	//!< 'time hook' function
	ctx_appdt_t m_appldata;	//!< application (context specific) data

	struct timespec m_timer;	//!< time stamp for the first expired timer(s)
	struct timespec m_realTime;	//!< real-clock current time
	struct timespec m_cpuTime;	//!< CPU time for this process

	int m_nfds;	//!< nr. of active I/O handlers
	cblist m_cblist;	//!< I/O handlers prepared for next iteration
	cbset m_cbset;	//!< current I/O handlers

	tmrlist m_tmrlist;	//!< timer handlers prepared for next iteration
	tmrset m_tmrset;	//!< current timer handlers

	static pthread_mutex_t g_lock;	//!< signal list synchronization lock
	static sigset_t g_sigset;	//!< global signal mask for all I/O multiplexers
	static MpxSignalContext* g_sigActions [_NSIG];	//!< global list of signal handlers
	static MpxRunningContext* g_initializer;	//!< signal handler initialization object

	int m_round;	//!< round number of current iteration of I/O multiplexer
	bool m_quit;	//!< quit indicator set by Quit()
};

}
