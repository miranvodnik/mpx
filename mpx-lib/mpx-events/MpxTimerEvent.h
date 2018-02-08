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
#include <mpx-core/MpxRunningContext.h>
#include <mpx-events/MpxEventBase.h>
#include <mpx-tasks/MpxTaskBase.h>

namespace mpx
{

class MpxTimerEvent: public mpx::MpxEventBase
{
public:
	MpxTimerEvent (MpxTaskBase* task, struct timespec timerStamp);
	virtual ~MpxTimerEvent ();
	virtual const char* Name ()
	{
		return "Timer Event";
	}

	timer_handler (HandleTimer, MpxTimerEvent)
	;
	inline struct timespec timerStamp ()
	{
		return m_timerStamp;
	}
	inline ctx_timer_t timer ()
	{
		return m_timer;
	}
	inline void timer (ctx_timer_t timer)
	{
		m_timer = timer;
	}
	virtual MpxEventBase* Copy ()
	{
		timespec t =
		{ 0, 0 };
		return new MpxTimerEvent (0, t);
	}
	virtual int Encode (xdrproc_t& proc, xdrdata_t& data)
	{
		return 0;
	}
	virtual int Decode (MpxEventStruct* eventStruct)
	{
		return 0;
	}
private:
	MpxTaskBase* m_task;
	struct timespec m_timerStamp;
	ctx_timer_t m_timer;
};

} /* namespace mpx */
