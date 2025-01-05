#include "unp.h"

// TODO: Dealing with game logic
#define capacity 100
const int INIT = 0;
const int PLAYER_INFO = 1;
const int TURN = 2;
const int CARD = 3;
const int DOUBT = 4;
const int DEATH = 5;
const int WIN = 6;
const int QUIT = 7;
const int REMAIN = 8;
const int GAME = 9;
const int MSG = 10;

struct player {
        int death;
        int cnt;
        int count_down;
        char name[MAXLINE];
        int isConnected;
        char hand[5];
};

char card_map[4] = {'A', 'Q', 'K', 'J'};
int map_card(char c) {
        switch (c) {
        case 'A':
                return 0;
        case 'Q':
                return 1;
        case 'K':
                return 2;
        case 'J':
                return 3;
        default:
                return 100;
        }
}
char *initial_deck();
void initialize(int sockfd[4], struct player *p, char *table, int *table_cnt, char **prev_cards);
void sort_hand(char *hand);
void distribute_deck(struct player *p, char *deck);
void send_PLAYER_INFO(int sockfd[4], struct player *p);
int isCardsSame(char *cards, char table);

// TODO: Dealing with child process termination and socket return
struct socketAndfd {
        int idx;
        int fd;
        int isConnected;
        socklen_t len;
        struct sockaddr_in addr;
        char name[MAXLINE];
};
struct socketAndfd client[capacity];

char *initial_deck() {
        char *deck = Malloc(20 * sizeof(char));
        for (int i = 0; i < 20; i++) {
                if (i < 6) {
                        deck[i] = 'A';
                } else if (i < 12) {
                        deck[i] = 'Q';
                } else if (i < 18) {
                        deck[i] = 'K';
                } else {
                        deck[i] = 'J';
                }
        }
        srand(time(NULL));
        for (int i = 0; i < 20; i++) {
                int j = rand() % 20;
                char tmp = deck[i];
                deck[i] = deck[j];
                deck[j] = tmp;
        }
        return deck;
}

void initialize(int sockfd[4], struct player *p, char *table, int *table_cnt, char **prev_cards) {
        *table = card_map[rand() % 3];
        *prev_cards = Malloc(100 * sizeof(char));
        *table_cnt = 0;
        char *deck = initial_deck();

        distribute_deck(p, deck);
        char sendline[MAXLINE];
        int n;
        n = sprintf(sendline, "%d %s\n", INIT, table);
        for (int i = 0; i < 4; i++) {
                if (p[i].isConnected) {
                        sendline[n] = '\0';
                        Writen(sockfd[i], sendline, n);
                }
        }
        send_PLAYER_INFO(sockfd, p);
}

void send_PLAYER_INFO(int sockfd[4], struct player *p) {
        // for each player, send all players' info
        char sendline[MAXLINE];
        int n;
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        char concateString[MAXLINE / 2];
                        if (strlen(p[j].hand) == 0) {
                                sprintf(concateString, "%s,%s", "no", p[j].name);
                        } else {
                                sprintf(concateString, "%s,%s", p[j].hand, p[j].name);
                        }
                        n = sprintf(sendline, "%d %d %d %d %d %d %s\n", PLAYER_INFO, j, p[j].death, p[j].cnt,
                                    p[j].count_down, p[j].isConnected, concateString);
                        if (p[i].isConnected) {
                                sendline[n] = '\0';
                                Writen(sockfd[i], sendline, n);
                        }
                }
        }
}

void sort_hand(char *hand) {
        int n = strlen(hand);
        for (int i = 0; i < n - 1; i++) {
                for (int j = 0; j < n - i - 1; j++) {
                        if (map_card(hand[j]) > map_card(hand[j + 1])) {
                                char temp = hand[j];
                                hand[j] = hand[j + 1];
                                hand[j + 1] = temp;
                        }
                }
        }
}

void distribute_deck(struct player *p, char *deck) {
        for (int i = 0; i < 20; i++) {
                // p[0]: deck 0, 4, 8, 12, 16 -> hand 0, 1, 2, 3, 4
                p[i % 4].hand[i / 4] = deck[i];
        }
        /*
        for(int i = 0;i < 4;++i){
                p[i].hand[5] = '\0';
        }
        */
        for (int i = 0; i < 4; i++) {
                sort_hand(p[i].hand);
        }
}

int isCardsSame(char *cards, char table) {
        for (int i = 0; cards[i] != 0; i++) {
                if (cards[i] != table && cards[i] != 'J') {
                        return 0;
                }
        }
        return 1;
}

