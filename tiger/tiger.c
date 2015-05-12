//#define WIN32_LEAN_AND_MEAN
#include "tiger.h"

#ifdef WIN32
#include <winsock2.h>
#else // WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>		// sleep

#include <sys/time.h>
#include <fcntl.h>
#endif // WIN32
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>//在windows下没有。。

#include <time.h>
#include <stdarg.h>

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif // WIN32

SOCKET sck_fd = INVALID_SOCKET;

struct sockaddr_in remote_sck_addr	= { 0 };
struct sockaddr_in local_sck_addr	= { 0 };

pthread_t recv_thread_id;

char tiger_buffer[MAX_TIGER_BUFFER];

void log(LOG_TYPE log_type, const char *format, ...)
{
	struct timeval raw_time;
	struct tm *time_info;

	FILE *target;

	va_list va_args;

	switch (log_type)
	{
	case LOG_ERROR:
		target = stderr;
		break;
	case LOG_GENERIC:
		target = stdout;
	default:
		break;
	}

#ifdef WIN32
	//GetLocalTime()
	time(&raw_time.tv_sec);
#else // WIN32
	gettimeofday(&raw_time, NULL);
#endif // WIN32

	time_info = localtime(&raw_time.tv_sec);

	fprintf(target, "[%02d:%02d:%02d %03d] ",
		time_info->tm_hour, time_info->tm_min, time_info->tm_sec, raw_time.tv_usec/1000);

	va_start (va_args, format);
	vfprintf (target, format, va_args);
	va_end (va_args);

	fprintf(target, "\r\n");
}

static void *recv_thread(void *arg)
{
	int ret = EXIT_SUCCESS;

	char *message;
	int length = 0;

	int addr_len = sizeof(struct sockaddr_in);

	message = tiger_buffer;

	log(LOG_GENERIC, "recv thread created");

	// try to bind local port
	log(LOG_GENERIC, "bind %s:%d...", 
		inet_ntoa(local_sck_addr.sin_addr), ntohs(local_sck_addr.sin_port));

	ret = bind(sck_fd, (struct sockaddr *)&local_sck_addr, sizeof(struct sockaddr_in));
	if (ret == 0)
	{
		log(LOG_GENERIC, "bind success");
	}
	else
	{
		log(LOG_ERROR, "bind err %d", ret);
	}

	while (1)
	{
		while (1)
		{
			log(LOG_GENERIC, "trying to connect %s:%d...",
				inet_ntoa(remote_sck_addr.sin_addr), ntohs(remote_sck_addr.sin_port));

			ret = connect(sck_fd, (struct sockaddr *)&remote_sck_addr, sizeof(struct sockaddr_in));
			if (ret == 0)
			{
				log(LOG_GENERIC, "connected");
				usleep(CONNECTED_DELAY);
				break;
			}
			else
			{
				log(LOG_ERROR, "connect err %d", ret);
			}

			usleep(CONNECT_RETRY_DELAY);
		}	

		while (1)
		{
			// blocking socket:
			ret = recv(sck_fd, tiger_buffer, MAX_TIGER_BUFFER, 0);
			if (ret > 0)
			{
				length = ret;
			}
			else if (ret == 0)
			{
				// orderly shutdown
				log(LOG_ERROR, "recv shutdown");
				break;
			}
			else
			{ // ret < 0, typically -1
				// an error occurred
				log(LOG_ERROR, "recv error %d", ret);
			}

	#ifdef TIGER_DEBUG
			log(LOG_GENERIC, "Receive %d:%s", length, message);
	#endif // TIGER_DEBUG

			// once received, we call handler:
			tiger_receive(message, length);

			// check how much time is wasted on receive module
		}

		log(LOG_GENERIC, "disconnect");

		// we should gently close socket, and try to connect again
		closesocket(sck_fd);//是否是closesocket？close（原）。
	}

	log(LOG_GENERIC, "recv thread exit");
}

void tiger_send(char *message, int length)
{
	int ret = EXIT_SUCCESS;

	if (sck_fd == INVALID_SOCKET)
	{
		log(LOG_ERROR, "send failed, invalid socket");
		return ;
	}

	ret = send(sck_fd, message, length, 0);
	if (ret == SOCKET_ERROR)
	{
#ifdef WIN32
		log(LOG_ERROR, "Err %d", WSAGetLastError());
#endif // WIN32
		return ;
	}

	return ;
}

int init_socket(char *remote_ip, unsigned short remote_port,
				char *local_ip, unsigned short local_port)
{
	int ret = EXIT_SUCCESS;

	int sck_flags;

	unsigned int r_ip, l_ip;

#ifdef WIN32
	WSADATA wsa_data;
#endif // WIN32

	pthread_attr_t t_attr;

#ifdef WIN32
	// initialize Winsock:
	ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (ret != NO_ERROR)
	{
		log(LOG_ERROR, "WSAStartup fail");
		return -1;
	}
#endif // WIN32

	sck_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sck_fd == INVALID_SOCKET)
	{
		log(LOG_ERROR, "socket create error");
		return EXIT_FAILURE;
	}

	// force socket as blocking
#ifdef WIN32
#else // WIN32
	sck_flags = fcntl(sck_fd, F_GETFL, 0);
	sck_flags &= ~O_NONBLOCK;
	fcntl(sck_fd, F_SETFL, sck_flags);
#endif // WIN32

	r_ip = inet_addr(remote_ip);
	l_ip = inet_addr(local_ip);

	remote_sck_addr.sin_family = AF_INET;	// ipv4
	remote_sck_addr.sin_addr.s_addr = r_ip;
	remote_sck_addr.sin_port = htons(remote_port);

	local_sck_addr.sin_family = AF_INET;
	local_sck_addr.sin_addr.s_addr = l_ip;
	local_sck_addr.sin_port = htons(local_port);

	// create receive thread:
	ret = pthread_attr_init(&t_attr);
	ret = pthread_attr_setstacksize(&t_attr, RECV_STACK);

	ret = pthread_create(&recv_thread_id, &t_attr, recv_thread, NULL);

	return ret;
}

int deinit_socket()
{
	int ret = EXIT_SUCCESS;

	closesocket(sck_fd);

#ifdef WIN32
	WSACleanup();
#endif // WIN32

	return ret;
}

#if 1
int main(int argc, char *argv[])
{
#ifdef WIN32
	init_socket("192.168.225.128", 6000,
				"192.168.225.1", 6001);
#else // WIN32
	init_socket("127.0.0.1", 6000,
				"127.0.0.1", 6001);
#endif // WIN32

	while (1)
	{
	}

	return 0;
}
#endif
