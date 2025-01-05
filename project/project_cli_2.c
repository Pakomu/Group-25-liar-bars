#include "unp.h"
#include <termios.h>

#define bool int
#define true 1
#define false 0

// TODO: definition of color and window functions
#define RED "\x1b[;31;1m"
#define GREEN "\x1b[;32;1m"
#define YELLOW "\x1b[;33;1m"
#define BLUE "\x1b[;34;1m"
#define PURPLE "\x1b[;35;1m"
#define CYAN "\x1b[;36;1m"
#define WHITE "\x1b[;37;1m"
#define SHINING "\x1b[;32;1m\x1b[6m"
#define HIDECUR "\x1b[?25l"
#define SHOWCUR "\x1b[?25h"
#define CLEARLINE "\x1b[K"

const int title_x = 5, title_y = 44;
const int player_x = 10, player_y = 30;
const int table_x = 20, table_y = 58;
const int card_x = 30, card_y = 38;
const int sys_x = 4, sys_y = 110;
const int cmd_x = 23, cmd_y = 110;

int line_num = 0;
char sys_msg[15][44];
struct player {
        int death;
        int cnt;
        int count_down;
        char name[MAXLINE];
        int isConnected;
        char hand[6];
};

void od_set_cursor(int x, int y) {
        printf("\x1B[%d;%dH", x, y);
        fflush(stdout);
}
void od_clr_scr() { printf("\x1B[2J"); }

void set_cursor_bottom() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        int rows = w.ws_row;
        od_set_cursor(rows, 1);
}

void putxy(int x, int y, char *str, char *color) {
        od_set_cursor(x, y);
        printf("%s%s" WHITE, color, str);
        fflush(stdout);
}

void put_sys_msg(char *str) {
        int index = line_num % 15;
        line_num++;
        strncpy(sys_msg[index], str, sizeof(sys_msg[index]) - 1);
        sys_msg[index][sizeof(sys_msg[index]) - 1] = '\0';
        int start = line_num > 15 ? line_num % 15 : 0;
        for (int i = 0; i < 15; i++) {
                int idx = (start + i) % 15;
                putxy(sys_x + 3 + i, sys_y + 2, "                                          ", YELLOW);
                putxy(sys_x + 3 + i, sys_y + 2, sys_msg[idx], YELLOW);
        }
        set_cursor_bottom();
}

void draw_title(int x, int y) {
        putxy(x + 0, y, "░░▒║░▒║░╔═╗░╔═╗¶▒╔══░░░╔═╗▒╔═╗▒╔═╗▒░░", RED);
        putxy(x + 1, y, "░░▒║░▒║▒╬═╣▒╠╦╝░▒╚═╗░░▒╠═╣▒╬═╣░╠╦╝▒░░", RED);
        putxy(x + 2, y, "░░▒╚═▒║▒║░║▒║╚═░░══╝░░▒╚═╝░║▒║▒║╚═░░░", RED);
}

void draw_sys_msg_board(int x, int y) {
        putxy(x + 0, y, "┌────────────────────────────────────────────┐", WHITE);
        putxy(x + 1, y, "│               System Message               │", WHITE);
        putxy(x + 2, y, "├────────────────────────────────────────────┤", WHITE);
        for (int i = 3; i < 18; i++) {
                putxy(x + i, y, "│                                            │", WHITE);
        }
        putxy(x + 18, y, "└────────────────────────────────────────────┘", WHITE);
}

void draw_cmd_board(int x, int y) {
        putxy(x + 0, y, "┌───────────────────────────────────┐", WHITE);
        putxy(x + 1, y, "│ [  1  ] Choose cards to drop      │", WHITE);
        putxy(x + 2, y, "│ [  2  ] Doubt player at your left │", WHITE);
        putxy(x + 3, y, "│ [ A/D ] Switch candicate card     │", WHITE);
        putxy(x + 4, y, "│ [  C  ] Choose the card           │", WHITE);
        putxy(x + 5, y, "│ [  F  ] Drop the chosen card      │", WHITE);
        putxy(x + 6, y, "│                                   │", WHITE);
        putxy(x + 7, y, "│ [  Q  ] Quit Game                 │", WHITE);
        putxy(x + 8, y, "└───────────────────────────────────┘", WHITE);
}

