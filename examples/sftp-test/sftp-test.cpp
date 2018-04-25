//    Event Driven Task Multiplexer test Program using FTP/SFTP Client Library
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

#include <mpx-event-queues/MpxLocalMQTask.h>
#include <mpx-core/MpxEnvironment.h>
#include <mpx-core/MpxTaskMultiplexer.h>
using namespace mpx;

#include <ftpwrk/FtpClientWorker.h>
#include <ftpwrk/SftpClientWorker.h>
using namespace sftp;

#include "SftpTest.h"
using namespace sftp_test;

#include <map>
using namespace std;

static void sigint (int)
{
	signal (SIGINT, SIG_DFL);
}

int main (int n, char*p [])
{
	FtpRequest* req = SftpTest::CreateFtpRequest (n, p);
	if (req == 0)
		return 0;

	signal (SIGINT, sigint);

	MpxTaskMultiplexer* cmpx = MpxEnvironment::CreateTaskMultiplexer ();
	MpxTaskMultiplexer* smpx = MpxEnvironment::CreateTaskMultiplexer ();
	MpxTaskMultiplexer* fmpx = MpxEnvironment::CreateTaskMultiplexer ();

	for (int i = 0; i < 10; ++i)
	{
		SftpTest* sftp = new SftpTest ();
		sftp->request (req);
		cmpx->RegisterTask (sftp);

		MpxTaskBase* task = new FtpClientWorker ();
		sftp->AddFtpWorker (task);
		fmpx->RegisterTask (task);

		task = new SftpClientWorker ();
		sftp->AddSftpWorker (task);
		smpx->RegisterTask (task);
	}

	SftpTest::DeleteFtpRequest (req);

	if (MpxEnvironment::CreateEnvironment ("mpx-config.xml") < 0)
		cout << "cannot create mpx environment" << endl;

	MpxEnvironment::Start (new MpxLocalMQTask ());
	pause ();
	cout << endl << "quit" << endl;
	MpxEnvironment::Stop ();

	return 0;
}
