#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "client.h"

int vypnutySocket = 0;

char prihlaseny[128];
int jePrihlaseny = 0;

int vysledok = 0;

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


    while (!konci) {

        if (jePrihlaseny == 0) {
            autentifikacia(sockfd);
            if(vypnutySocket == 1){
                printf("INFO:   Program sa vypol\n");
                return 0;
            }
            //fgets(buffer, 255, stdin);

            //printf(buffer);
        } else {

            if(vysledok =! 1) {
                continue;
            }

            printf("Please enter a message: ");
            bzero(buffer, 256);
            fgets(buffer, 255, stdin);


            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0) {
                perror("Error writing to socket");

                return 5;
            }

            int i = strncmp("bye\n", buffer, 4);
            if (i == 0){
                //printf("napisal bye. \n");

                menuPouzivatela(sockfd);
                continue;
            }

            bzero(buffer, 256);
            n = read(sockfd, buffer, 255);
            if (n < 0) {
                perror("Error reading from socket");
                return 6;
            }
            printf("%s\n", buffer);;
        }
    }
    close(sockfd);

    return 0;
}

int prihlas(int socket) {
    printf("===================\n");
    printf("PRIHLASENIE\n");
    printf("===================\n");
    char meno[128];
    char heslo[128];
    char odpoved[128];

    printf("\n");
    printf("Zadajte meno: \n");
    scanf("%s", &meno);

    write(socket, meno, sizeof(meno));

    bzero(odpoved, sizeof(odpoved));
    read(socket, odpoved, sizeof(odpoved));

    if (strncmp(odpoved, "error", 5) == 0 || strncmp(odpoved, "ok", 2) != 0) {
        printf("INFO:   Tento pouzivatel neexistuje!\n");
        prihlas(socket);
        return 0;
    }

    printf("%s\n", odpoved);

    int pocitadlo = 0;

    while (pocitadlo < 3) {
        printf("Zadajte svoje heslo: \n");
        scanf("%s", &heslo);

        write(socket, heslo, sizeof(heslo));

        bzero(odpoved, sizeof(odpoved));
        read(socket, odpoved, sizeof(odpoved));

        if(strncmp(odpoved, "ok", 2) == 0) {
            printf("%s\n", odpoved);
            strcpy(prihlaseny, meno);
            jePrihlaseny = 1;
            menuPouzivatela(socket);
            return 1;
        }

        pocitadlo++;
    }
    if(strcmp(odpoved, "error") == 0) {
        printf("%s\n", odpoved);
        prihlas(socket);
        return 0;
    }
    return 0;
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

    bzero(odpoved, sizeof(odpoved));
    read(socket, odpoved, sizeof(odpoved));

    if (strcmp(odpoved, "error") == 0) {
        printf("%s", odpoved);
        registruj(socket);
        return 0;
    }
    printf("Zadajte svoje heslo\n");
    scanf("%s", &heslo);

    printf("*******Potvrdte svoje heslo*******\n");
    scanf("%s", &potvrdenie);


    if (strcmp(heslo, potvrdenie) == 0) {
        printf("INFO:   Dobre heslo\n");
        write(socket, heslo, sizeof(heslo));
    } else {
        write(socket, "error", strlen("error"));
        registruj(socket);
        return 0;
    }
    jePrihlaseny = 1;
    menuPouzivatela(socket);
    return 1;
}

int chatovanie(int socket){

}

int ziadostiPriatelov(int socket) {
    char potvrdenie[128];
    char menoUzivatela[128];

    while(1){
        bzero(menoUzivatela, sizeof(menoUzivatela));
        read(socket,menoUzivatela, sizeof(menoUzivatela));
        if(strcmp(menoUzivatela,"empty") != 0){
            printf("Potvrdit ziadost od použivatela %s \n",menoUzivatela);
            printf("[a] --> ANO\n[o] --> ODMIETNUT\n\n");

            scanf("%s",potvrdenie);

            write(socket,potvrdenie, sizeof(potvrdenie));
        }else {
            printf("INFO    Nema ziadne ziadosti\n");
            menuPouzivatela(socket);
            break;
        }
    }
    return 0;
}

int zoznamPriatelov(int socket){
    char priatelia[128];

    printf("ZOZNAM PRIATELOV\n");
    printf("------------------------------------------------\n");

    while(1) {
        bzero(priatelia, sizeof(priatelia));
        read(socket, priatelia, sizeof(priatelia));

        if(strcmp("koniec", priatelia) == 0) {
            vysledok = 2;
            printf("------------------------------------------------\n");
            printf("\n");
            menuPouzivatela(socket);
            break;
        }
        printf("%s\n", priatelia);

    }
}

