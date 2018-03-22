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

#include <mpx-jobs/MpxJob.h>
#include <pthread.h>
#include <deque>
using namespace std;

namespace mpx
{

class MpxWorkingQueue
{
	typedef deque <MpxJob*> wqueue;
private:
	MpxWorkingQueue ();
	virtual ~MpxWorkingQueue ();
public:
	inline static int Put (MpxJob* job)
	{
		return g_workingQueue->_Put (job);
	}
	inline static MpxJob* Get ()
	{
		return g_workingQueue->_Get ();
	}
private:
	int _Put (MpxJob* job);
	MpxJob* _Get ();
	inline static void Cleanup (void* arg)
	{
		(reinterpret_cast <MpxWorkingQueue*> (arg))->_Cleanup ();
	}
	void _Cleanup ();
private:
	static MpxWorkingQueue* g_workingQueue;
	wqueue m_wqueue;
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
};

} /* namespace mpx */
