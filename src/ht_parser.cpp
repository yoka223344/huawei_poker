#include <stdio.h>
#include <string.h>

#include <Windows.h>

#include "hunter.h"
#include "poker.h"

#define MAX_CARD_INFO_STR_LEN	16

#define CARD_COLOR_CLUBS		"CLUBS"
#define CARD_COLOR_DIAMONDS		"DIAMONDS"
#define CARD_COLOR_SPADES		"SPADES"
#define CARD_COLOR_HEARTS		"HEARTS"

extern __int64 CardMasksTable[];

char card_color_table[][16] = {
    CARD_COLOR_CLUBS, CARD_COLOR_DIAMONDS, CARD_COLOR_SPADES, CARD_COLOR_HEARTS,
};

char card_point_table[][4] = {
    "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A",
};

Action_Table action_table[] = {
    { ACTION_BLIND	, ACT_STR_BIND	,	},
    { ACTION_CHECK	, ACT_STR_CHECK	,	},
    { ACTION_CALL	, ACT_STR_CALL	,	},
    { ACTION_RAISE	, ACT_STR_RAISE	,	},
    { ACTION_ALL_IN	, ACT_STR_ALL_IN,	},
    { ACTION_FOLD	, ACT_STR_FOLD	,	},
};

__int64 get_cards(char *color, char *point)
{
    __int64 cards;

    char i_card_color, i_card_point;

    // strip out card color:
    for (i_card_color = 0; i_card_color < 4; i_card_color ++)
    {
        if (strcmp(color, card_color_table[i_card_color]) == 0)
        {
            break;
        }
    }

    // strip out card point:
    for (i_card_point = 0; i_card_point < 13; i_card_point ++)
    {
        if (strcmp(point, card_point_table[i_card_point]) == 0)
        {
            break;
        }
    }

    cards = CardMasksTable[i_card_color + i_card_point];

    return cards;
}

Player_Info *get_player_by_name(char *player_name)
{
    Player_Info *player_info = NULL;

    char i_player;

    for (i_player = 0; i_player < g_opponent_count; i_player ++)
    {
        if (strcmp(player_name, g_opponent[i_player].player_name) == 0)
        {
            player_info = g_opponent;
            break;
        }
    }

    return player_info;
}

Player_Info *get_player_by_id(int player_id)
{
    Player_Info *player_info = NULL;

    char i_player;

    if (g_our.player_id == player_id)
    {
        player_info = &g_our;
    }
    else
    {
        for (i_player = 0; i_player < g_opponent_count; i_player ++)
        {
            if (player_id == g_opponent[i_player].player_id)
            {
                player_info = g_opponent;
                break;
            }
        }
    }

    return player_info;
}


int msg_null(char *msg_start, char *msg_end)
{
    return 0;
}

int msg_seat_info(char *msg_start, char *msg_end)
{
#define MSG_SEAT_INFO_BUTTON		"button"
#define MSG_SEAT_INFO_SMALL_BIND	"small blind"
#define MSG_SEAT_INFO_BIG_BIND		"big blind"

    char *msg_line;

    Player_Info *player_info = g_opponent;

    // clean global var first:
    g_our.cards     = 0;    // we shouldn't clear our's every variable
    g_our.jetton    = 0;
    g_our.money     = 0;

    g_opponent_count = 0;
    memset(g_opponent, 0, sizeof(g_opponent));

    g_board.total_pot = 0;
    g_board.cards = 0;
    g_board.max_jetton_this_turn = 0;

    msg_line = strtok(msg_start, "\n");
    while ((msg_line != NULL) && (msg_line < msg_end))
    {
        if (strncmp(msg_line, MSG_SEAT_INFO_BUTTON, strlen(MSG_SEAT_INFO_BUTTON)) == 0)
        { // button:
            player_info->seat_type = SEAT_BUTTON;
            sscanf(msg_line, "button: %d %d %d ", &player_info->player_id, &player_info->jetton, &player_info->money);
        }
        else if (strncmp(msg_line, MSG_SEAT_INFO_SMALL_BIND, strlen(MSG_SEAT_INFO_SMALL_BIND)) == 0)
        { // small bind:
            player_info->seat_type = SEAT_SMALL_BIND;
            sscanf(msg_line, "small blind: %d %d %d ", &player_info->player_id, &player_info->jetton, &player_info->money);
        }
        else if (strncmp(msg_line, MSG_SEAT_INFO_BIG_BIND, strlen(MSG_SEAT_INFO_BIG_BIND)) == 0)
        {
            player_info->seat_type = SEAT_BIG_BIND;
            sscanf(msg_line, "big blind: %d %d %d ", &player_info->player_id, &player_info->jetton, &player_info->money);

            g_board.big_blind = player_info->jetton;
        }
        else
        {
            player_info->seat_type = SEAT_GENERIC;
            sscanf(msg_line, "%d %d %d ", &player_info->player_id, &player_info->jetton, &player_info->money);
        }
        
        // check if current line player_info is ours:
        if (player_info->player_id == g_our.player_id)
        {
            g_our.jetton	= player_info->jetton;
            g_our.money		= player_info->money;
        }
        else
        { // it's opponent's info
            player_info ++;
            g_opponent_count ++;
        }

        msg_line = strtok(NULL, "\n");
    }

    log(LOG_GENERIC, "jetton=%d, money=%d, opponent=%d", g_our.jetton, g_our.money, g_opponent_count);

    return 0;
}

