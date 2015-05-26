#ifndef POKER_H
#define POKER_H

#define MAX_MSG_STR_NAME	16
#define MAX_ACT_STR_NAME	16

#define MAX_PLAYER_NAME		16

#define MAX_PLAYER			7

typedef enum _Card_Color
{
    CLUBS		= 0,	// mei hua
    DIAMONDS	= 1,	// fang pian
    HEARTS		= 2,	// hong xin
    SPADES		= 3,	// hei tao
} Card_Color;

typedef enum _Hand_Types
{
    HIGH_CARD = 0,
    PAIR,
    TWO_PAIR,
    TRIPS,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
} Hand_Types;

typedef enum _Seat_Type
{
    SEAT_DEAD		,
    SEAT_BUTTON		,
    SEAT_BIND		,
    SEAT_SMALL_BIND	,
    SEAT_BIG_BIND	,
    SEAT_GENERIC	,
} Seat_Type;

typedef enum _Action_Type
{
    ACTION_BLIND	,
    ACTION_CHECK	,
    ACTION_CALL		,
    ACTION_RAISE	,
    ACTION_ALL_IN	,
    ACTION_FOLD		,
} Action_Type;

typedef enum _Msg_Type 
{
    MSG_TYPE_NONE		,
    MSG_TYPE_REG		,
    MSG_TYPE_SEAT_INFO	,
    MSG_TYPE_GAME_OVER	,
    MSG_TYPE_BIND		,
    MSG_TYPE_HOLD_CARD	,
    MSG_TYPE_INQUIRE	,
    MSG_TYPE_ACTION		,
    MSG_TYPE_FLOP		,
    MSG_TYPE_TURN		,
    MSG_TYPE_RIVER		,
    MSG_TYPE_SHOWDOWN	,
    MSG_TYPE_POT_WIN	,
    MSG_TYPE_NOTIFY		,
} Msg_Type;

#define MSG_STR_H_NA			"na"
#define MSG_STR_H_SEAT_INFO		"seat"
#define MSG_STR_H_GAME_OVER		"game-over"
#define MSG_STR_H_BIND			"blind"
#define MSG_STR_H_HOLD_CARDS	"hold"
#define MSG_STR_H_INQUIRY		"inquire"
#define MSG_STR_H_FLOP			"flop"
#define MSG_STR_H_TURN			"turn"
#define MSG_STR_H_RIVER			"river"
#define MSG_STR_H_SHOWDOWN		"showdown"
#define MSG_STR_H_POT_WIN		"pot-win"
#define MSG_STR_H_NOTIFY		"notify"

#define ACT_STR_BIND			"blind"
#define ACT_STR_CHECK			"check"
#define ACT_STR_CALL			"call"
#define ACT_STR_RAISE			"raise"
#define ACT_STR_ALL_IN			"all_in"
#define ACT_STR_FOLD			"fold"

typedef struct _Msg_Table
{
    Msg_Type	msg_type;
    char		msg_str[MAX_MSG_STR_NAME];
    int			(*msg_func)(char *, char *);
} Msg_Table;

typedef struct _Action_Table
{
    Action_Type action_type;
    char		action_str[MAX_ACT_STR_NAME];
} Action_Table;

typedef struct _Player_Info	// the opponents
{
    int			player_id;
    char		player_name[MAX_PLAYER_NAME];
    Seat_Type	seat_type;
    int			jetton;
    int			money;
    int			bet;
    Action_Type	action_type;
    __int64		cards;
} Player_Info;

typedef struct _Board_Info
{
    int         total_pot;
    int         big_blind;
    int         max_jetton_this_turn;
    int         our_max_blind_this_turn;
    __int64     cards;
} Board_Info;

extern Board_Info g_board;

extern Player_Info g_our;

extern char g_opponent_count;
extern Player_Info g_opponent[MAX_PLAYER];

extern int g_board_total_pot;
extern __int64 g_board_cards;

extern int msg_null			(char *, char *);
extern int msg_seat_info	(char *, char *);
extern int msg_game_over	(char *, char *);
extern int msg_blind		(char *, char *);
extern int msg_hold_card	(char *, char *);
extern int msg_inquire		(char *, char *);
extern int msg_flop			(char *, char *);
extern int msg_turn			(char *, char *);
extern int msg_river		(char *, char *);
extern int msg_showdown		(char *, char *);
extern int msg_pot_win		(char *, char *);
extern int msg_notify		(char *, char *);

extern double handprocess_get_firsthand_odds(__int64 player_card, char num_of_player);

#endif // POKER_H