int pridajPriatela(int socket) {

    char pouzivatelia[128];
    char menoUsera[128];
    char odpoved[128];

    printf("AKTUALNE REGISTROVANY POUZIVATELIA:\n");
    printf("------------------------------------------------\n");

    while(1) {
        bzero(pouzivatelia, sizeof(pouzivatelia));
        read(socket, pouzivatelia, sizeof(pouzivatelia));

        if(strcmp("koniec", pouzivatelia) == 0) {
            break;
        }
        printf("%s\n", pouzivatelia);
    }

    while (1) {
        printf("Zadajte meno, koho chcete pridat\n");
        scanf("%s", &menoUsera);
        write(socket, menoUsera, sizeof(menoUsera));

        if(strcmp("bye", menoUsera) == 0) {
            vysledok = 2;
            menuPouzivatela(socket);
            break;
        }

        bzero(odpoved, sizeof(odpoved));
        read(socket, odpoved,sizeof(odpoved));

        if(strncmp(odpoved, "ok", 2) == 0) {
            vysledok = 2;
            menuPouzivatela(socket);
            break;
        } else if(strncmp(odpoved,"already friend",14) == 0){
            printf("INFO    Uz ste priatelia!!\n");
            vysledok = 2;
            menuPouzivatela(socket);
            break;
        } else {
            continue;
        }
    }
}

int odhlasitSa(int socket){
    memset(prihlaseny, 0, sizeof(prihlaseny));
    jePrihlaseny = 0;
    write(socket, "odhlaseny", strlen("odhlaseny"));
    return 1;
}

int menuPouzivatela(int socket) {
    printf("Zadajte volbu, co chcete aby sa stalo:\n");
    printf("[a] -- chatovat\n");
    printf("[b] -- pridat priatelov\n");
    printf("[c] -- odstranit priatelov\n");
    printf("[f] -- zoznam priatelov\n");
    printf("[e] -- ziadosti o priatelstvo\n");
    printf("[d] -- odhlasit sa\n");
    printf("---------------------------------------------\n");
    char volba[20];
    bzero(volba, 20);
    scanf("%s", &volba);

    while((getchar()) != '\n');

    write(socket, volba, sizeof(volba));

    if (strncmp(volba, "a", 1) == 0) {
        vysledok = 1;
        return 1;
    } else if(strncmp(volba, "b", 1) == 0) {
        pridajPriatela(socket);

    }else if(strncmp(volba, "f", 1) == 0) {
        zoznamPriatelov(socket);

    }else if(strncmp(volba, "e", 1) == 0) {
        ziadostiPriatelov(socket);

    }else if(strncmp(volba, "d", 1) == 0){
        odhlasitSa(socket);
        return 3;
    }
    else {
        menuPouzivatela(socket);
        return 0;
    }
}

void autentifikacia(int socket) {
    printf("Zadajte volbu, co chcete aby sa stalo:\n");
    printf("[a] -- prihlasenie\n");
    printf("[b] -- registracia\n");
    printf("[c] -- zmazanie uctu\n");
    printf("[x] -- vypnuť program\n");
    printf("---------------------------------------------\n");
    char volba[20];

    scanf("%s", &volba);

    while((getchar()) != '\n');

    write(socket, volba, sizeof(volba));

    if (strncmp(volba, "a", 1) == 0) {
        prihlas(socket);
    } else if (strncmp(volba, "b", 1) == 0) {
        registruj(socket);
    } else if (strncmp(volba, "c", 1) == 0) {
        zmazanieUctu(socket);
    } else if (strncmp(volba, "x", 1) == 0) {
        vypnut(socket);
    } else {
        autentifikacia(socket);
    }
}
int vypnut(int socket) {
    close(socket);
    return vypnutySocket = 1;
}
int zmazanieUctu(int socket) {
    char meno[128];
    char heslo[128];
    char odpoved[10];

    printf("Zadajte meno na zrusenie uctu\n");
    scanf("%s", &meno);

    write(socket, meno, sizeof(meno));

    bzero(odpoved, sizeof(odpoved));
    read(socket, odpoved, sizeof(odpoved));

    if(strcmp(odpoved, "error") == 0) {
        printf("%s\n", odpoved);
        zmazanieUctu(socket);
        return 0;
    }

    printf("Zadajte svoje heslo\n");
    scanf("%s", &heslo);

    write(socket, heslo, sizeof(heslo));

    bzero(odpoved, sizeof(odpoved));
    read(socket, odpoved, sizeof(odpoved));

    if(strcmp(odpoved, "error") == 0) {
        printf("%s\n", odpoved);
        zmazanieUctu(socket);
        return 0;
    }

    if(strcmp(odpoved,"ok") == 0){
        printf("%s\n", odpoved);
        printf("INFO:   Uspesne ste zmazali ucet pouzivatela %s: \n", meno);
    }

    autentifikacia(socket);
    return 1;
}
