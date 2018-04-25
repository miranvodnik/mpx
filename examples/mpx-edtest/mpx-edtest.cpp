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
// mpx-edtest.cpp
//
//  Created on: Apr 5, 2018
//      Author: miran
//

#include <alloca.h>

#include <MpxConsumerEventXDR.h>
#include <MpxConsumerEventA.h>
#include <MpxConsumerEventB.h>
using namespace mpx_edlib;

static void makeTest (MpxConsumerEventXDR* xdr, MpxEventBase* event)
{
	char *buffer;
	int size = xdr->Encode (event, 0, 0);
	buffer = reinterpret_cast <char*> (alloca (size));
	xdr->Encode (event, buffer, size);
	xdr->Decode (event, buffer, size);
	delete event;
}

int main (int n, char* p[])
{
	MpxConsumerEventXDR* xdr = new MpxConsumerEventXDR ();
	while (true)
	{
		MpxConsumerEventA* a = new MpxConsumerEventA ("asdfasdfasdfasdf");
		makeTest (xdr, a);
		delete a;
	}
	return 0;
}
