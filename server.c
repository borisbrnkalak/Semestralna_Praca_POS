#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "server.h"

#define MAX_POCET_KLIENTOV 5

int main(int argc, char *argv[]) {

    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char buffer[256];

    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket address");
        return 2;
    }

    listen(sockfd, MAX_POCET_KLIENTOV);

    bool skoncil = false;

    //pri viac klientov tieto veci musia byt vnutry cyklu
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);

    if (newsockfd < 0) {
        perror("ERROR on accept");
        return 3;
    }

    int jePrihlaseny = 0;

    while (!skoncil) {

        bzero(buffer, 256);

        if(jePrihlaseny == 0) {
            skontrolujMeno(newsockfd);
            jePrihlaseny++;
        } else {
            n = read(newsockfd, buffer, 255);
            if (n < 0) {
                perror("Error reading from socket");
                return 4;
            }


            printf("Here is the message: %s\n", buffer);
            const char *msg = "I got your message";
            n = write(newsockfd, msg, strlen(msg) + 1);
            if (n < 0) {
                perror("Error writing to socket");
                return 5;
            }

            int i = strncmp("bye", buffer, 3);
            if(i == 0){
                break;
            }
        }
    }
    close(newsockfd);
    close(sockfd);
    return 0;
}

FILE* otvorSubor(char* nazov) {
    FILE* subor;
    subor = fopen(nazov, "a+");

    if(subor == NULL) {
        printf("Nastala chyba pri otvarani suboru\n");
        return NULL;
    }
    return subor;
}

int skontrolujMeno(int socket) {

    FILE* subor = otvorSubor("prihlaseny.txt");

    char buffMeno[128]; // zo suboru
    char buffHeslo[128];
    char buffLogin[128]; // to čo zada pouzivatel

    read(socket, buffLogin, sizeof(buffLogin));
    printf("Precital som uzivatela\n");
    printf(buffLogin);
    while (fscanf(subor, "%s %*s", buffMeno) != EOF) {
        if(strcmp(buffMeno, buffLogin) == 0) {
            printf("Pouzivatel s tymto loginom uz existuje!\n");
            write(socket, "error", sizeof("error"));
            fclose(subor);
            skontrolujMeno(socket);
            return 0;
        }
    }
    //bzero(buffHeslo, 256);

    write(socket, "ok", 2);

    read(socket, buffHeslo, sizeof(buffHeslo));

    if(strncmp(buffHeslo, "error", 5) == 0) {
        printf("Zle heslo\n");
        skontrolujMeno(socket);
        return 0;
    }

    printf(buffHeslo);

    fprintf(subor,buffLogin);
    fprintf(subor," ");
    fprintf(subor,buffHeslo);
    fprintf(subor,"\n");
    fclose(subor);
    return 1;
}


