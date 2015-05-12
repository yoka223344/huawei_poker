#ifndef MSG_FETCH_H
#define MSG_FETCH_H
#include <iostream>
#include <cstring>
#include "poker.h"

int parser_card(Poker poker, char *card_msg);

int msg_seat_info(char *msg_ptr);

int game_over();

int msg_blind(char *msg_ptr);

int msg_cards(char *msg_ptr, int my_pid);

int msg_inquire(char *msg_ptr);

int msg_flop(char *msg_ptr);

int msg_turn(char *msg_ptr);

int msg_river(char *msg_ptr);

int msg_showdown(char *msg_ptr);

int msg_pot_win(char *msg_ptr);

#endif