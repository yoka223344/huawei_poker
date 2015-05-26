#ifdef WIN32
#include <Windows.h>
#endif // WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>

#include "hunter.h"

char *g_remote_ip, *g_local_ip;
unsigned short g_remote_port, g_local_port;

static pthread_t recv_thread_id;
static sem_t sem_log;

#ifdef WIN32
BOOL ctrl_handler(DWORD ctrl_type)
{
    switch (ctrl_type)
    {
    case CTRL_C_EVENT:
        system("TASKKILL /IM hunter.exe /F");
        return TRUE;
    default:
        return FALSE;
    }
}
#endif // WIN32

void log(Log_Type log_type, const char *format, ...)
{
    time_t raw_time;	// should use this to compatible with 64bit
    struct timeval time_value;
    struct tm *time_info;

    unsigned short time_ms;

    FILE *target;

    va_list va_args;

    sem_wait(&sem_log); // prevent reentry

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

    sem_post(&sem_log);

    return ;
}

// TODO: validate if Linux use this kind of var argument
int main(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS;

    int player_id;

    char hunt_cmd[MAX_PATH], hunt_arg[MAX_PATH];

    pthread_attr_t t_attr;

    sem_init(&sem_log, 0, 1);

    fprintf(stdout, "%s V%d.%d\r\n", TEAM_LOGO, VERSION_MAJOR, VERSION_MINOR);
#ifdef WIN32
    fprintf(stdout, "Windows Build\r\n");
#else // WIN32
    fprintf(stdout, "Linux Build\r\n");
#endif // WIN32

#if _DEBUG
    // kill all hunter instance:
#ifdef WIN32
    //system("TASKKILL /IM hunter.exe /F");
#else // WIN32
    system("killall hunter");
#endif // WIN32
    
#endif // DEBUG

    // parse the args:
    if (argc == 6)
    {
        g_remote_ip = argv[1];
        g_local_ip  = argv[3];

        // TODO: validate ip, port to prevent crash
        g_remote_port   = (unsigned short)atoi(argv[2]);
        g_local_port    = (unsigned short)atoi(argv[4]);

        player_id = atoi(argv[5]);
    }
    else if (argc < 5)
    {
        fprintf(stderr, "Error: no argument input!!\r\n");
        fprintf(stderr, "Use built in argument\r\n");

        g_remote_ip		= DEFAULT_SERVER_IP;
        g_remote_port	= DEFAULT_SERVER_PORT;
        g_local_ip		= DEFAULT_CLIENT_IP;
        g_local_port	= DEFAULT_CLIENT_PORT;
        
        player_id		= DEFAULT_PLAY_ID;

#ifdef WIN32
        if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE))
        {
            fprintf(stdout, "Ctrl handler installed\r\n");
        }
#if 0
        // start another 7 instance, filled with parameter:
        for (int i_inst = 1; i_inst <= HUNT_TEST_INST_COUNT; i_inst ++)
        {
            //Sleep(200);

            GetModuleFileName(NULL, hunt_cmd, MAX_PATH);

            memset(hunt_arg, 0, MAX_PATH);
            sprintf(hunt_arg, "%s %d %s %d %d%d%d%d",
                DEFAULT_SERVER_IP, DEFAULT_SERVER_PORT,
                DEFAULT_CLIENT_IP, DEFAULT_CLIENT_PORT + i_inst, 
                i_inst + 1, i_inst + 1, i_inst + 1, i_inst + 1);

            ShellExecute( NULL, "open", 
                hunt_cmd,
                hunt_arg,
                NULL,
                SW_MINIMIZE 
                );
        }
#endif // HUNT_SELF_TEST
#endif // WIN32
    }

    // create receive thread:
    ret = pthread_attr_init(&t_attr);
    ret = pthread_attr_setstacksize(&t_attr, RECV_STACK);

    ret = pthread_create(&recv_thread_id, &t_attr, recv_thread, NULL);

    // game_start won't return till game over
    game_start(player_id);

#if 1
    usleep(1000000000);
#endif

    return 0;
}
