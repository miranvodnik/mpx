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

#include <ftplog/SftpLog.h>	// wrappers for lib. functions used by CDCP

class _SftpLogInit
{
private:
	_SftpLogInit ()
	{
		char *env;
		pthread_mutexattr_t attr;

		m_logLevel = LOG_ERR;
		m_syslogSocketPath = "/var/run/sftp.socket";
		m_syslogSocket = -1;

		if ((env = getenv ("SFTP_LOGLEVEL")) != NULL)
		{
			if (strcmp (env, "error") == 0)
				m_logLevel = LOG_ERR;
			else if (strcmp (env, "warning") == 0)
				m_logLevel = LOG_WARNING;
			else if (strcmp (env, "info") == 0)
				m_logLevel = LOG_INFO;
		}

		if ((env = getenv ("SFTP_SYSLOG_SOCKET")) != NULL)
		{
			m_syslogSocketPath = env;
		}

		pthread_mutexattr_init (&attr);
		pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init (&m_printSync, &attr);

		m_syslogSocket = -1;

		setvbuf (stdout, 0, _IOLBF, 0);
		setvbuf (stderr, 0, _IOLBF, 0);
	}

public:

	int Connect (void)
	{
		struct sockaddr_un clnaddr;

		Disconnect ();

		if ((m_syslogSocket = socket (AF_LOCAL, SOCK_STREAM, 0)) < 0)
		{
			cerr << "socket() failed, errno = " << errno << endl;
		}
		else
		{
			bzero (&clnaddr, sizeof(clnaddr));
			clnaddr.sun_family = AF_LOCAL;
			strncpy (clnaddr.sun_path, m_syslogSocketPath.c_str (), sizeof(clnaddr.sun_path) - 1);
			if (connect (m_syslogSocket, (sockaddr*) &clnaddr, sizeof(clnaddr)) < 0)
			{
				close (m_syslogSocket);
				m_syslogSocket = -1;
				cerr << "connect() failed, errno = " << errno << endl;
			}
		}
		return m_syslogSocket;
	}

	void Disconnect (void)
	{
		if (m_syslogSocket < 0)
			return;
		close (m_syslogSocket);
		m_syslogSocket = -1;
	}

	inline int Lock ()
	{
		int ret;
		while ((ret = pthread_mutex_lock (&m_printSync)) != 0)
		{
			if (errno != EINTR)
				break;
		}
		return ret;
	}

	inline int Unlock ()
	{
		int ret;
		while ((ret = pthread_mutex_unlock (&m_printSync)) != 0)
		{
			if (errno != EINTR)
				break;
		}
		return ret;
	}

	static inline _SftpLogInit *GetSftpInit (void)
	{
		return g_sftpLogInit;
	}

	inline int syslogSocket (void)
	{
		return m_syslogSocket;
	}

	inline int logLevel (void)
	{
		return m_logLevel;
	}

private:
	static _SftpLogInit *g_sftpLogInit;
	pthread_mutex_t m_printSync;	// sc_print() synchronization mutex
	string m_syslogSocketPath;	// unix doman socket path to access syslog-ng
	int m_syslogSocket;		// unix doman socket to access syslog-ng
	int m_logLevel;		// default log level = error
};

_SftpLogInit *_SftpLogInit::g_sftpLogInit = new _SftpLogInit ();	// CDCP initializer

static int g_sftp_print_disabled = 0;

int disableSftpPrint (int disable)
{
	bool retcode = g_sftp_print_disabled;
	g_sftp_print_disabled = disable;
	return retcode;
}

static const char* modulName [] =
{ "", "CDCP", "CHRG", "SDL ", "GAD ", "SFTP", };

