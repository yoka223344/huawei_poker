#include <stdio.h>
#include <string.h>

#include <Windows.h>

#include <semaphore.h>

#include "hunter.h"
#include "poker.h"

#define MAX_MSG_LEN			1024	// t66y.com
#define MAX_PLAYER_NAME		32

#define MAX_MSG_BUFFER		8*1024	// bytes

extern Action_Table action_table[];

Board_Info g_board;

Player_Info g_our;

char g_opponent_count;
Player_Info g_opponent[MAX_PLAYER];

static sem_t sem_hunter_message;
static Hunter_Message hunter_message;	// store the message queue (len=1) between threads

const Msg_Table msg_table[] = {
    { MSG_TYPE_NONE			, MSG_STR_H_NA			, msg_null		},
    { MSG_TYPE_REG			, MSG_STR_H_NA			, msg_null		},
    { MSG_TYPE_SEAT_INFO	, MSG_STR_H_SEAT_INFO	, msg_seat_info	},
    { MSG_TYPE_GAME_OVER	, MSG_STR_H_GAME_OVER	, msg_game_over	},
    { MSG_TYPE_BIND			, MSG_STR_H_BIND		, msg_blind		},
    { MSG_TYPE_HOLD_CARD	, MSG_STR_H_HOLD_CARDS	, msg_hold_card	},
    { MSG_TYPE_INQUIRE		, MSG_STR_H_INQUIRY		, msg_inquire	},
    { MSG_TYPE_ACTION		, MSG_STR_H_NA			, msg_null		},
    { MSG_TYPE_FLOP			, MSG_STR_H_FLOP		, msg_flop		},
    { MSG_TYPE_TURN			, MSG_STR_H_TURN		, msg_turn		},
    { MSG_TYPE_RIVER		, MSG_STR_H_RIVER		, msg_river		},
    { MSG_TYPE_SHOWDOWN		, MSG_STR_H_SHOWDOWN	, msg_showdown	},
    { MSG_TYPE_POT_WIN		, MSG_STR_H_POT_WIN		, msg_pot_win	},
    { MSG_TYPE_NOTIFY		, MSG_STR_H_NOTIFY		, msg_notify	},
};

 /*
 * The memmem() function finds the start of the first occurrence of the
 * substring 'needle' of length 'nlen' in the memory area 'haystack' of
 * length 'hlen'.
 *
 * The return value is a pointer to the beginning of the sub-string, or
 * NULL if the substring is not found.
 */
static void *memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen)
{
    int needle_first;
    const void *p = haystack;
    size_t plen = hlen;

    if (!nlen)
        return NULL;

    needle_first = *(unsigned char *)needle;

    while (plen >= nlen && (p = memchr(p, needle_first, plen - nlen + 1)))
    {
        if (!memcmp(p, needle, nlen))
            return (void *)p;

        //p++;
        p = ((char *)p) + 1;
        //plen = hlen - (p - haystack);
        plen = hlen - ((char *)p - haystack);
    }

    return NULL;
}

void game_incoming_msg(Msg_Type msg_type, char *msg_start, char *msg_end)
{
    char *msg_line;

#if 0
    char buffer[1024];
    int buffer_len;

    buffer_len = msg_end - msg_start;

    if (buffer_len > 1024)
    {
        while (1)
            ;
    }

    strncpy(buffer, msg_start, buffer_len);
    buffer[buffer_len] = '\0';

    log(LOG_GENERIC, "===INCOMING MSG===");
#endif

    // parse message first:
    if (msg_type > MSG_TYPE_NONE)
    {
        log(LOG_GENERIC, "Got: %s", msg_table[msg_type].msg_str);
        (*msg_table[msg_type].msg_func)(msg_start, msg_end);
    }

    // process after parse action:
    switch (msg_type)
    {
    case MSG_TYPE_NONE:
        break;
    case MSG_TYPE_INQUIRE:
        // send a the inquire message
        send_hunter_message(HUNT_MSG_ACTION);
        break;
    default:
        break;
    }

#if 0	// print received packet:
    fprintf(stdout, "%s\r\n", buffer);

    //Sleep(250);
#endif

    return ;
}

