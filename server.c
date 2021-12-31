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


        if (jePrihlaseny == 0) {

            char volba[20];
            char odpoved[20];

            read(newsockfd, volba, sizeof(volba));
            if (strncmp(volba, "a", 1) == 0) {
                skontrolujPrihlasenie(newsockfd);
            } else if (strncmp(volba, "b", 1) == 0) {
                skontrolujRegistraciu(newsockfd);
            } else if(strncmp(volba, "c", 1) == 0) {
                vymazanieUctu(newsockfd);
            }
            jePrihlaseny++;
        } else {
            bzero(buffer, 256);
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
            if (i == 0) {
                break;
            }
        }
    }
    close(newsockfd);
    close(sockfd);
    return 0;
}

FILE *otvorSubor(char *nazov) {
    FILE *subor;
    subor = fopen(nazov, "a+");

    if (subor == NULL) {
        printf("Nastala chyba pri otvarani suboru\n");
        return NULL;
    }
    return subor;
}

int skontrolujRegistraciu(int socket) {

    FILE *subor = otvorSubor("prihlaseny.txt");

    char buffMeno[128]; // zo suboru
    char buffHeslo[128];
    char buffLogin[128]; // to čo zada pouzivatel

    read(socket, buffLogin, sizeof(buffLogin));
    //printf(buffLogin);
    while (fscanf(subor, "%s %*s", buffMeno) != EOF) {
        if (strcmp(buffMeno, buffLogin) == 0) {
            printf("Pouzivatel s tymto loginom uz existuje!\n");
            write(socket, "error", sizeof("error"));
            fclose(subor);
            skontrolujRegistraciu(socket);
            return 0;
        }
    }
    //bzero(buffHeslo, 256);

    write(socket, "ok", 2);

    read(socket, buffHeslo, sizeof(buffHeslo));

    if (strncmp(buffHeslo, "error", 5) == 0) {
        printf("Zle heslo\n");
        skontrolujRegistraciu(socket);
        return 0;
    }

    printf(buffHeslo);

    fprintf(subor, buffLogin);
    fprintf(subor, " ");
    fprintf(subor, buffHeslo);
    fprintf(subor, "\n");
    fclose(subor);
    return 1;
}

int skontrolujPrihlasenie(int socket) {
    FILE *subor = otvorSubor("prihlaseny.txt");

    char buffZadaneMeno[128];
    char buffZadaneHeslo[128];
    char buffSuborHeslo[128];
    char buffSuborMeno[128];

    read(socket, buffZadaneMeno, sizeof(buffZadaneMeno));

    int pocitadlo = 0;
    while (fscanf(subor, "%s %s", buffSuborMeno, buffSuborHeslo) != EOF) {
        if (strcmp(buffSuborMeno, buffZadaneMeno) == 0) {

            write(socket, "ok", 2);
            while (pocitadlo < 3) {
                read(socket, buffZadaneHeslo, sizeof(buffZadaneHeslo));

                if (strcmp(buffZadaneHeslo, buffSuborHeslo) == 0) {
                    printf("Prihlasenie uspesne!\n");
                    write(socket, "ok", 2);
                    fclose(subor);
                    return 1;
                } else {
                    pocitadlo++;
                    write(socket, "error", sizeof("error"));
                }
            }
        }
    }
    printf("Pouzivatelovi sa nepodarilo prihlasit!\n");
    write(socket, "error", sizeof("error"));
    fclose(subor);
    skontrolujPrihlasenie(socket);
    return 0;
}

int vymazanieUctu(int socket) {
    char menoSuboru[128] = "prihlaseny.txt";
    char menoSuboru2[128] = "temp.txt";
    FILE *subor = fopen(menoSuboru, "a+");
    FILE *suborTmp = fopen(menoSuboru2, "a+");

    char string[256];
    char buffSuborMeno[128];
    char buffSuborHeslo[128];
    char buffZadaneMeno[128];
    char buffZadaneHeslo[128];

    if (!subor) {
        printf("Error with opening file!\n");
        return 0;
    }

    if (!suborTmp) {
        printf("Error with opening tmp file!\n");
    }

    read(socket, buffZadaneMeno, sizeof(buffZadaneMeno));

    int nasielSa = 0;

    while (fscanf(subor, " %s %s", buffSuborMeno, buffSuborHeslo) == 2) {

        if (strcmp(buffZadaneMeno, buffSuborMeno) == 0) {
            write(socket, "ok", 2);

            read(socket, buffZadaneHeslo, sizeof(buffZadaneHeslo));

            if (strcmp(buffZadaneHeslo, buffSuborHeslo) == 0) {
                write(socket, "ok", 2);

                nasielSa = 1;

                break;

                /*while (!feof(subor)) {
                    strcpy(string, "\0"); //vynuluje kopirovany riadok
                    fgets(string, 256, subor); // vlozi do vynulovaneho meno a heslo
                    if (!feof(subor)) {
                        if (strcmp(buffZadaneMeno, buffSuborMeno) != 0 && strcmp(buffZadaneHeslo, buffSuborHeslo) !=
                                                                          0) { // ak sa meno heslo nerovna zadanemu tak ten riadok skopiruje do durheho suboru
                            fprintf(suborTmp, "%s", string); // do temporary suboru zapise string
                        }
                    }
                }
                rename(menoSuboru2, menoSuboru);
                fclose(subor);
                fclose(suborTmp);
                remove(menoSuboru); //vymaze povodny
                rename(menoSuboru2, menoSuboru); // premenuje docasny na povodny
                return 1;*/
            }

        }

    }

    fclose(subor);

    subor = fopen(menoSuboru, "a+");

    if(nasielSa == 1) {
        while (fscanf(subor, " %s %s", buffSuborMeno, buffSuborHeslo) == 2) {
            if(strcmp(buffZadaneMeno, buffSuborMeno) != 0) {
                fprintf(suborTmp, buffSuborMeno);
                fprintf(suborTmp, " ");
                fprintf(suborTmp, buffSuborHeslo);
                fprintf(suborTmp, "\n");
            }
        }
        remove(menoSuboru);
        rename(menoSuboru2, menoSuboru);
        fclose(subor);
        fclose(suborTmp);
        return 1;
    }

    /*write(socket, "error", 5);
    fclose(subor);
    fclose(suborTmp);
    vymazanieUctu(socket);
    return 0;*/
}