int msg_game_over(char *msg_start, char *msg_end)
{
#define MSG_GAME_OVER		"game-over"

    char *msg_line;

    Player_Info *player_info = g_opponent;

    msg_line = strtok(msg_start, "\n");

    if (strncmp(msg_line, MSG_GAME_OVER, sizeof(MSG_GAME_OVER)) == 0)
    {
        // end of game
        Sleep(1000000);
    }

    return 0;
}

int msg_blind(char *msg_start, char *msg_end)
{
    char *msg_line;

    Player_Info *player_info = g_opponent;

    int player_id;
    int bet;

    msg_line = strtok(msg_start, "\n");
    while ((msg_line != NULL) && (msg_line < msg_end))
    {
        sscanf(msg_line, "%d: %d ", &player_id, &bet);
        player_info = get_player_by_id(player_id);

        player_info->bet = bet;
        player_info->jetton -= bet;
        if (player_info->jetton < 0)
        {
            log(LOG_ERROR, "jetton = 0");
        }

        msg_line = strtok(NULL, "\n");
    }

    return 0;
}

int msg_hold_card(char *msg_start, char *msg_end)
{
    char *msg_line;

    char card_color[MAX_CARD_INFO_STR_LEN], card_point[MAX_CARD_INFO_STR_LEN];

    msg_line = strtok(msg_start, "\n");
    while ((msg_line != NULL) && (msg_line < msg_end))
    {
        sscanf(msg_line, "%s %s ", card_color, card_point);

        g_our.cards |= get_cards(card_color, card_point);

        msg_line = strtok(NULL, "\n");
    }

    return 0;
}

int msg_inquire(char *msg_start, char *msg_end)
{
#define MSG_INQUIRE_TOTAL_POT		"total pot"
    char *msg_line;

    Player_Info *player_info = g_opponent;

    int player_id, jetton, money, bet;
    char action_str[MAX_ACT_STR_NAME];

    int i_action_type;

    msg_line = strtok(msg_start, "\n");
    while ((msg_line != NULL) && (msg_line < msg_end))
    {
        if (strncmp(msg_line, MSG_INQUIRE_TOTAL_POT, sizeof(MSG_INQUIRE_TOTAL_POT)) == 0)
        {
            sscanf(msg_line, "total pot: %d ", &g_board.total_pot);
        }
        else
        {
            sscanf(msg_line, "%d %d %d %d %s ", &player_id, &jetton, &money, &bet, action_str);

            player_info = get_player_by_id(player_id);

            player_info->jetton = jetton;
            player_info->money	= money;
            player_info->bet		= bet;

            for (i_action_type = 0; i_action_type < 6; i_action_type ++)
            {
                if (strcmp(action_str, action_table[i_action_type].action_str) == 0)
                {
                    player_info->action_type = action_table[i_action_type].action_type;
                    break;
                }
            }

            if (g_board.max_jetton_this_turn < jetton)
            {
                g_board.max_jetton_this_turn = jetton;
            }
        }

        msg_line = strtok(NULL, "\n");
    }

    return 0;
}

int msg_flop(char *msg_start, char *msg_end)
{
    char *msg_line;

    char card_color[MAX_CARD_INFO_STR_LEN], card_point[MAX_CARD_INFO_STR_LEN];

    g_board.max_jetton_this_turn = 0;

    msg_line = strtok(msg_start, "\n");
    while ((msg_line != NULL) && (msg_line < msg_end))
    {
        sscanf(msg_line, "%s %s ", card_color, card_point);

        g_board.cards |= get_cards(card_color, card_point);

        msg_line = strtok(NULL, "\n");
    }

    return 0;
}

int msg_turn(char *msg_start, char *msg_end)
{
    char *msg_line;

    char card_color[MAX_CARD_INFO_STR_LEN], card_point[MAX_CARD_INFO_STR_LEN];

    g_board.max_jetton_this_turn = 0;

    msg_line = strtok(msg_start, "\n");

    sscanf(msg_line, "%s %s ", card_color, card_point);

    g_board.cards |= get_cards(card_color, card_point);

    return 0;
}

int msg_river(char *msg_start, char *msg_end)
{
    g_board.max_jetton_this_turn = 0;

    msg_turn(msg_start, msg_end);

    return 0;
}

int msg_showdown(char *msg_start, char *msg_end)
{
    return 0;
}

int msg_pot_win(char *msg_start, char *msg_end)
{
    char *msg_line;

    Player_Info *player_info = g_opponent;

    int player_id, jetton;

    msg_line = strtok(msg_start, "\n");
    while ((msg_line != NULL) && (msg_line < msg_end))
    {
        sscanf(msg_line, "%d: %d ", &player_id, &jetton);

        log(LOG_GENERIC, "%d got %d", player_id, jetton);

        msg_line = strtok(NULL, "\n");
    }

    return 0;
}

int msg_notify(char *msg_start, char *msg_end)
{
    return 0;
}
