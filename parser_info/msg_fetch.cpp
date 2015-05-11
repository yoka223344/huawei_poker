#include "msg_fetch.h"
#include <cstring>

extern Play_Info *my_play_info;

int parser_card(Poker *poker, char *card_msg)
{
	 char *p = strtok(card_msg, " ");
	 if (!strcmp(p, "SPADES"))
		 poker -> mycolor = SPADES;
	 else if (!strcmp(p, "HEARTS"))
		 poker -> mycolor = HEARTS;
	 else if (!strcmp(p, "CLUBS"))
		 poker -> mycolor = CLUBS;
	 else if (!strcmp(p, "DIAMONDS"))
		 poker -> mycolor = DIAMONDS;
	 p = strtok(NULL, " ");
	 if (!strcmp(p, "10"))
		 poker -> point = 10;
	 else if (*p == 'J')
		 poker -> point = 11;
	 else if (*p == 'Q')
		 poker -> point = 12;
	 else if (*p == 'K')
		 poker -> point = 13;
	 else if (*p == 'A')
		 poker -> point = 14;
	 else if (*p <= '9' && *p >= '2')
		 poker -> point = (int)(*p - '1');
	 return 0;
}

int seat_info_msg(char *msg_ptr)
{
	string msg = string(msg_ptr);
	int count = 0;
	msg_ptr = strtok(NULL, "\n");
	char p_tmp[10][30];
	while(strcmp(msg_ptr, "/seat "))
	{
		strcpy(p_tmp[count], msg_ptr);
		count++;
		msg_ptr = strtok(NULL, "\n");
	}
	for (int i =0; i < count; i++)
	{
		Player *new_player = new Player;
		char *p = NULL;
		if(i < 3)
		{
			char *p_f = strtok(p_tmp[i], ":"); 
			p_f = strtok(NULL, ":");
			p = strtok(p_f, " ");
		}
		else
			p = strtok(p_tmp[i], " "); 
		new_player -> PlayerId = atoi(p);
		if (count == 1)
			my_play_info -> Button = new_player -> PlayerId;
		p = strtok(NULL, " ");
		new_player -> jetton = atoi(p);
		p = strtok(NULL, " ");
		new_player -> money = atoi(p);
		new_player -> bet = 0;
		my_play_info -> players[new_player -> PlayerId] = new_player;
	}
	return 0;
}

int game_over()
{
	cout << __FUNCTION__ <<endl;
	return 0;
}

int blind_msg(char *msg_ptr)
{
	cout << __FUNCTION__<<endl;
	int count = 0;
	msg_ptr = strtok(NULL, "\n");
	char p_tmp[10][30];
	while(strcmp(msg_ptr, "/blind"))
	{
		strcpy(p_tmp[count], msg_ptr);
		count++;
		msg_ptr = strtok(NULL, "\n");
	}
	for (int i = 0; i < count; i++)
	{
		char *p = strtok(p_tmp[i], ":");
		char pid[10];
		strcpy_s(pid, p);
		p = strtok(NULL, ":");
		my_play_info -> players[atoi(pid)] ->bet = atoi(p);
		my_play_info -> players[atoi(pid)] ->jetton -= atoi(p);
	}
	return 0;
}

int hold_cards_msg(char *msg_ptr, int my_pid)
{
	cout << __FUNCTION__<<endl;
	int count = 0;
	msg_ptr = strtok(NULL, "\n");
	char p_tmp[10][30];
	while(strcmp(msg_ptr, "/hold"))
	{
		strcpy(p_tmp[count], msg_ptr);
		count++;
		msg_ptr = strtok(NULL, "\n");
	}
	msg_ptr = strtok(NULL, "\n");
	parser_card(&(my_play_info -> players[my_pid] ->hand_card[0]), p_tmp[0]);
	parser_card(&(my_play_info -> players[my_pid] ->hand_card[1]), p_tmp[1]);
	return 0;
}

int inquire_msg(char *msg_ptr)
{
	cout << __FUNCTION__<<endl;
	int count = 0;
	msg_ptr = strtok(NULL, "\n");
	char p_tmp[10][50];
	while(strcmp(msg_ptr, "/inquire"))
	{
		strcpy(p_tmp[count], msg_ptr);
		count++;
		msg_ptr = strtok(NULL, "\n");
	}
	for(int i = 0; i < count - 1; i ++)
	{
		char *p = strtok(p_tmp[i], " ");
		int playerid = atoi(p);
		Player *play_ptr = my_play_info -> players[playerid];
		p = strtok(NULL, " ");
		play_ptr -> jetton = atoi(p);
		p = strtok(NULL, " ");
		play_ptr -> money = atoi(p);
		p = strtok(NULL, " ");
		play_ptr -> bet = atoi(p);
		p = strtok(NULL, " ");
		if (!strcmp(p, "blind"))
			play_ptr ->choice = blind;
		else if (!strcmp(p, "check"))
			play_ptr ->choice = check;
		else if (!strcmp(p, "raise"))
			play_ptr ->choice = raise;
		else if (!strcmp(p, "all_in"))
			play_ptr ->choice = all_in;
		else if (!strcmp(p, "fold"))
			play_ptr ->choice = fold;
	}
	char *total_ptr = strtok(p_tmp[count - 1], ":");
	total_ptr = strtok(NULL, ":");
	my_play_info ->total_sum = atoi(total_ptr); 
	return 0;
}

int flop_msg(char *msg_ptr)
{
	char p_tmp[3][30]; 
	for (int i = 0; i < 3; i++)
	{
		msg_ptr = strtok(NULL, "\n");
		strcpy(p_tmp[i], msg_ptr);
	}
	for (int i = 0; i < 3; i++)
	{
		parser_card(&(my_play_info -> public_cards.cards[i]), p_tmp[i]);
	}
	return 0;
}

int turn_msg(char *msg_ptr)
{
	msg_ptr = strtok(NULL, "\n");
	parser_card(&(my_play_info -> public_cards.cards[4]), msg_ptr);
	return 0;
}

int river_msg(char *msg_ptr)
{
	msg_ptr = strtok(NULL, "\n");
	parser_card(&(my_play_info -> public_cards.cards[5]), msg_ptr);
	return 0;
}

int showdown_msg(char *msg_ptr)
{
	cout << __FUNCTION__ << endl;
	return 0;
}

int pot_win_msg(char *msg_ptr)
{
	cout << __FUNCTION__<<endl;
	int count = 0;
	msg_ptr = strtok(NULL, "\n");
	char p_tmp[10][30];
	while(strcmp(msg_ptr, "/pot-win"))
	{
		strcpy(p_tmp[count], msg_ptr);
		count++;
		msg_ptr = strtok(NULL, "\n");
	}
	for(int i = 0; i < count; i++)
	{
		char *p = strtok(p_tmp[i], ":");
		int playid = atoi(p);
		p = strtok(NULL, ":");
		my_play_info -> players[playid] -> jetton += atoi(p); 
		my_play_info -> players[playid] -> bet = 0;
	}
	return 0;
}





