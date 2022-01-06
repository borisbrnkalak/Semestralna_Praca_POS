//
// Created by user on 30. 12. 2021.
//

#ifndef SEMESTRALNA_PRACA_POS_SERVER_H
#define SEMESTRALNA_PRACA_POS_SERVER_H

static _Atomic unsigned int cli_count = 0;
static int uid = 10;


typedef struct{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[32];
    int jePrihlaseny;
    char aktualnePrihlaseny[128];
} client_t;

FILE* otvorSubor(char* nazov);
int skontrolujRegistraciu(int socket, client_t* cli);
int skontrolujPrihlasenie(int socket, client_t* cli);
int ukazZiadostOpriatelstvo(int socket, client_t* cli);
int vymazanieUctu(int socket, client_t* cli);
int pridajNovehoPriatela(int socket, client_t* cli);
int zapisDoFriendList(char* pouzivatel, char* priatel);
int premazListinu(client_t* cli);
int prejdiFriendList(char* prijmatel,client_t* cli);
void posliZoznamPriatelov(int socket, client_t* cli);
int odstranPriatela(int socket, client_t* cli);

#endif //SEMESTRALNA_PRACA_POS_SERVER_H
