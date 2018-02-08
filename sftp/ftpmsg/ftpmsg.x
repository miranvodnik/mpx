/*
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
*/

#ifdef	RPC_HDR
%#define	FTP_USE_WORKING_DIR		(1 << 0)
%#define	FTP_STORE_UNIQUE		(1 << 1)
%#define	FTP_REGEX_TEMPLATE		(1 << 2)
%#define	FTP_PROTECT_EXISTING_FILE	(1 << 3)
%#define	FTP_CHECK_FILE_SIZE		(1 << 4)
%#define	FTP_TEXT_FILE_TRANSFER		(1 << 5)
%#define	FTP_REMOVE_SOURCE_FILE		(1 << 6)
%#define	FTP_NAME_LISTING		(1 << 7)
%#define	FTP_RESOLVED			(1 << 8)
%#define	FTP_RECURSIVE_OPERATION		(1 << 9)
%#define	FTP_DESTRUCTIVE_OPERATION	(1 << 10)
%#define	FTP_SSH_OPERATION		(1 << 11)
#endif

#ifdef	RPC_XDR
%
%void dump_FtpRequest (FtpRequest *req, char* msg)
%{
%	if (req == 0)
%		return;
%	printf ("REQUEST: (%s)\n", (msg == 0) ? "" : msg);
%	FtpConnectionInfo	*connection = &req->connection;
%	char *hostname = connection->hostname;
%	char *user = connection->user;
%	char *password = connection->password;
%	char *authentication = connection->authentication;
%	int flags = connection->flags;
%	if (hostname != 0)
%		printf ("   hostname = %s\n", hostname);
%	if (user != 0)
%		printf ("   user = %s\n", user);
%	if (password != 0)
%		printf ("   password = %s\n", password);
%	if (authentication != 0)
%		printf ("   authentication = %s\n", authentication);
%	if (flags & FTP_SSH_OPERATION)
%		printf ("   protocol = ssh2\n");
%	else
%		printf ("   protocol = ftp\n");
%}
%
#endif

enum FtpRequestCode
{
	FirstRequest = 0,
	CheckConnectivity = 1,
	MakeDir = 2,
	GetDir = 3,
	StoreFile = 4,
	StoreAllFiles = 5,
	RetrieveFile = 6,
	RetrieveAllFiles = 7,
	DeleteFile = 8,
	DeleteAllFiles = 9,
	MakeWorkingEnv = 10,
	CleanDir = 11,
	LastRequest = 12
};

enum FtpExceptionAction
{
	FirstExceptionAction = 0,
	ExceptionAllow = 1,
	ExceptionDeny = 2,
	LastExceptionAction = 3
};

struct	ExceptionFileList
{
	FtpExceptionAction	action;
	ExceptionFileList*	next;
	string	fileNameTemplate<>;
};

struct	FtpConnectionInfo
{
	string	hostname<>;
	string	user<>;
	string	password<>;
	string	authentication<>;
	int	flags;
};

struct	CheckConnectivityRequest
{
	int	dummy;
};

struct	MakeDirRequest
{
	string	remoteDirName<>;
};

struct	GetDirRequest
{
	string	remoteDirName<>;
};

struct	StoreFileRequest
{
	string	localDirName<>;
	string	localFileName<>;
	string	remoteDirName<>;
	string	remoteFileName<>;
	string	workingDirName<>;
};

struct	RetrieveFileRequest
{
	string	localDirName<>;
	string	localFileName<>;
	string	remoteDirName<>;
	string	remoteFileName<>;
	string	workingDirName<>;
};

struct	StoreAllFilesRequest
{
	string	localDirName<>;
	string	remoteDirName<>;
	string	workingDirName<>;
	ExceptionFileList*	exceptions;
};

struct	RetrieveAllFilesRequest
{
	string	localDirName<>;
	string	remoteDirName<>;
	string	workingDirName<>;
	ExceptionFileList*	exceptions;
};

struct	DeleteFileRequest
{
	string	remoteDirName<>;
	string	remoteFileName<>;
};

struct	DeleteAllFilesRequest
{
	string	remoteDirName<>;
	ExceptionFileList*	exceptions;
};

struct	MakeWorkingEnvRequest
{
	string	remoteDirName<>;
	string	workingDirName<>;
};

struct	CleanDirRequest
{
	string	remoteDirName<>;
};

struct	EmptyFtpRequest
{
	int	empty;
};

union	FtpRequestUnion
switch (FtpRequestCode requestCode)
{
	case	FirstRequest:
		EmptyFtpRequest		empty;
	case	CheckConnectivity:
		CheckConnectivityRequest	checkConnectivity;
	case	MakeDir:
		MakeDirRequest		makeDir;
	case	GetDir:
		GetDirRequest		getDir;
	case	StoreFile:
		StoreFileRequest	storeFile;
	case	StoreAllFiles:
		StoreAllFilesRequest	storeAllFiles;
	case	RetrieveFile:
		RetrieveFileRequest	retrieveFile;
	case	RetrieveAllFiles:
		RetrieveAllFilesRequest	retrieveAllFiles;
	case	DeleteFile:
		DeleteFileRequest	deleteFile;
	case	DeleteAllFiles:
		DeleteAllFilesRequest	deleteAllFiles;
	case	MakeWorkingEnv:
		MakeWorkingEnvRequest	makeWorkingEnv;
	case	CleanDir:
		CleanDirRequest		cleanDir;
};

struct	FtpRequest
{
	int	id;
	FtpConnectionInfo	connection;
	int	flags;
	FtpRequestUnion		request;
};

#ifdef	RPC_HDR
%
%extern void dump_FtpRequest (FtpRequest *req, char* msg);
%
#endif
