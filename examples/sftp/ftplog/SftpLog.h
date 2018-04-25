//    FTP/SFTP Client Library
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

#include <sys/types.h>
#include <stdarg.h>		// variable argument lists
#include <stdlib.h>		// standard library functions
#include <unistd.h>		// standard library functions
#include <string.h>		// C string manipulation
#include <syslog.h>		// syslog
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>		// error numbers
#include <stdio.h>		// standard I/O

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#define	SC_SFTP		5

#define	SC_ERR		1
#define	SC_WRN		2
#define	SC_APL		3

#ifdef	__cplusplus
extern "C"
{
#endif

int disableSftpPrint (int disable);
int sc_printf (int module, int severity, const char* format, ...);
void cf_sc_printf (int module, int severity, const char* format, ...);

#ifdef	__cplusplus
}
#endif