void draw_information(int x, int y,struct player p) {
        char string_name[2 * MAXLINE];
        sprintf(string_name, "Name: %s",p.name);
        char string_death[MAXLINE];
        sprintf(string_death, "Death: %d / 6",p.cnt);
        putxy(x + 0, y, string_name, WHITE);
        putxy(x + 1, y, string_death, WHITE);
        putxy(x + 2, y, "───────────────────────────────────", WHITE);
}

void draw_card(int x, int y, char *color, char card) {
        switch (card) {
        case 'A':
                putxy(x + 0, y, "┌────────┐", color);
                putxy(x + 1, y, "│  ◢◤◥◣  │", color);
                putxy(x + 2, y, "│  █▄▄█  │", color);
                putxy(x + 3, y, "│  █  █  │", color);
                putxy(x + 4, y, "│  █  █  │", color);
                putxy(x + 5, y, "└────────┘", color);
                break;
        case 'Q':
                putxy(x + 0, y, "┌────────┐", color);
                putxy(x + 1, y, "│  ◢██◣  │", color);
                putxy(x + 2, y, "│  █  █  │", color);
                putxy(x + 3, y, "│  █  █  │", color);
                putxy(x + 4, y, "│  ◥██▀▄ │", color);
                putxy(x + 5, y, "└────────┘", color);
                break;
        case 'K':
                putxy(x + 0, y, "┌────────┐", color);
                putxy(x + 1, y, "│  █ ◢◤  │", color);
                putxy(x + 2, y, "│  █◢◤   │", color);
                putxy(x + 3, y, "│  █▜▌   │", color);
                putxy(x + 4, y, "│  █ ◥◣  │", color);
                putxy(x + 5, y, "└────────┘", color);
                break;
        case 'J':
                putxy(x + 0, y, "┌────────┐", color);
                putxy(x + 1, y, "│  ▄▄▄▄  │", color);
                putxy(x + 2, y, "│   ▐▌   │", color);
                putxy(x + 3, y, "│   ▐▌   │", color);
                putxy(x + 4, y, "│  ▙▟▌   │", color);
                putxy(x + 5, y, "└────────┘", color);
                break;
        }
}

void draw_player(int x, int y, struct player player, char *color) {
        char info[2 * MAXLINE];
        int n = sprintf(info, "%s: %d/6", player.name, player.cnt);
        info[n] = 0;
        putxy(x + 0, y - (strlen(info) / 2 - 3), info, color);
        putxy(x + 1, y, " ╭──╮", color);
        putxy(x + 2, y, " │  │", color);
        putxy(x + 3, y, "╭╰──╯╮", color);
        putxy(x + 4, y, "│    │", color);
        putxy(x + 5, y - 2, "          ", color);
        putxy(x + 6, y - 2, "          ", color);
        switch (strlen(player.hand)) {
        case 1:
                putxy(x + 5, y + 2, "┌┐", color);
                putxy(x + 6, y + 2, "└┘", color);
                break;
        case 2:
                putxy(x + 5, y + 1, "┌┐┌┐", color);
                putxy(x + 6, y + 1, "└┘└┘", color);
                break;
        case 3:
                putxy(x + 5, y + 0, "┌┐┌┐┌┐", color);
                putxy(x + 6, y + 0, "└┘└┘└┘", color);
                break;
        case 4:
                putxy(x + 5, y - 1, "┌┐┌┐┌┐┌┐", color);
                putxy(x + 6, y - 1, "└┘└┘└┘└┘", color);
                break;
        case 5:
                putxy(x + 5, y - 2, "┌┐┌┐┌┐┌┐┌┐", color);
                putxy(x + 6, y - 2, "└┘└┘└┘└┘└┘", color);
                break;
        default:
                putxy(x + 5, y - 2, "          ", color);
                putxy(x + 6, y - 2, "          ", color);
                break;
        }
}

