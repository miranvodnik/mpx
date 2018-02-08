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

namespace mpx
{
typedef void* mpx_appdt_t;	//!< general pointer representing additional application data
typedef void* mpx_evndes_t;	//!< general pointer representing I/O handler associated with I/O call-back function

#define	mpx_event_handler(x,y) \
inline	static	void	x (MpxEventBase *event, mpx_appdt_t appdata) \
{ \
	((y*)appdata)->x (event); \
} \
void	x (MpxEventBase *event);

typedef pair <u_int, u_int> evnkey;

class evncmp: public less <evnkey>
{
public:
	bool operator() (const evnkey& __x, const evnkey& __y) const
	{
		return (__x.first < __y.first) || ((__x.first == __y.first) && (__x.second < __y.second));
	}
};

typedef void* tskmpx_t;

enum MpxTaskState
{
	InvalidState, StartState = (u_int) -1, AnyState = (u_int) -2,
};

class MpxTaskBase
{
	typedef void (*evnfunc) (MpxEventBase *event, mpx_appdt_t appdata);

	typedef pair <evnfunc, mpx_appdt_t> evndata;
	typedef map <evnkey, evndata, evncmp> evnset;
public:
	MpxTaskBase (const char* name = 0);
	virtual ~MpxTaskBase ();
	void Dispose (bool release);
	int Send (MpxTaskBase* task, MpxEventBase* event, bool invoke = false);
	evndata RegisterEventHandler (unsigned int stateCode, unsigned int eventCode, evnfunc f, mpx_appdt_t data);
	evndata RetrieveEventHandler (u_int state, u_int event);
	int RetrieveExternalTask (const char* connString);
	int HandleEvent (MpxEventBase* event);
	inline MpxTaskState state ()
	{
		return m_state;
	}
	inline void state (MpxTaskState state)
	{
		m_state = state;
	}
	inline tskmpx_t mpx ()
	{
		return m_mpx;
	}
	inline void mpx (tskmpx_t mpx)
	{
		m_mpx = mpx;
	}
	inline const char* name ()
	{
		return m_name.c_str ();
	}
	struct timespec GetCurrentTime ();
	void* StartTimer (struct timespec timerStamp);
	void StopTimer (void* timer);
	static inline void EnableSend ()
	{
		g_enableSend = true;
	}
	static inline void DisableSend ()
	{
		g_enableSend = false;
	}
	static inline int sentCount () { return g_sentCount; }

private:
	int RetrieveExternalTaskLocal (const char* connString);
	int RetrieveExternalTaskTcp4 (const char* connString);
	int RetrieveExternalTaskTcp6 (const char* connString);

private:
	static const char* g_protocolField;
	static const char* g_protocolLocal;
	static const char* g_protocolTcp4;
	static const char* g_protocolTcp6;
	static const char* g_pathField;
	static const char* g_portField;
	static const char* g_nameField;
	static const char* g_hostnameField;

private:
	static int g_sentCount;
	static bool g_enableSend;
	tskmpx_t m_mpx;
	string m_name;
	MpxTaskState m_state;
	evnset m_evnset;
};

} /* namespace mpx */
