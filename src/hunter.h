#ifndef TIGER_H
#define TIGER_H

#define TIGER_DEBUG
//#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define TEAM_LOGO				"ECNU's hunter for career-elite.huawei contest"
#define VERSION_MAJOR			0
#define VERSION_MINOR			1

#define DEFAULT_SERVER_IP		"192.168.100.1"	//"114.215.171.218"
#define DEFAULT_SERVER_PORT		6000
#define DEFAULT_CLIENT_IP		"0.0.0.0"
#define DEFAULT_CLIENT_PORT		6001		// 6001~6007
#define DEFAULT_PLAY_ID			1111

#define HUNT_SELF_TEST			0

#define HUNT_TEST_INST_COUNT	7

#define HUNT_PLAYER_NAME		"hunter"

#define RECV_STACK				64*1024		// 64 kB

#define MAX_SOCK_BUFFER			8*1024		// bytes

#define CONNECT_RETRY_DELAY		500*1000	// 100 ms
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

typedef enum _Log_Type
{
    LOG_ERROR,
    LOG_GENERIC,
} Log_Type;

typedef enum _Hunter_Status
{
    STAT_WAIT_FOR_CONNECT,
    STAT_REGISTER,
    STAT_GAME_PLAY,
    STAT_GAME_OVER,
} Hunter_Status;

typedef enum _Hunter_Message
{
    HUNT_MSG_UNKNOWN,
    HUNT_MSG_CONNECTED,
    HUNT_MSG_ACTION,
    HUNT_MSG_GAMEOVER,
} Hunter_Message;

#if 0
#define sock_receive(x, y)		{}
#else
void sock_receive	(char *, int );
#endif
void sock_send		(char *, int );

int init_socket		();
int deinit_socket	();

void *recv_thread	(void *);

void log(Log_Type log_type, const char *, ...);

void send_hunter_message(Hunter_Message );

void game_start(int );

#endif // TIGER_H
