#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "server.h"
#include "client.h"


int main(int argc, char *argv[]) {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char *) server->h_addr,
            (char *) &serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(atoi(argv[2]));

    bool konci = false;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 3;
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error connecting to socket");
        return 4;
    }

    int jePrihlaseny = 0;

    while (!konci) {

        if (jePrihlaseny == 0) {
            autentifikacia(sockfd);
            jePrihlaseny++;
        }

        printf("Please enter a message: ");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);


        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            perror("Error writing to socket");
            return 5;
        }

        int i = strncmp("bye", buffer, 3);
        if (i == 0)
            break;

        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            return 6;
        }
        printf("%s\n", buffer);

    }
    close(sockfd);

    return 0;
}

void prihlas(int socket) {
    printf("===================\n");
    printf("PRIHLASENIE\n");
    printf("===================\n");
    char *meno;
    char *heslo;
    char buffMeno[128];
    char buffHeslo[128];
    bool prihlasilSa = false;


    printf("\n");
    printf("Zadajte meno: \n");
    scanf("%s", &meno);

    FILE *subor;
    subor = fopen("prihlaseny.txt", "a+");

    while (fscanf(subor, "%s %s", buffMeno, buffHeslo) != EOF) {
        if (strcmp(buffMeno, &meno) == 0) {
            printf("Prihlasuje sa použivateľ s menom : %s\n", buffMeno);

            int pocitadlo = 0;

            while (pocitadlo < 3) {
                printf("Zadajte heslo: \n");
                scanf("%s", &heslo);
                if (strcmp(buffHeslo, &heslo) == 0) {
                    printf("Prihlasenie USPESNE!!\n");
                    prihlasilSa = true;
                    break;
                } else {
                    pocitadlo++;
                    printf("Zle zadane heslo: %d. krat.\n", pocitadlo);
                    char *odpoved;
                    printf("Chcete dalej pokracovat? [A/N]\n");
                    scanf("%s", &odpoved);
                    if (strcmp(&odpoved, "N") == 0) {
                        prihlas(socket);
                    }
                }
            }
        }
    }

    if (prihlasilSa) {
        printf("Prihlasil sa pouzivatel: %s, s heslom %s\n", &meno, &heslo);
    } else {
        printf("Tento uzivatel neexistuje, skuste to znova.\n");
        prihlas(socket);
    }

}

int registruj(int socket) {
    printf("===================\n");
    printf("REGISTRACIA\n");
    printf("===================\n");

    char meno[128];
    char heslo[128];
    char potvrdenie[128];
    char odpoved[128];

    printf("\n");
    printf("Zadajte nove meno: \n");
    scanf("%s", &meno);
    write(socket, meno, sizeof(meno));

    read(socket, odpoved, sizeof(odpoved));

    if (strcmp(odpoved, "error") == 0) {
        printf("%s", odpoved);
        registruj(socket);
        return 0;
    }
    printf("Zadajtes svoje heslo\n");
    scanf("%s", &heslo);

    printf("Potvrdte svoje heslo\n");
    scanf("%s", &potvrdenie);


    if (strcmp(heslo, potvrdenie) == 0) {
        printf("Dobre heslo\n");
        write(socket, heslo, sizeof(heslo));
    } else {
        write(socket, "error", strlen("error"));
        registruj(socket);
        return 0;
    }

    return 1;
}

void autentifikacia(int socket) {
    printf("Zadajte volbu, co chcete aby sa stalo:\n");
    printf("1 -- prihlasenie\n");
    printf("2 -- registracia\n");
    printf("---------------------------------------------\n");
    int volba;

    scanf("%d", &volba);

    if (volba == 1) {
        prihlas(socket);
    } else if (volba == 2) {
        registruj(socket);
    } else if (volba == 3) {
        exit(5);
    } else {
        autentifikacia(socket);
    }
}

