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
#include <stdarg.h>

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif // WIN32

extern char *g_remote_ip, *g_local_ip;
extern unsigned short g_remote_port, g_local_port;

SOCKET sck_fd = INVALID_SOCKET;

struct sockaddr_in remote_sck_addr	= { 0 };
struct sockaddr_in local_sck_addr	= { 0 };

static char recv_buffer[MAX_SOCK_BUFFER];

/** Returns true on success, or false if there was an error */
bool set_socket_blocking_enabled(int fd, bool blocking)
{
    if (fd < 0) return false;

#ifdef WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else // WIN32
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif // WIN32
}

void *recv_thread(void *arg)
{
    int ret = EXIT_SUCCESS;

    char *message;
    int length = 0;

    fd_set fd_connect_set;
    struct timeval tv_connect_timeout;

    int addr_len = sizeof(struct sockaddr_in);

    message = recv_buffer;

    log(LOG_GENERIC, "recv thread created");

    while (1)
    {
        init_socket();

        while (1)
        {
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

            // try to connect remote:
            log(LOG_GENERIC, "trying to connect %s:%d...",
                inet_ntoa(remote_sck_addr.sin_addr), ntohs(remote_sck_addr.sin_port));

            // force socket as non-blocking
            set_socket_blocking_enabled(sck_fd, false);

            ret = connect(sck_fd, (struct sockaddr *)&remote_sck_addr, sizeof(struct sockaddr_in));
            if (ret < 0)
            {
                if (WSAGetLastError() == WSAEWOULDBLOCK)	//EINPROGRESS
                {
                    tv_connect_timeout.tv_sec = 1; 
                    tv_connect_timeout.tv_usec = 0;

                    FD_ZERO(&fd_connect_set); 
                    FD_SET(sck_fd, &fd_connect_set);

                    // wait 1 seconds:
                    ret = select(sck_fd + 1, NULL, &fd_connect_set, NULL, &tv_connect_timeout);

                    if (ret <= 0)
                    {
                        log(LOG_GENERIC, "connect too slow, reinit socket");
                        deinit_socket();
                        init_socket();
                    }
                    else
                    { // connected
                        break;
                    }
                }
                else
                {
                    log(LOG_ERROR, "connect err %d", ret);
                }
            }
            else if (ret == 0)
            {
                break;
            }

            usleep(CONNECT_RETRY_DELAY);
        }

        // force socket as blocking
        set_socket_blocking_enabled(sck_fd, true);

        log(LOG_GENERIC, "connected");
        usleep(CONNECTED_DELAY);

        send_hunter_message(HUNT_MSG_CONNECTED);

        while (1)
        {
            // blocking socket:
            ret = recv(sck_fd, recv_buffer, MAX_SOCK_BUFFER, 0);
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

int init_socket()
{
    int ret = EXIT_SUCCESS;

    unsigned int r_ip, l_ip;

#ifdef WIN32
    WSADATA wsa_data;
#endif // WIN32

    int enable_reuse = 1;

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

    // enable socket addr reuse
#if 1	
    if (setsockopt(sck_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&enable_reuse, sizeof(int)) < 0)
    {
        log(LOG_ERROR, "setsockopt error");
    }
#endif

    r_ip = inet_addr(g_remote_ip);
    l_ip = inet_addr(g_local_ip);

    remote_sck_addr.sin_family = AF_INET;	// ipv4
    remote_sck_addr.sin_addr.s_addr = r_ip;
    remote_sck_addr.sin_port = htons(g_remote_port);

    local_sck_addr.sin_family = AF_INET;
    local_sck_addr.sin_addr.s_addr = l_ip;
    local_sck_addr.sin_port = htons(g_local_port);

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
