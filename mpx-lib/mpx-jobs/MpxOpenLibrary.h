//    TODO
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

//
// MpxOpenLibrary.h
//
//  Created on: Mar 10, 2018
//      Author: miran
//

#pragma once

#include <mpx-jobs/MpxJob.h>
#include <string>
using namespace std;

namespace mpx
{

//
//
//
class MpxOpenLibrary: public MpxJob
{
public:
	MpxOpenLibrary (MpxTaskBase* task, const char* libName);
	virtual ~MpxOpenLibrary ();
	virtual int Execute ();
	inline const char* libName ()
	{
		return m_libName.c_str ();
	}
	void* lib ()
	{
		return m_lib;
	}
	void* fcn ()
	{
		return m_fcn;
	}
private:
	string m_libName;
	void* m_lib;
	void* m_fcn;
};

} // namespace mpx