void game(struct socketAndfd socket[4]) {
        srand(time(NULL));
        int n;
        int sockfd[4];
        struct player p[4];
        for (int i = 0; i < 4; i++) {

                sockfd[i] = socket[i].fd;
                p[i].death = 0;
                p[i].cnt = 0;
                p[i].count_down = rand() % 6 + 1;
                p[i].isConnected = 1;
                strcpy(p[i].name, socket[i].name);
        }

        char table = card_map[rand() % 3], *prev_cards;
        int table_cnt = 0;
        int turn = 0;
        int prev_turn = 0, alive_player = 4;
        char recvline[MAXLINE], sendline[MAXLINE];
        char temp[MAXLINE];
        for (int i = 0; i < 4; ++i) {
                n = sprintf(temp, "%d\n", GAME);
                temp[n] = '\0';
                Writen(sockfd[i], temp, n);
        }
        initialize(sockfd, p, &table, &table_cnt, &prev_cards);
        fd_set rset;
        int maxfd = -1;

        while (1) {
                if (alive_player == 1) {
                        // This game has a winner
                        int winner = -1;
                        for (int i = 0; i < 4; i++) {
                                if (!p[i].death) {
                                        winner = i;
                                        break;
                                }
                        }
                        for (int i = 0; i < 4; i++) {
                                n = sprintf(sendline, "%d %d\n", WIN, winner);
                                if (p[i].isConnected) {
                                        Writen(sockfd[i], sendline, n);
                                        Close(sockfd[i]);
                                        p[i].isConnected = 0;
                                }
                        }
                        printf("Winner: player #%d\n", winner);
                        return;
                }
                while (p[turn].death || strlen(p[turn].hand) == 0) {
                        // this player died or this player has no card
                        turn = (turn + 1) % 4;
                }
                // tell players who is the next
                for (int j = 0; j < 4; j++) {
                        n = sprintf(sendline, "%d %d\n", TURN, turn);
                        if (p[j].isConnected) {
                                sendline[n] = '\0';
                                Writen(sockfd[j], sendline, n);
                        }
                }
                // If there is only 1 player having card, he can only doubt
                int remain1p_haveCard = 1;
                for (int i = 0; i < 4; i++) {
                        if (strlen(p[i].hand) != 0 && i != turn) {
                                remain1p_haveCard = 0;
                                break;
                        }
                }
                if (remain1p_haveCard) {
                        n = sprintf(sendline, "%d\n", REMAIN);
                        Writen(sockfd[turn], sendline, n);
                }
                maxfd = -1;
                FD_ZERO(&rset);
                for (int i = 0; i < 4; i++) {
                        printf("player #%d connection: %d\n", i, p[i].isConnected);
                        if (p[i].isConnected) {
                                FD_SET(sockfd[i], &rset);
                                maxfd = max(maxfd, sockfd[i]);
                        }
                }
                printf("maxfd: %d\n", maxfd);
                
                printf("current turn: %d\n", turn);
                if (maxfd < 1) {
                        return;
                }
                printf("Do in gane select\n");
                Select(maxfd + 1, &rset, NULL, NULL, NULL);
                
                for (int i = 0; i < 4; i++) {
                        if (FD_ISSET(sockfd[i], &rset)) {
                                if ((n = Read(sockfd[i], recvline, MAXLINE)) <= 0) {
                                        // Player i is quitting
                                        printf("Player %d left.\n",i);
                                        if (n == 0) {
                                                // Ctrl+D
                                                n = sprintf(sendline, "%d %d (Bye!)\n", MSG, -1);
                                                Writen(sockfd[i], sendline, n);
                                        }
                                        for (int j = 0; j < 4; j++) {
                                                if (j != i && p[j].isConnected) {
                                                        n = sprintf(sendline, "%d %d\n", QUIT, i);
                                                        Writen(sockfd[j], sendline, n);
                                                }
                                        }
                                        Close(sockfd[i]);
                                        p[i].death = 1;
                                        p[i].isConnected = 0;
                                        alive_player--;
                                        turn = (turn + 1) % 4;
                                        initialize(sockfd, p, &table, &table_cnt, &prev_cards);
                                        continue;
                                } else {
                                        printf("(main): receive: %s\n", recvline);
                                        // Player i has other input
                                        recvline[n] = 0;
                                        int action;
                                        char selected[100];
                                        char remain[MAXLINE / 4];
                                        int received_player = -1;
                                        // format: action | selected cards | remaining cards
                                        sscanf(recvline, "%d %d %s %s", &action, &received_player,selected, remain);
                                        if(received_player != turn){
                                        	continue;
                                        }
                                        if (action == 1) {
                                                // Player has selected cards <= 3 to be thrown
                                                prev_turn = turn;
                                                printf("select: %s\nremain: %s\n", selected, remain);
                                                if (strcmp(remain, "no") == 0) {
                                                        strcpy(p[i].hand, "");
                                                } else {
                                                        strcpy(p[i].hand, remain);
                                                }
                                                strcpy(prev_cards, selected);
                                                table_cnt += strlen(prev_cards);
                                                turn = (turn + 1) % 4;
                                                send_PLAYER_INFO(sockfd, p);
                                                n = sprintf(sendline, "%d %d %s\n", CARD, prev_turn, selected);
                                                for (int j = 0; j < 4; j++) {
                                                        if (p[j].isConnected) {
                                                                Writen(sockfd[j], sendline, n);
                                                        }
                                                }
                                        } else if (action == 2) {
                                                // Player doubt the left player
                                                int isSame = isCardsSame(prev_cards, table);
                                                n = sprintf(sendline, "%d %d %d %d\n", DOUBT, turn, prev_turn, isSame);
                                                for (int j = 0; j < 4; j++) {
                                                        if (p[j].isConnected) {
                                                                Writen(sockfd[j], sendline, n);
                                                        }
                                                }
                                                if (!isSame) {
                                                        // if isSame, player raising the doubt will get bang
                                                        // otherwise the doubted player get bang
                                                        turn = prev_turn;
                                                }
                                                p[turn].count_down--;
                                                p[turn].cnt++;
                                                if (p[turn].count_down == 0) {
                                                        alive_player--;
                                                        p[turn].death = 1;
                                                        for (int j = 0; j < 4; j++) {
                                                                n = sprintf(sendline, "%d %d\n", DEATH, turn);
                                                                if (p[j].isConnected) {
                                                                        Writen(sockfd[j], sendline, n);
                                                                }
                                                        }
                                                        turn = (turn + 1) % 4;
                                                }
                                                initialize(sockfd, p, &table, &table_cnt, &prev_cards);
                                        } else if (action == 7) { // special kill
                                                p[turn].count_down--;
                                                p[turn].cnt++;
                                                if (p[turn].count_down == 0) {
                                                        alive_player--;
                                                        p[turn].death = 1;
                                                        for (int j = 0; j < 4; j++) {
                                                                n = sprintf(sendline, "%d %d\n", DEATH, turn);
                                                                if (p[j].isConnected) {
                                                                        Writen(sockfd[j], sendline, n);
                                                                }
                                                        }
                                                        turn = (turn + 1) % 4;
                                                }
                                                initialize(sockfd, p, &table, &table_cnt, &prev_cards);

                                        } else if (action == 10) {
                                                for (int j = 0; j < 4; j++) {
                                                        n = sprintf(sendline, "%d %d\n", MSG, turn);
                                                        if (p[j].isConnected) {
                                                                Writen(sockfd[j], sendline, n);
                                                        }
                                                }
                                        }
                                }
                                memset(recvline, 0, sizeof(recvline));
                        }
                }
        }
        return;
}

