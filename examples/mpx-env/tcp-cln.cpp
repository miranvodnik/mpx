
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stropts.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <string>
#include <iostream>
using namespace std;

int main (int n, char*p[])
{
	string	hostname;
	int	ctrlSocket;

	if (n < 2)
	{
		cout << "Usage: " << p[0] << " hostname" << endl;
		return	0;
	}
	hostname = p[1];
	ctrlSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ctrlSocket < 0)
	{
		return	0;
	}

	struct hostent* hostEntry = gethostbyname (hostname.c_str ());
	if (hostEntry == NULL)
	{
		return	0;
	}

	struct sockaddr_in addr;
	memset (&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (12345);
	addr.sin_addr.s_addr = *((uint*) (hostEntry->h_addr_list [0]));

	int status;
	if ((status = connect (ctrlSocket, (struct sockaddr*) &addr, sizeof(struct sockaddr_in))) < 0)
	{
		return	0;
	}

	int	count = 0;
	int	ret;
	char	buffer[64 * 1024];
	for (int i = 0; i < 1000 * 1000; ++i)
	{
		char message[128];
		sprintf (message, "%06d: TEST MESSAGE", i);
		while ((ret = send (ctrlSocket, message, strlen (message) + 1, 0)) < 0)
		{
			if (errno != EWOULDBLOCK)
				break;
		}
		if (ret > 0)
			++count;
		if ((i & 0x3ff) == 0)
			recv (ctrlSocket, buffer, 64 * 1024, 0);
	}
	int	needSpace = 0;
	while (true)
	{
		sleep (1);
		if (ioctl (ctrlSocket, FIONREAD, &needSpace) < 0)
			break;
		if (needSpace <= 0)
			break;
		cout << "NEED " << needSpace << endl;
		if (recv (ctrlSocket, buffer, 64 * 1024, 0) <= 0)
			break;
	}

	close (ctrlSocket);

	cout << "COUNT = " << count << endl;
	return	0;
}

