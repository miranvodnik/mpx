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

#include "FtpRequestInterface.h"

namespace sftp
{

char* FtpRequestInterface::GetFtpRequestText (FtpRequest* req, char ini, char trm)
{
	char *report = 0;
	int size = 0;

	while (true)
	{
		switch (req->request.requestCode)
		{
		case CheckConnectivity:
		{
			size = snprintf (report, size, "CHECK CONNECTIVITY REQUEST%c"
				"%cuser@hostname     = %s@%s%c", trm, ini, req->connection.user, req->connection.hostname, trm);
		}
			break;
		case MakeDir:
		{
			MakeDirRequest* m_makeDir = &req->request.FtpRequestUnion_u.makeDir;
			size = snprintf (report, size, "MAKE DIRECTORY REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%cremote directory  = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_makeDir->remoteDirName, trm);
		}
			break;
		case GetDir:
		{
			GetDirRequest* m_getDir = &req->request.FtpRequestUnion_u.getDir;
			size = snprintf (report, size, "LIST DIRECTORY REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%cremote directory  = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_getDir->remoteDirName, trm);
		}
			break;
		case StoreFile:
		{
			StoreFileRequest* m_storeFile = &req->request.FtpRequestUnion_u.storeFile;
			size = snprintf (report, size, "STORE FILE REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%clocal directory   = %s%c"
				"%clocal file        = %s%c"
				"%cremote directory  = %s%c"
				"%cremote file       = %s%c"
				"%cworking directory = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_storeFile->localDirName, trm, ini, m_storeFile->localFileName, trm, ini, m_storeFile->remoteDirName,
				trm, ini, m_storeFile->remoteFileName, trm, ini, m_storeFile->workingDirName, trm);
		}
			break;
		case RetrieveFile:
		{
			RetrieveFileRequest* m_retrieveFile = &req->request.FtpRequestUnion_u.retrieveFile;
			size = snprintf (report, size, "RETRIEVE FILE REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%clocal directory   = %s%c"
				"%clocal file        = %s%c"
				"%cremote directory  = %s%c"
				"%cremote file       = %s%c"
				"%cworking directory = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_retrieveFile->localDirName, trm, ini, m_retrieveFile->localFileName, trm, ini,
				m_retrieveFile->remoteDirName, trm, ini, m_retrieveFile->remoteFileName, trm, ini,
				m_retrieveFile->workingDirName, trm);
		}
			break;
		case StoreAllFiles:
		{
			StoreAllFilesRequest* m_storeAllFiles = &req->request.FtpRequestUnion_u.storeAllFiles;
			size = snprintf (report, size, "STORE ALL FILES REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%clocal directory   = %s%c"
				"%cremote directory  = %s%c"
				"%cworking directory = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_storeAllFiles->localDirName, trm, ini, m_storeAllFiles->remoteDirName, trm, ini,
				m_storeAllFiles->workingDirName, trm);
		}
			break;
		case RetrieveAllFiles:
		{
			RetrieveAllFilesRequest* m_retrieveAllFiles = &req->request.FtpRequestUnion_u.retrieveAllFiles;
			size = snprintf (report, size, "RETRIEVE ALL FILES REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%clocal directory   = %s%c"
				"%cremote directory  = %s%c"
				"%cworking directory = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_retrieveAllFiles->localDirName, trm, ini, m_retrieveAllFiles->remoteDirName, trm, ini,
				m_retrieveAllFiles->workingDirName, trm);
		}
			break;
		case DeleteFile:
		{
			DeleteFileRequest* m_deleteFile = &req->request.FtpRequestUnion_u.deleteFile;
			size = snprintf (report, size, "DELETE FILE REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%cremote directory  = %s%c"
				"%cremote file       = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_deleteFile->remoteDirName, trm, ini, m_deleteFile->remoteFileName, trm);
		}
			break;
		case DeleteAllFiles:
		{
			DeleteAllFilesRequest* m_deleteAllFiles = &req->request.FtpRequestUnion_u.deleteAllFiles;
			size = snprintf (report, size, "DELETE ALL FILES REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%cremote directory  = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_deleteAllFiles->remoteDirName, trm);
		}
			break;
		case MakeWorkingEnv:
		{
			MakeWorkingEnvRequest* m_makeWorkingEnv = &req->request.FtpRequestUnion_u.makeWorkingEnv;
			size = snprintf (report, size, "MAKE WORKING ENVIRONMENT REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%cremote directory  = %s%c"
				"%cworking directory = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_makeWorkingEnv->remoteDirName, trm, ini, m_makeWorkingEnv->workingDirName, trm);
		}
			break;
		case CleanDir:
		{
			CleanDirRequest* m_cleanDir = &req->request.FtpRequestUnion_u.cleanDir;
			size = snprintf (report, size, "CLEAN DIRECTORY REQUEST%c"
				"%cuser@hostname     = %s@%s%c"
				"%cremote directory  = %s%c", trm, ini, req->connection.user, req->connection.hostname, trm, ini,
				m_cleanDir->remoteDirName, trm);
		}
			break;
		default:
			return 0;
			break;
		}
		if (size <= 0)
			return 0;
		if (size > 0)
		{
			if (report != 0)
				return report;
			if ((report = (char*) malloc (size += 1)) == 0)
				return 0;
		}
	}
	return 0;
}

} /* namespace sftp */