int SftpPrintf (int module, int severity, const char* format, va_list msg)
{
	ostringstream logMessage;
	char logStr [1024];

	if (g_sftp_print_disabled != 0)
		return 0;

	_SftpLogInit* log = _SftpLogInit::GetSftpInit ();

	time_t now = time (NULL);
	struct tm ltime;

	localtime_r (&now, &ltime);
	sprintf (logStr, "%04d-%02d-%02dT%02d:%02d:%02dZ %s(%5d) : ", 1900 + ltime.tm_year, 1 + ltime.tm_mon, ltime.tm_mday,
		ltime.tm_hour, ltime.tm_min, ltime.tm_sec, modulName [module], getpid ());

	// beginning of syslog protocol message (HEADER has default values - RFC3164):
	// <facility,severity> = <16,3> or <16,4> or <16,5>, timestamp = NILLVALUE (-), hostname = NILVALUE (-), process = NILVALUE (-)
	// logStr contains generated part of message (date, time and process id)

	switch (severity)
	{
	case SC_ERR:		// syslog header: facility = local use 0, severity = Error
		logMessage << "<" << (16 << 3) + 3 << "> ";
		break;
	case SC_WRN:		// syslog header: facility = local use 0, severity = Warning
		logMessage << "<" << (16 << 3) + 4 << "> ";
		break;
	case SC_APL:		// syslog header: facility = local use 0, severity = Notice
		logMessage << "<" << (16 << 3) + 5 << "> ";
		break;
	default:		// default in cdcp: syslog header: facility = local use 0, severity = Error
		logMessage << "<" << (16 << 3) + 3 << "> ";
		break;
	}

	logMessage << logStr;

	// MSG part of syslog protocol message: append severity

	switch (severity)
	{
	case SC_ERR:
		logMessage << "(E) ";
		break;
	case SC_WRN:
		logMessage << "(W) ";
		break;
	case SC_APL:
		logMessage << "(I) ";
		break;
	default:
		logMessage << "(E) ";
		break;
	}

	// MSG part of syslog protocol message: append application defined message

	vsnprintf (logStr, 1024, format, msg);
	logMessage << logStr << ends;

	basic_string <char> str = logMessage.str ();
	const char *msgString = str.c_str ();

	for (int i = 0; i < 2; ++i)
	{
		volatile int syslogSocket = log->syslogSocket ();
		if ((syslogSocket < 0) && ((syslogSocket = log->Connect ()) < 0))
			cerr << "cannot send syslog message: " << msgString << endl;
		else
		{
			switch (log->logLevel ())
			{
			case LOG_ERR:
				if (severity != SC_ERR)
					goto nomsglab;
				break;
			case LOG_WARNING:
				if ((severity != SC_ERR) && (severity != SC_WRN))
					goto nomsglab;
				break;
			case LOG_INFO:
				if ((severity != SC_ERR) && (severity != SC_WRN) && (severity != SC_APL))
					goto nomsglab;
				break;
			default:
				if (severity != SC_ERR)
					goto nomsglab;
				break;
			}
			cout << msgString << endl;
			if (write (syslogSocket, msgString, strlen (msgString) + 1) <= 0)
			{
				cerr << "failed to write syslog message (errno = " << errno << "): " << msgString << endl;
				log->Disconnect ();
				continue;
			}
		}
		nomsglab: break;
	}

#ifdef	SFTP_TEST
	switch (log->logLevel())
	{
		case LOG_ERR:
		if (severity != SC_ERR)
		goto endlab;
		break;
		case LOG_WARNING:
		if ((severity != SC_ERR) && (severity != SC_WRN))
		goto endlab;
		break;
		case LOG_INFO:
		if ((severity != SC_ERR) && (severity != SC_WRN) && (severity != SC_APL))
		goto endlab;
		break;
		default:
		if (severity != SC_ERR)
		goto endlab;
		break;
	}

	cout << msgString + 12 << endl;
	endlab:

#endif	// SFTP_TEST

	return 0;
}

/****************************************************************/
/* Function:    cf_sc_printf ()                                 */
/* In:          module - software module id                     */
/*              severity - error severity                       */
/*              format - print format                           */
/*              ... - printf arguments                          */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      return code of sc_printf                        */
/* Description: function disables cancellation mechanisms during*/
/*              invocation of sc_printf                         */
/****************************************************************/

void cf_sc_printf (int module, int severity, const char* format, ...)
{
	int cancelstate, dummy;
	va_list pArg;

	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &cancelstate);
	va_start(pArg, format);
	SftpPrintf (module, severity, format, pArg);
	va_end(pArg);
	pthread_setcancelstate (cancelstate, &dummy);
	pthread_testcancel ();
}
