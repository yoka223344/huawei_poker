#include <iostream>
#include <cstring>
#include "msg_fetch.h"
#include "poker.h" 
using namespace std;
Play_Info *my_play_info;

const char *split = "\n"; 

void tiger_receive	(char *message)
{
	char *msg_ptr = strtok(message, split);
	int lines = 0;
	if (msg_ptr != NULL)
	{
		if(!strcmp(msg_ptr, "seat/ "))
			seat_info_msg(msg_ptr);
		else if(!strcmp(msg_ptr, "game-over/"))
			game_over();
		else if(!strcmp(msg_ptr, "blind/"))
			blind_msg(msg_ptr);
		else if(!strcmp(msg_ptr,"hold/"))
			hold_cards_msg(msg_ptr, 33);
		else if(!strcmp(msg_ptr, "inquire/"))
			inquire_msg(msg_ptr);
		else if(!strcmp(msg_ptr, "flop/"))
			flop_msg(msg_ptr);
		else if(!strcmp(msg_ptr, "turn/"))
			turn_msg(msg_ptr);
		else if(!strcmp(msg_ptr, "river/"))
			river_msg(msg_ptr);
		else if(!strcmp(msg_ptr, "showdown/"))
			showdown_msg(msg_ptr);
		else if(!strcmp(msg_ptr, "pot-win/"))
			pot_win_msg(msg_ptr);
	}
}
int main()
{
	my_play_info = new Play_Info;
	char a[] = "seat/ \nbutton:11 11 11 \nsmall button:22 22 22 \nbig button:33 33 33 \n44 44 44 \n/seat \n";
	char b[] = "blind/\n11:22\n22:22\n/blind\n";
	char hold[] = "hold/\nSPADES J\nCLUBS K\n/hold\n";
	char flop[] = "flop/\nSPADES J\nCLUBS K\nCLUBS 5\n/flop\n";
	char inquire[] = "inquire/\n11 5 100 1 fold\ntotal pot: 11\n/inquire\n";
	char pot_win[] = "pot-win/\n11:4\n22:5\n/pot-win\n";
	tiger_receive(a);
	tiger_receive(b);
	tiger_receive(hold);
	tiger_receive(inquire);
	tiger_receive(flop);
	tiger_receive(pot_win);
	return 0;
}
