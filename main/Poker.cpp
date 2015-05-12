#include "stdafx.h"
#include <stdlib.h>
#include <string.h>

#include "tiger\tiger.h"
#include "parser_info\msg_fetch.h"

#define Man_Inning 10	//应该是多少？


extern char tiger_buffer[MAX_TIGER_BUFFER];


int main(int argc, char* argv[])
{
	int  Innings = 0, Player_Remain = 8;
	int offline_cnt[8] = {0},Player_ID = 0;

	char Main_Message[30] = {0};
	Poker poker;

	unsigned short remoteport = 0;
	unsigned short clientport = 0;

	char *remoteip = argv[1];
	char *clientip = argv[3];
	
	remoteport = (unsigned short)atoi(argv[2]);
	clientport = (unsigned short)atoi(argv[4]);
	init_socket(remoteip,remoteport,clientip,clientport);


	parser_card(poker,tiger_buffer);//

	while((Player_Remain >= 2) && (Innings < Man_Inning))
	{
		seat_info_msg(Main_Message);

		blind_msg(Main_Message);
		
		hold_cards_msg(Main_Message, Player_ID);
		
		inquire_msg(Main_Message);
		
		flop_msg(Main_Message);
		inquire_msg(Main_Message);
		
		turn_msg(Main_Message);
		inquire_msg(Main_Message);
		
		river_msg(Main_Message);
		
		showdown_msg(Main_Message);
		
		pot_win_msg(Main_Message);
		
	}

	game_over();
	return 0;
}

