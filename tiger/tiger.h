#ifndef TIGER_H
#define TIGER_H

#define TIGER_DEBUG
#define _CRT_SECURE_NO_WARNINGS

#define RECV_STACK				64*1024		// 64 kB

#define MAX_TIGER_BUFFER		8*1024		// bytes

#define CONNECT_RETRY_DELAY		100*1000	// 100 ms
#define CONNECTED_DELAY			500*1000

#ifdef WIN32
#define usleep(x)			Sleep(x/1000)
#else // WIN32
typedef int SOCKET;
#define closesocket(x)		close(x);
#define INVALID_SOCKET		-1
#define SOCKET_ERROR		-1
#endif // WIN32

typedef enum _LOG_TYPE
{
	LOG_ERROR,
	LOG_GENERIC,
} LOG_TYPE;

#if 1
#define tiger_receive(x, y)		{}
#else
void tiger_receive	(char *message, int length);
#endif
void tiger_send		(char *message, int length);

int init_socket		(char *remote_ip, unsigned short remote_port,
					char *local_ip, unsigned short local_port);
int deinit_socket	();

#endif // TIGER_H
