#ifndef MSG_FETCH_H
#define MSG_FETCH_H
#include <iostream>
#include <cstring>
#include "poker.h"

int parser_card(Poker poker, char *card_msg);

int seat_info_msg(char *msg_ptr);

int game_over();

int blind_msg(char *msg_ptr);

int hold_cards_msg(char *msg_ptr, int my_pid);

int inquire_msg(char *msg_ptr);

int flop_msg(char *msg_ptr);

int turn_msg(char *msg_ptr);

int river_msg(char *msg_ptr);

int showdown_msg(char *msg_ptr);

int pot_win_msg(char *msg_ptr);

#endif