void sock_receive(char *message, int length)
{
    static char msg_buffer[MAX_MSG_BUFFER];
    static char *msg_buffer_tail = msg_buffer;

    char is_msg_found;

    int i_msg_table;

    char msg_header[MAX_MSG_STR_NAME], msg_tail[MAX_MSG_STR_NAME];
    char *msg_start_pos, *msg_end_pos, *msg_remaint_start_pos;
    int remaint_len;

    if (length <= 0)
    {
        goto done;
    }

    // simply cat to the end of message buffer
    if ((msg_buffer_tail + length) < (msg_buffer + MAX_MSG_BUFFER))
    {
        memcpy(msg_buffer_tail, message, length);
        msg_buffer_tail += length;
    }

    do
    {
        is_msg_found = 0;

        // try to process from the start of message buffer
        for (i_msg_table = 0; i_msg_table < (sizeof(msg_table)/sizeof(Msg_Table)); i_msg_table ++)
        {
            sprintf(msg_header	, "%s/ ", msg_table[i_msg_table].msg_str);
            sprintf(msg_tail	, "/%s ", msg_table[i_msg_table].msg_str);

            msg_start_pos	= (char *)memmem(msg_buffer, (msg_buffer_tail - msg_buffer), msg_header	, strlen(msg_header	));
            msg_end_pos		= (char *)memmem(msg_buffer, (msg_buffer_tail - msg_buffer), msg_tail	, strlen(msg_tail	));

            if ((msg_start_pos == NULL) || (msg_end_pos == NULL))
            {
                continue;
            }

            msg_start_pos += (strlen(msg_header) + 1);	// + 1 to reject \n

            if (msg_start_pos >= msg_end_pos)
            {
                continue;
            }

            is_msg_found = 1;

            // valid msg found, call upper layer function:
            game_incoming_msg(msg_table[i_msg_table].msg_type, msg_start_pos, msg_end_pos);

            msg_remaint_start_pos	= msg_end_pos + strlen(msg_tail) + 1;
            remaint_len				= msg_buffer_tail - msg_remaint_start_pos;

            if (remaint_len < 0)
            {
                while (1)
                    ;
            }

            // remove the msg from buffer:
            memmove(msg_buffer, msg_remaint_start_pos, remaint_len);
            msg_buffer_tail = msg_buffer + remaint_len;

            break;
        }
    } while ((is_msg_found == 1) && (remaint_len > 0));

done:
    return ;
}

void game_register()
{
#ifdef V124	// adapt the new 124 version server
    // reg: pid pname need_notify eol
#define MSG_STR_REG			"reg: %s %s need_notify \n"
#else
#define MSG_STR_REG			"reg: %d %s \n"
#endif
    char msg_buffer[MAX_MSG_LEN];

    sprintf(msg_buffer, MSG_STR_REG, g_our.player_id, g_our.player_name);

    sock_send(msg_buffer, strlen(msg_buffer));

    return ;
}

void game_action()
{
    char msg_buffer[MAX_MSG_LEN];

    // we should reply < 500ms
    double odds;
    int current_jetton;

    Action_Type current_action;

#if 1
    // get handprocess result:
    if (g_board.cards == 0)	// cards not shown
    {
        odds = handprocess_get_firsthand_odds(g_our.cards, g_opponent_count);
    }
    else
    {
        //odds = handprocess_get_odds();
    }

    // call team leader's function to feed results:
    current_jetton = 0;

    if (current_action == ACTION_RAISE)
    {
        sprintf(msg_buffer, "%s %d", action_table[ACTION_RAISE].action_str, current_jetton);
    }
    else
    {
        sprintf(msg_buffer, "%s", action_table[current_action].action_str);
    }

    g_board.max_jetton_this_turn += current_jetton;
#else
    //next_action = ACTION_BLIND;
    current_action = ACTION_CALL;
    //next_action = ACTION_ALL_IN;
    //next_action = ACTION_FOLD;

    //if (g_board_cards == 0)	// cards not shown
    {
        int odds_num;

        odds = handprocess_get_firsthand_odds(g_our.cards, g_opponent_count);

        odds_num = odds;

        if (odds_num > 60)
        {
            current_action = ACTION_ALL_IN;
            log(LOG_GENERIC, "ALL_IN!!!!!!!!!");
        }
    }

    strcpy(msg_buffer, action_table[current_action].action_str);
#endif

    log(LOG_GENERIC, "Sent: %s", action_table[current_action].action_str);

    sock_send(msg_buffer, strlen(msg_buffer));

    return ;
}

void game_over()
{
    // the server will auto disconnect us:

    return ;
}

void send_hunter_message(Hunter_Message new_message)
{
    hunter_message = new_message;
    sem_post(&sem_hunter_message);

    return ;
}

void game_start(int player_id)
{
    static Hunter_Status hunter_status;

    g_our.player_id = player_id;
    sprintf(g_our.player_name, "%s%d", HUNT_PLAYER_NAME, player_id);

    log(LOG_GENERIC, "%s", g_our.player_name);

    //hunter_status = STAT_WAIT_FOR_CONNECT;
    sem_init(&sem_hunter_message, 0, 0);

    // try to obtain the semaphore, once we got the semaphore,
    // check the state machine to figure out what to do next
    while (1)
    {
        sem_wait(&sem_hunter_message);

        switch (hunter_message)
        {
        case HUNT_MSG_CONNECTED:
            game_register();
            break;
        case HUNT_MSG_ACTION:
            game_action();
            break;
        case HUNT_MSG_GAMEOVER:
            game_over();
        default:
            log(LOG_ERROR, "Unknown message");
            break;
        }
    }

    return ;
}