void draw_all_player(int x, int y, int playerid, int turn, struct player *p) {
        int right = (playerid + 1) % 4, mid = (playerid + 2) % 4, left = (playerid + 3) % 4;
        char *right_color = (right == turn) ? RED : WHITE;
        char *mid_color = (mid == turn) ? RED : WHITE;
        char *left_color = (left == turn) ? RED : WHITE;
        draw_player(x + 0, y + 30, p[mid], mid_color);
        draw_player(x + 8, y + 0, p[left], left_color);
        draw_player(x + 8, y + 60, p[right], right_color);
}

void draw_all_card(int x, int y, struct player player, int isChoose[], int cardpointer) {
        for (int i = -3; i < 6; i++) {
                putxy(x + i, y, "                                                   ", WHITE);
        }
        int ncard = strlen(player.hand);
        int start_point = y + 25 - 5 * ncard;
        for (int i = 0; i < ncard; i++) {
                int x_pos = x, y_pos = start_point + 10 * i;
                if (i == cardpointer) {
                        x_pos -= 3;
                }
                if (isChoose[i]) {
                        draw_card(x_pos, y_pos, RED, player.hand[i]);
                } else {
                        draw_card(x_pos, y_pos, WHITE, player.hand[i]);
                }
        }
}

void init_window() {
        draw_title(title_x, title_y);
        draw_sys_msg_board(sys_x, sys_y);
        draw_cmd_board(cmd_x, cmd_y);
}

// TODO: definition of flags and game function
// flag
const int INIT = 0;
const int PLAYER_INFO = 1;
const int TURN = 2;
const int TABLE = 3;
const int DOUBT = 4;
const int DEATH = 5;
const int WIN = 6;
const int QUIT = 7;
const int REMAIN = 8;

char id[MAXLINE];
int playerid;
int turn = -1;
int prev_turn = -1;
int gameflag = 0;
int action_flag = 0;
int remain_flag = 0;
char deskcard[6];
char News[MAXLINE / 2];
int ne = -1;
char temp_news[MAXLINE / 2];
int tn = -1;
int QUIT_NOT_SEND = 0;
char card_decision[MAXLINE];
struct player p[4];
void print_status_player_ver(int index, int f, char table);
int update_hand(char *hand, char *selected);
void clr_scr() { printf("\x1B[2J"); };

void set_scr() { // set screen to 80 * 25 color mode
        printf("\x1B[=3h");
};
void player_information(int index) {
        printf("-------------------------------------------------\n");
        printf("Name: %s\n", id);
        printf("#%d player.\n", index);
}

int getch() {
        struct termios oldt, newt;
        int ch;
        int oldf;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        fd_set set;
        struct timeval timeout;

        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        timeout.tv_sec = 5;
        timeout.tv_usec = 1000;

        if (select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout) > 0) {
                ch = getchar();
        } else {
                ch = EOF;
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        return ch;
}


