#ifndef POKER_H
#define POKER_H
#include <iostream>
#include <map>
using namespace std;
enum Colors {SPADES, HEARTS, CLUBS, DIAMONDS};
enum Player_Choice {start, blind, check, call, raise, all_in, fold};
class Poker
{
public:
	Colors mycolor;
	int point;
};

class Player
{
public:
	Poker hand_card[2];
	int PlayerId;
	int money;
	int jetton;
	int bet;
	Player_Choice choice;
};

class All_Cards
{
public:
	Poker cards[5];
	int cards_number;
};

class Play_Info
{
public:
	map<int, Player *> players; 
	All_Cards public_cards;
	int Button;
	int total_sum;

};
#endif