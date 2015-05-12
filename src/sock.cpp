#include "hunter.h"

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
#include <pthread.h>

#include <time.h>
#include <stdarg.h>

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif // WIN32

SOCKET sck_fd = INVALID_SOCKET;

struct sockaddr_in remote_sck_addr	= { 0 };
struct sockaddr_in local_sck_addr	= { 0 };

pthread_t recv_thread_id;

char sock_buffer[MAX_SOCK_BUFFER];

void log(LOG_TYPE log_type, const char *format, ...)
{
	time_t raw_time;	// should use this to compatitible with 64bit
	struct timeval time_value;
	struct tm *time_info;

	unsigned short time_ms;

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
	SYSTEMTIME system_time;
	GetLocalTime(&system_time);
	time_ms = system_time.wMilliseconds;

	// however, int64 could get corrupted here?
#else // WIN32
	gettimeofday(&time_value, NULL);
	time_ms = time_value.tv_usec/1000;
#endif // WIN32

	time(&raw_time);
	time_info = localtime(&raw_time);

	fprintf(target, "[%02d:%02d:%02d %03d] ",
		time_info->tm_hour, time_info->tm_min, time_info->tm_sec, time_ms);

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

	message = sock_buffer;

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
			ret = recv(sck_fd, sock_buffer, MAX_SOCK_BUFFER, 0);
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

#ifdef sock_DEBUG
			log(LOG_GENERIC, "Receive %d:%s", length, message);
#endif // sock_DEBUG

			// once received, we call handler:
			sock_receive(message, length);

			// check how much time is wasted on receive module
		}

		log(LOG_GENERIC, "disconnect");

		// we should gently close socket, and try to connect again
		closesocket(sck_fd);
	}

	log(LOG_GENERIC, "recv thread exit");
}

void sock_send(char *message, int length)
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
