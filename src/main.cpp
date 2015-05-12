#ifdef WIN32
#include <Windows.h>
#endif // WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hunter.h"

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

// TODO: validate if Linux use this kind of var argument
int main(int argc, char *argv[])
{
	char *server_ip, *client_ip;
	unsigned short server_port, client_port;

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
	
	char hunt_cmd[MAX_PATH], hunt_arg[MAX_PATH];

#endif // DEBUG

	// parse the args:
	if (argc == 5)
	{
		server_ip = argv[1];
		client_ip = argv[3];

		// TODO: validate ip, port to prevent crash
		server_port = (unsigned short)atoi(argv[2]);
		client_port = (unsigned short)atoi(argv[4]);
	}
	else if (argc < 5)
	{
		fprintf(stderr, "Error: no argument input!!\r\n");
		fprintf(stderr, "Use built in argument\r\n");

		server_ip	= DEFAULT_SERVER_IP;
		server_port	= DEFAULT_SERVER_PORT;
		client_ip	= DEFAULT_CLIENT_IP;
		client_port = DEFAULT_CLIENT_PORT;

#ifdef WIN32
		if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE))
		{
			fprintf(stdout, "Ctrl handler installed\r\n");
		}

		// start another 7 instance, filled with parameter:
		for (int i_inst = 0; i_inst < HUNT_TEST_INST_COUNT; i_inst ++)
		{
			GetModuleFileName(NULL, hunt_cmd, MAX_PATH);

			memset(hunt_arg, 0, MAX_PATH);
			sprintf(hunt_arg, "%s %d %s %d",
				DEFAULT_SERVER_IP, DEFAULT_SERVER_PORT,
				DEFAULT_CLIENT_IP, DEFAULT_CLIENT_PORT + i_inst);

			ShellExecute( NULL, "open", 
				hunt_cmd,
				hunt_arg,
				NULL,
				SW_MINIMIZE 
				);
		}
#endif // WIN32
	}

	init_socket(server_ip, server_port, client_ip, client_port);

	// game_start won't return till game over
	//game_start();

#if 1
	usleep(1000000000);
#endif

	return 0;
}