void sig_chld(int signo) {
        pid_t pid;
        int stat;
        while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
                printf("child %d killed\n", pid);
        }
        return;
}

int main(int argc, char **argv) {
        int listenfd;
        struct sockaddr_in servaddr;
        int maxfd = -1, player_cnt = 0, user_cnt = 0;
        pid_t pid;
        void sig_chld(int);

        listenfd = Socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(SERV_PORT + 25);

        int one = 1;
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

        Listen(listenfd, LISTENQ);

        Signal(SIGCHLD, sig_chld); /* must call waitpid() */
        for (int i = 0; i < capacity; i++) {
                client[i].idx = i;
                client[i].len = sizeof(client[i].addr);
                client[i].isConnected = 0;
        }
        fd_set rset;
        for (;;) {
                maxfd = -1;
                FD_ZERO(&rset);
                if (user_cnt < capacity) {
                        FD_SET(listenfd, &rset);
                        maxfd = max(maxfd, listenfd);
                }
                for (int i = 0; i < capacity; i++) {
                        if (client[i].isConnected) {
                                printf("(main) client #%d is connected\n", i);
                                FD_SET(client[i].fd, &rset);
                                maxfd = max(maxfd, client[i].fd);
                        }
                }
                int n;
                char sendline[2 * MAXLINE], recvline[MAXLINE];
                printf("(main) Do in main select\n");
                if (select(maxfd + 1, &rset, NULL, NULL, NULL) < 0) {
                        if (errno == EINTR) {
                                continue;
                        } else {
                                err_sys("(main) In main select errno = %d", errno);
                        }
                }
                if (FD_ISSET(listenfd, &rset)) {
                        int userNum = 0;
                        for (; userNum < capacity; userNum++) {
                                if (!client[userNum].isConnected) {
                                        break;
                                }
                        }
                        if ((client[userNum].fd = accept(listenfd, (SA *)&client[userNum].addr, &client[userNum].len)) <
                            0) {
                                if (errno == EINTR) {
                                        continue;
                                } else {
                                        err_sys("accept error");
                                }
                        }
                        user_cnt++;
                        player_cnt++;
                        client[userNum].isConnected = 1;
                        n = Read(client[userNum].fd, client[userNum].name, MAXLINE);
                        client[userNum].name[n] = 0;
                        n = sprintf(sendline, "You are the #%d user.\n", userNum + 1);
                        Writen(client[userNum].fd, sendline, n);
                        n = sprintf(sendline, "You may now wait for %d more users.\n",
                                    4 - player_cnt);
                        Writen(client[userNum].fd, sendline, n);
                        for (int i = 0; i < capacity; i++) {
                                if (client[i].isConnected && i != userNum) {
                                        n = sprintf(sendline, "%d %d (#%d user %s enters)\n",10, i, userNum + 1,
                                                    client[userNum].name);
                                        sendline[n] = '\0';
                                        Writen(client[i].fd, sendline, n);
                                }
                        }
                        printf("(main) user #%d called %s enters. he is the %dth player\n", userNum + 1,
                               client[userNum].name, player_cnt);
                }
                // Fork
                if (player_cnt == 4) {
                        struct socketAndfd socket[4];
                        int sock_idx = 0;
                        for (int i = 0; i < capacity; i++) {
                                if (client[i].isConnected) {
                                        client[i].isConnected = 0;
                                        socket[sock_idx] = client[i];
                                        sock_idx++;
                                        if (sock_idx == 4) {
                                                break;
                                        }
                                }
                        }
                        if ((pid = Fork()) == 0) {
                                Close(listenfd);
                                game(socket);
                                return 0;
                        }
                        printf("(main) Game pid %d with players [ ", pid);
                        for (int i = 0; i < 4; i++) {
                                printf("%s ", socket[i].name);
                                Close(socket[i].fd);
                                FD_CLR(socket[i].fd, &rset);
                        }
                        printf("] started\n");
                        player_cnt = 0;
                        user_cnt -= 4;
                }
                // message room
                for (int i = 0; i < capacity; i++) {
                        if (FD_ISSET(client[i].fd, &rset)) {
                                if ((n = Read(client[i].fd, recvline, MAXLINE)) <= 0) {
                                        // user quiting
                                        user_cnt--;
                                        player_cnt--;
                                        if (n == 0) {
                                                // Ctrl+D normal quit
                                                n = sprintf(sendline, "Bye!\n");
                                                Writen(client[i].fd, sendline, n);
                                        }
                                        for (int j = 0; j < capacity; j++) {
                                                if (client[j].isConnected && j != i) {
                                                        if (user_cnt > 1) {
                                                                n = sprintf(sendline,
                                                                            "(%s left the game. %d users left.)\n",
                                                                            client[i].name, user_cnt);
                                                                Writen(client[j].fd, sendline, n);
                                                        } else {
                                                                n = sprintf(
                                                                    sendline,
                                                                    "(%s left the game. You are the last one. Press "
                                                                    "Ctrl+D to left or wait for %d more users to start "
                                                                    "a game.)\n",
                                                                    client[i].name, 4 - player_cnt);
                                                                Writen(client[j].fd, sendline, n);
                                                        }
                                                }
                                        }
                                        FD_CLR(client[i].fd, &rset);
                                        client[i].isConnected = 0;
                                        Close(client[i].fd);
                                        printf("(main) user #%d called %s quit\n", i + 1, client[i].name);
                                } else {
                                        // normal talk
                                        recvline[n] = 0;
                                        n = sprintf(sendline, "(%s) %s", client[i].name, recvline);
                                        for (int j = 0; j < capacity; j++) {
                                                if (client[j].isConnected && j != i) {
                                                        Writen(client[j].fd, sendline, n);
                                                }
                                        }
                                        Writen(fileno(stdout), sendline, n);
                                }
                        }
                }
        }
}
