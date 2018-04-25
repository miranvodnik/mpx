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
// MpxOpenLibrary.cpp
//
//  Created on: Mar 10, 2018
//      Author: miran
//

#include <dlfcn.h>
#include <mpx-jobs/MpxOpenLibrary.h>

namespace mpx
{

MpxOpenLibrary::MpxOpenLibrary (MpxTaskBase* task, const char* libName) :
	MpxJob (task), m_libName (libName), m_lib (0), m_fcn (0)
{
}

MpxOpenLibrary::~MpxOpenLibrary ()
{
	dlclose (m_lib);
}

int MpxOpenLibrary::Execute ()
{
	return
		(((m_lib = dlopen (m_libName.c_str (), RTLD_LAZY)) == 0) || ((m_fcn = dlsym (m_lib, "CreateMpxEventXDR")) == 0)) ?
			-1 : 0;
}

} // namespace mpx