void handle_action(int flag, char *recvline, char *table, int sockfd) {
        int index, death, count, count_down, connected;
        int prev_index, outcome, winner;
        int none = 0;
        char concateString[MAXLINE / 2];
        char ttemp[MAXLINE / 4];
        //put_sys_msg("handle\n");
        switch (flag) {
        case 0: // new game initial
                remain_flag = 0;
                memset(p, 0, (4 * sizeof(struct player)));
                memset(deskcard, 0, sizeof(deskcard));

                sscanf(recvline, "%d %s", &none, table);
                tn = sprintf(temp_news, "A new game round start.\n");
                put_sys_msg(temp_news);
                strcat(News, temp_news);
                memset(temp_news, 0, sizeof(temp_news));
                break;
        case 1: // send or update the player information

                sscanf(recvline, "%d %d %d %d %d %d %s", &none, &index, &death, &count, &count_down, &connected, concateString);
                p[index].cnt = count;
                p[index].death = death;
                p[index].count_down = count_down;
                p[index].isConnected = connected;
                char *t = strtok(concateString, ",");
                if (strcmp(t, "no") == 0) {
                        strcpy(p[index].hand, "");
                } else {
                        strcpy(p[index].hand, t);
                }
                p[index].hand[5] = '\0';
                // strcpy(p[index].hand, strtok(concateString, ","));
                t = strtok(NULL, ",");
                strcpy(p[index].name, t);
                p[index].name[MAXLINE - 1] = '\0';
                prev_turn = -1;
                // strcpy(p[index].name, strtok(NULL, ",");
                break;
        case 2: // information the player's turn
                sscanf(recvline, "%d %d", &none, &turn);

                if (turn == playerid) { // it' s your turn and print something.
                        action_flag = 1;
                } else {
                        tn = sprintf(temp_news, "It's %s's turn.\n", p[turn].name);
                        put_sys_msg(temp_news);
                }
                
                memset(temp_news, 0, sizeof(temp_news));
                break;
        case 3: // CARD: CARD index DOUBT
                sscanf(recvline, "%d %d %s", &none, &index, deskcard);

                tn = sprintf(temp_news, "%s just put %ld card.\n", p[index].name, strlen(deskcard));
                put_sys_msg(temp_news);
                memset(temp_news, 0, sizeof(temp_news));

                prev_turn = turn;
                break;
        case 4: // doing doubt: (doubt_flag, turn index, previous turn index, doubt outcome)
                sscanf(recvline, "%d %d %d %d", &none, &index, &prev_index, &outcome);
                /*
                printf("Player %s just doubted Player %s\n\n The doubt outcome is ",p[index]);
                */
                tn = sprintf(temp_news, "%s just doubted %s\n", p[index].name, p[prev_index].name);
                put_sys_msg(temp_news);
                memset(temp_news, 0, sizeof(temp_news));
                if (outcome){
                        sprintf(temp_news, "%s  doubted %s failed.\n",p[index].name,p[prev_index].name);
                        put_sys_msg(temp_news);
                }    
                else{
                        sprintf(temp_news, "%s  doubted %s successfully.\n",p[index].name,p[prev_index].name);
                        put_sys_msg(temp_news);
                }
                memset(temp_news, 0, sizeof(temp_news));

                break;
        case 5: // player death
                sscanf(recvline, "%d %d", &none, &death);
                tn = sprintf(temp_news, "%s death.\n", p[death].name);
                put_sys_msg(temp_news);

                memset(temp_news, 0, sizeof(temp_news));
                p[death].death = 1;
                break;
        case 6: // win game over. fd close.
                sscanf(recvline, "%d %d", &none, &winner);
                sprintf(temp_news, "%s win.\n", p[winner].name);
                put_sys_msg(temp_news);
                putxy(card_x + 7, card_y, temp_news, SHINING);
                sleep(5);
                break;
        case 7: // Some player quit. the game keep going in without quit player.
                sscanf(recvline, "%d %d", &none, &index);
                sprintf(temp_news, "%s Quit the game.\n", p[index].name);
                put_sys_msg(temp_news);
                QUIT_NOT_SEND = 1;
                p[index].death = 1;
                p[index].isConnected = 0;
                break;
        case 8: // The player can only doubt.
                remain_flag = 1;
                break;
        case 9: // Game start:
                gameflag = 1;
                break;
        case 10:
                sscanf(recvline, "%d %d (%[^)])", &none, &index, ttemp);
                put_sys_msg(ttemp);
                break;
        }
}
int handle_stdin_action_client_version(int *curState, char table,int sockfd,  int *action_flag) {
        fflush(stdout);
        char sendline[MAXLINE];
        int n;
        int action;
        int endloop = 0;
        int choose_card[5] = {0};
        put_sys_msg("It's your turn. ");
        for (;;) { // big loop until whole action select end.
                draw_all_player(player_x, player_y, playerid, turn, p);
                draw_card(table_x, table_y, WHITE, table);
                draw_all_card(card_x, card_y, p[playerid], choose_card, -1);
                draw_information(cmd_x + 10, cmd_y, p[playerid]);
                fflush(stdout);
                if (endloop == 1) {
                        
                        break;
                }
                if (*curState == 0) {  

                        while (1) { // loop the correct action choose.
                                action = getch();
                                if (action == '1') {
                                        if (remain_flag == 1) { // doubt
                                                *curState = 0;
                                                //printf("You cannot choose doubt. Please try again.\n");
                                                put_sys_msg("You cannot choose doubt. Please try again.\n");
                                                fflush(stdout);
                                        } else { // into the second status.
                                                *curState = 1;
                                        }
                                        //printf(" choose action 1\n");
                                        put_sys_msg("choose action 1.\n");
                                        fflush(stdout);
                                        break;
                                } else if (action == '2') {
                                        if (prev_turn == -1) { // the first player of the new game round.
                                                //printf("You cannot choose doubt. Please try again.\n");
                                                put_sys_msg("You cannot choose doubt. Please try again.\n");
                                                fflush(stdout);
                                        } 
                                        else { // doubt
                                                *curState = 0;
                                                n = sprintf(sendline, "%d %d\n", 2,playerid);
                                                sendline[n] = '\0';
                                                Writen( sockfd, sendline, n + 1);
                                                *action_flag = 0;
                                                endloop = 1;
                                                //printf("choose action 2\n");
                                                put_sys_msg("choose action 2.\n");
                                                fflush(stdout);
                                                break;
                                        }
                                } else if (action == 'Q' || action == 'q'){
                                        
                                        return -1;
                                }
                        }
                } else if (*curState == 1) {
                        fflush(stdout);
                        int choose_card_action;
                        int cardpointer = 0;
                        int cardslength = strlen(p[playerid].hand);
                        for (int i = 0; i < 5; ++i) {
                                choose_card[i] = 0;
                        }
                        while (1) {
                                draw_all_card(30, 38, p[playerid], choose_card, cardpointer);
                                //sprintf(sysline, "cardpointer: %d\n", cardpointer);
                                //put_sys_msg(sysline);
                                // for (int i = 0; i < 5; ++i) {
                                //         printf("%d ", choose_card[i]);
                                // }
                                // printf("\n");
                                /*
                                for(int i = 0;i < 5; ++i){
                                        if(choose_card[i] == 1){
                                                printf("%c", p[playerid].hand[i]);
                                                fflush(stdout);
                                        }
                                }
                                
                                printf("\n");
                                fflush(stdout);
                                */
                                choose_card_action = getch();

                                if (choose_card_action == 'A' || choose_card_action == 'a') { // point to the next left card.
                                        if (cardpointer != 0) {
                                                cardpointer--;
                                        }
                                } else if (choose_card_action == 'D' || choose_card_action == 'd') { // point to the next right card
                                        if (cardpointer != (cardslength - 1)) {
                                                cardpointer++;
                                        }
                                } else if (choose_card_action == 'C' || choose_card_action == 'c') { // choose / cancel the card
                                        if (choose_card[cardpointer] == 0) {
                                                choose_card[cardpointer] = 1;
                                                // printf("choose card index: %d, card: %s\n", cardpointer,
                                                // hand[cardpointer]);
                                        } else {
                                                choose_card[cardpointer] = 0;
                                                // printf("cancel card index: %d, card: %s\n", cardpointer,
                                                // hand[cardpointer]);
                                        }
                                } else if (choose_card_action == 'F' || choose_card_action == 'f') { // final decision.
                                        int choose_count = 0;
                                        for (int i = 0; i < 5; ++i) {
                                                if(choose_card[i] == 1){
                                                        choose_count++;
                                                }
                                        }
                                        if(choose_count < 1 || choose_count > 3){
                                                put_sys_msg("Invalid Choose. Please try again\n");
                                        }
                                        else{
                                                break;
                                        }
                                        
                                }else if (action == 'Q' || action == 'q'){
                                        
                                        return -1;
                                }
                                //printf("choose_card pointer: %d\n", cardpointer);
                                //printf("choose_card: %c\n", p[playerid].hand[cardpointer]);
                        }
                        char selected_card[5];
                        int index = 0;
                        for (int i = 0; p[playerid].hand[i] != 0; ++i) {
                                if (choose_card[i] == 1) {
                                        selected_card[index] = p[playerid].hand[i];
                                        index++;
                                }
                        }
                        selected_card[index] = 0;
                        //sprintf(sysline, "selected card: %s\n", selected_card);
                        //put_sys_msg(sysline);
                        //sprintf(sysline, "hand: %s\n", p[playerid].hand);
                        //put_sys_msg(sysline);
                        if (update_hand(p[playerid].hand, selected_card) < 0) {
                                //printf("Invalid Choose. Please try again\n");
                                put_sys_msg("Invalid Choose. Please try again\n");
                                index = 0;
                                memset(selected_card, 0, sizeof(selected_card));
                        } else {
                        	
                                n = sprintf(sendline, "%d %d %s %s\n", 1, playerid,selected_card, p[playerid].hand);
                                sendline[n] = '\0';
                                
                                Writen(sockfd, sendline, n + 1);
                                memset(sendline, 0, sizeof(sendline));
                                
                                *curState = 0;
                                *action_flag = 0;
                                remain_flag = 0;
                                endloop = 1;
                                for (int i = 0; i < 5; i++) {
                                        choose_card[i] = 0;
                                }
                                break;
                        }
                }
        }
        return 1;
}

