#ifndef TIGER_H
#define TIGER_H

#define TIGER_DEBUG
//#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define TEAM_LOGO				"ECNU's hunter for career-elite.huawei contest"
#define VERSION_MAJOR			0
#define VERSION_MINOR			1

#define DEFAULT_SERVER_IP		"222.204.232.234"
#define DEFAULT_SERVER_PORT		6000
#define DEFAULT_CLIENT_IP		"0.0.0.0"
#define DEFAULT_CLIENT_PORT		6001		// 6001~6007

#define HUNT_TEST_INST_COUNT	7


#define RECV_STACK				64*1024		// 64 kB

#define MAX_SOCK_BUFFER			8*1024		// bytes

#define CONNECT_RETRY_DELAY		100*1000	// 100 ms
#define CONNECTED_DELAY			500*1000

#ifdef WIN32
#define usleep(x)			Sleep(x/1000)
#else // WIN32
typedef int SOCKET;
#define closesocket(x)		close(x);
#define INVALID_SOCKET		-1
#define SOCKET_ERROR		-1
#define MAX_PATH			260
#endif // WIN32

typedef enum _LOG_TYPE
{
	LOG_ERROR,
	LOG_GENERIC,
} LOG_TYPE;

#if 1
#define sock_receive(x, y)		{}
#else
void sock_receive	(char *message, int length);
#endif
void sock_send		(char *message, int length);

int init_socket		(char *remote_ip, unsigned short remote_port,
					char *local_ip, unsigned short local_port);
int deinit_socket	();

#endif // TIGER_H