int update_hand(char *hand, char *selected) {
        int choose[5] = {0};
        for (int i = 0; selected[i] != 0; i++) {
                if (i > 2) {
                        return -1;
                }
                for (int j = 0; hand[j] != 0; j++) {
                        if (selected[i] == hand[j] && !choose[j]) {
                                choose[j] = 1;
                                break;
                        }
                        if (hand[j + 1] == 0) {
                                return -1;
                        }
                }
        }
        char new_hand[6];
        int idx = 0;
        for (int i = 0; hand[i] != 0; i++) {
                if (!choose[i]) {
                        new_hand[idx] = hand[i];
                        idx++;
                }
        }
        new_hand[idx] = 0;
        strcpy(hand, new_hand);
        if (strlen(hand) == 0) {
                strcpy(hand, "no");
        }
        return 0;
}

void xchg_data(FILE *fp, int sockfd) {
        int maxfdp1, stdineof, peer_exit, n;
        fd_set rset;
        char sendline[MAXLINE], recvline[MAXLINE], read_smallline[MAXLINE / 2];

        set_scr();
        clr_scr();
        init_window();
        
        Writen(sockfd, id, strlen(id));
        // printf("sent: %s\n", id);
        readline(sockfd, read_smallline, MAXLINE);
        sprintf(recvline, "recv: %s\n", read_smallline);
        put_sys_msg(recvline);
        sscanf(recvline, "recv: You are the #%d user.\n", &playerid);
        playerid--;
        readline(sockfd, read_smallline, MAXLINE);
        sprintf(recvline, "recv: %s\n", read_smallline);;
        put_sys_msg(recvline);
        /*
        char *test_txt = "test the msg\n";
        put_sys_msg(test_txt);
        */
        char table;
        memset(deskcard, 0, sizeof(deskcard));
        stdineof = 0;
        peer_exit = 0;
        int flag = -1;
        int curState = 0;
        for (int i = 0; i < 4; ++i) {
                p[i].death = 0;
                p[i].cnt = 0;
                p[i].count_down = 0;
                p[i].isConnected = 0;
                memset(p[i].hand, 0, sizeof(p[i].hand));
                memset(p[i].name, 0, sizeof(p[i].name));
        }
        for (;;) {

                int allcardchoose[6];
                for(int i = 0;i < 5;++i){
                        allcardchoose[i] = 0;
                }
                draw_all_card(30, 38, p[playerid], allcardchoose, -1);
                draw_all_player(player_x, player_y, playerid, turn, p);
                draw_card(table_x, table_y, WHITE, table);
                draw_information(cmd_x + 10, cmd_y, p[playerid]);
                set_cursor_bottom();
                
                FD_ZERO(&rset);
                maxfdp1 = 0;
                if (stdineof == 0) {
                        FD_SET(fileno(fp), &rset);
                        maxfdp1 = fileno(fp);
                }
                if (peer_exit == 0) {
                        FD_SET(sockfd, &rset);
                        if (sockfd > maxfdp1)
                                maxfdp1 = sockfd;
                }
                maxfdp1++;
                Select(maxfdp1, &rset, NULL, NULL, NULL);
                if (FD_ISSET(sockfd, &rset)) { /* socket is readable */
                        n = read(sockfd, recvline, MAXLINE);
                        if (n <= 0) {
                                if (stdineof == 1) {
                                        put_sys_msg("Game over.\n");
                                        
                                        return; /* normal termination */
                                } else {
                                        put_sys_msg("(End of input from the peer!)\n");
                                        peer_exit = 1;
                                        return;
                                }
                        } else if (n > 0) {
                                recvline[n] = '\0';
                                char testline[3 * MAXLINE] = "(test):";
                                strcat(testline, recvline);
                                //put_sys_msg(testline);
                                //printf("\x1B[0;36m%s\x1B[0m", recvline);
                                char *curLine = recvline;
                                while (curLine) {
                                        char *nextLine = strchr(curLine, '\n');
                                        if (nextLine)
                                                *nextLine = '\0'; // temporarily terminate the current line
                                        //printf("curLine=[%s]\n", curLine);
                                        if (strlen(curLine) == 0) {
                                                break;
                                        } else {
                                                sscanf(curLine, "%d", &flag);
                                                handle_action(flag, curLine, &table, sockfd);
                                        }
                                        if (nextLine)
                                                *nextLine = '\n'; // then restore newline-char, just to be tidy
                                        curLine = nextLine ? (nextLine + 1) : NULL;
                                }
                                memset(recvline, 0, sizeof(recvline));
                        }
                }
                if (FD_ISSET(fileno(fp), &rset)) { /* input is readable */
                        n = Read(fileno(stdin), recvline, MAXLINE);
                        recvline[n - 1] = '\0';
                        if (n <= 0) {
                                if (peer_exit)
                                        return;
                                else {
                                        put_sys_msg("(leaving...)\n");
                                        stdineof = 1;
                                        Shutdown(sockfd, SHUT_WR); /* send FIN */
                                }
                        } else {
                                // int l = snprintf(sendline, MAXLINE, "%d %d (%s)", 10, playerid, recvline);
                                // sendline[l] = '\0';
                                // Writen(sockfd, sendline, l + 1);
                                //TODO
                                //put_sys_msg(recvline);
                                /*
                                printf("from stdin: %s\n", recvline);
                                printf("Game flag: %d, action flag: %d\n", gameflag, action_flag);
                                if (gameflag == 1) {
                                        if (action_flag == 1) {
                                                handle_stdin_action(&curState, recvline, table, sockfd, &action_flag);
                                        } else {
                                                char *p;
                                                int l = snprintf(sendline, MAXLINE, "%d %d %s", 10, playerid, recvline);
                                                sendline[l] = '\0';
                                                Writen(sockfd, sendline, l + 1);
                                        }
                                }
                                */
                        }
                        memset(recvline, 0, sizeof(recvline));
                }
                if (turn == playerid && action_flag == 1) {
                        if(handle_stdin_action_client_version(&curState,table,sockfd, &action_flag) < 0){
                                close(sockfd);
                                return;
                        }
                        
                        continue;
                }
        }
}

int main(int argc, char **argv) {
        int sockfd;
        struct sockaddr_in servaddr;

        if (argc != 3)
                err_quit("usage: tcpcli <IPaddress> <ID>");

        sockfd = Socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(SERV_PORT + 25);
        Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
        strcpy(id, argv[2]);

        Connect(sockfd, (SA *)&servaddr, sizeof(servaddr));

        xchg_data(stdin, sockfd); /* do it all */

        exit(0);
}
