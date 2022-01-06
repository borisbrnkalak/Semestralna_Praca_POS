#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include <pthread.h>

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048



client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void queue_add(client_t *cl){
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i < MAX_CLIENTS; ++i){
        if(!clients[i]){
            clients[i] = cl;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid){
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i < MAX_CLIENTS; ++i){
        if(clients[i]){
            if(clients[i]->uid == uid){
                clients[i] = NULL;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void send_message(char *s, int uid){
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i]){
            if(clients[i]->uid != uid){
                if(write(clients[i]->sockfd, s, strlen(s)) < 0){
                    perror("ERROR: write to descriptor failed");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

int ukazZiadostOpriatelstvo(int socket, client_t* cli){
    char pouzivatel[128];
    char prihlaseny[128];
    char odpovedKlienta[128];

    FILE* subor = otvorSubor("priatelskaListina.txt");
    //rewind(subor);

    int malDajake = 0;
    while(fscanf(subor, "%s %s", pouzivatel, prihlaseny) != EOF) {

        if(strcmp(cli->aktualnePrihlaseny, prihlaseny) == 0) {
            malDajake = 1;
            write(socket, pouzivatel, sizeof(pouzivatel)); // napis klientovi uzivatela ktory si ho chce pridat

            bzero(odpovedKlienta, sizeof(odpovedKlienta));
            read(socket,odpovedKlienta, sizeof(odpovedKlienta));// odpoved ci klient potvrdil ziadost

            if(strcmp(odpovedKlienta,"a") == 0){
                printf("INFO    %s potvrdil žiadosť od %s\n", prihlaseny, pouzivatel);
                zapisDoFriendList(prihlaseny, pouzivatel);

            } else if (strcmp(odpovedKlienta,"o") == 0){
                printf("INFO    %s nepotvrdil žiadosť od %s\n", prihlaseny, pouzivatel);
                continue;
            }
        }
    }
    if(malDajake == 1){
        premazListinu(cli);
        write(socket,"empty",strlen("empty"));
    } else {
        write(socket,"empty",strlen("empty"));
    }
    fclose(subor);
}

int premazListinu(client_t* cli) {
    FILE *subor = otvorSubor("priatelskaListina.txt");
    FILE *suborTmp = otvorSubor("temp.txt");
    char buffListinaLavaStrana[128];
    char buffListinaPravaStrana[128];

    if (!suborTmp) {
        printf("Error with opening tmp file!\n");
        return 0;
    }

    while (fscanf(subor,"%s %s",buffListinaLavaStrana,buffListinaPravaStrana) == 2) { // prechadza Listinu
        if(strcmp(buffListinaPravaStrana,cli->aktualnePrihlaseny) != 0){ // ak sa nacitany riadok nerovna prihlasenemu
            fprintf(suborTmp, buffListinaLavaStrana);
            fprintf(suborTmp, " ");                             // tak sa to zapise do tmp suboru
            fprintf(suborTmp, buffListinaPravaStrana);
            fprintf(suborTmp, "\n");
        }
    } // end WHILE
    fclose(subor);
    fclose(suborTmp);
    remove("priatelskaListina.txt");
    rename("temp.txt", "priatelskaListina.txt");
    return 1;
}

int pridajNovehoPriatela(int socket, client_t* cli) {

    char pouzivatel[128];
    char buffer[128];

    FILE* subor = otvorSubor("prihlaseny.txt");

    while(fscanf(subor, "%s %*s", buffer) != EOF) { // vypise registrovanych
        if(strcmp(cli->aktualnePrihlaseny, buffer) != 0) {
            write(socket, buffer, sizeof(pouzivatel));
        }
    }

    write(socket, "koniec", strlen("koniec"));

    rewind(subor);
    while (1) {
        int nasielSa = 0;
        bzero(pouzivatel, sizeof(pouzivatel));
        read(socket, pouzivatel, sizeof(pouzivatel));

        if(strcmp(pouzivatel,"bye") == 0){
            break;
        }

        rewind(subor);
        while(fscanf(subor, "%s %*s", buffer) != EOF) {
            if(strcmp(pouzivatel, buffer) == 0 && strcmp(pouzivatel, cli->aktualnePrihlaseny) != 0) {
                nasielSa = 1;
                break;
            }
        }
        if(nasielSa == 1) {
            int mozeZapisat = prejdiFriendList(buffer,cli);
            if(mozeZapisat == 1){
                FILE* subor2 = otvorSubor("priatelskaListina.txt");
                fprintf(subor2, cli->aktualnePrihlaseny); // odosielatel
                fprintf(subor2, " ");
                fprintf(subor2, buffer); // prijmatel
                fprintf(subor2, "\n");
                fclose(subor2);

                printf("Nasiel sa typek\n");
                bzero(buffer, sizeof(buffer));
                write(socket, "ok", 2);
            } else if(mozeZapisat == 0){
                write(socket, "already friend", strlen("already friend"));
            }
            break;
        }
        write(socket, "error", strlen("error"));
    }
    return 8;
}

int prejdiFriendList(char* prijmatel,client_t* cli){
    char* buffMeno1[128];
    char* buffMeno2[128];
    FILE *suborFriendList = otvorSubor("friendList.txt");

    while(fscanf(suborFriendList,"%s %s", buffMeno1,buffMeno2) == 2){
        if(((strcmp(buffMeno1,prijmatel) == 0) || (strcmp(buffMeno1,cli->aktualnePrihlaseny) == 0))
           && ((strcmp(buffMeno2,cli->aktualnePrihlaseny) == 0) || (strcmp(buffMeno2,prijmatel) == 0))) {
            printf("INFO    Uz maju priatelstvo\n");
            close(suborFriendList);
            rewind(suborFriendList);
            return 0;
        }
    }
    close(suborFriendList);
    return 1;
}

void posliZoznamPriatelov(int socket, client_t* cli){
    char buffMeno1[128];
    char buffMeno2[128];

    FILE* subor = otvorSubor("friendList.txt");
    rewind(subor);

    while(fscanf(subor, "%s %s", buffMeno1, buffMeno2) == 2) {
        if((strcmp(cli->aktualnePrihlaseny, buffMeno1) == 0)) {
            write(socket, buffMeno2, sizeof(buffMeno2));
        }
        if((strcmp(cli->aktualnePrihlaseny, buffMeno2) == 0)){
            write(socket, buffMeno1, sizeof(buffMeno1));
        }
    }
    fclose(subor);
    write(socket, "koniec", strlen("koniec"));
}

void *handle_client(void *arg) {
    char buffer[BUFFER_SZ];


    client_t *cli = (client_t *)arg;
    cli_count++;

    //bzero(buffer, BUFFER_SZ);
    char volba[20];
    int n = 0;

    while (1) {

        if(strcmp(volba, "b") != 0 ) {
            if(strcmp(volba, "e") != 0){
                if(strcmp(volba, "f") != 0){
                    if(strcmp(volba, "c") != 0) {
                        bzero(buffer, 256);
                        n = read(cli->sockfd, buffer, sizeof(buffer));
                    }
                }
            }
        }

        if (n < 0) {
            perror("Error reading from socket");
            return 4;
        }

        if(strncmp(buffer,"odhlaseny",9 ) == 0) {
            printf("INFO:   Pouzivatel sa odhlasil. \n");
            memset(cli->aktualnePrihlaseny, 0, sizeof(cli->aktualnePrihlaseny));
            cli->jePrihlaseny = 0;
        }
        if (cli->jePrihlaseny == 0) {

            if (strncmp(buffer, "a", 1) == 0) {
                skontrolujPrihlasenie(cli->sockfd, cli);

                bzero(volba, sizeof(volba));
                read(cli->sockfd, volba, sizeof(volba));

                //printf("Volba bola: %s\n",volba);
            } else if (strncmp(buffer, "b", 1) == 0) {
                skontrolujRegistraciu(cli->sockfd, cli);

                bzero(volba, sizeof(volba));
                read(cli->sockfd, volba, sizeof(volba));

                //printf("Volba bola: %s\n",volba);
            }else if(strncmp(buffer, "c", 1) == 0) {
                vymazanieUctu(cli->sockfd, cli);
            } else if(strncmp(buffer, "x", 1) == 0){
                printf("INFO:   Klient sa odpojil\n");
                close(cli->sockfd);
                queue_remove(cli->uid);
                free(cli);
                cli_count--;
                pthread_detach(pthread_self());
                return 6;
            } else {
                cli->jePrihlaseny = 0;
            }

        } else {
            if(strncmp(volba,"a",1) == 0){

                int i = strncmp("bye\n", buffer, 4);
                if (i == 0) {
                    bzero(volba, sizeof(volba));
                    read(cli->sockfd,volba,sizeof (volba));
                    //printf("Napisal bye v volbe je: %s\n", volba);
                }else {

                    printf("Here is the message: %s\n", buffer);
                    //send_message(buffer, cli->uid);
                    const char *msg = "I got your message";
                    n = write(cli->sockfd, msg, strlen(msg));
                    if (n < 0) {
                        perror("Error writing to socket");
                        return 5;
                    }
                }
            } else if(strncmp(volba, "b", 1) == 0) {

                pridajNovehoPriatela(cli->sockfd, cli);
                bzero(volba, sizeof(volba));
                read(cli->sockfd,volba,sizeof (volba));

            } else if(strncmp(volba, "f", 1) == 0) {
                posliZoznamPriatelov(cli->sockfd, cli);
                bzero(volba, sizeof(volba));
                read(cli->sockfd,volba,sizeof (volba));
            }else if(strncmp(volba, "e", 1) == 0) {

                ukazZiadostOpriatelstvo(cli->sockfd, cli);
                bzero(volba, sizeof(volba));
                read(cli->sockfd,volba,sizeof (volba));
            }
        }
        bzero(buffer, 256);
    }

    close(cli->sockfd);
    queue_remove(cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char *argv[]) {

    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t vlakno;


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

    listen(sockfd, 10);

    while(1) {
        cli_len = sizeof(cli_addr);

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);

        if(newsockfd < 0) {
            perror("ERROR\n");
            return 3;
        }

        /* Client settings */
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = newsockfd;
        cli->uid = uid++;
        cli->jePrihlaseny = 0;

        queue_add(cli);

        pthread_create(&vlakno, NULL, &handle_client, (void*)cli);

        /* Reduce CPU usage */
        sleep(1);
    }


    close(newsockfd);
    close(sockfd);
    return 0;
}

FILE *otvorSubor(char *nazov) {
    FILE *subor;
    subor = fopen(nazov, "a+");

    if (subor == NULL) {
        printf("INFO:   Nastala chyba pri otvarani suboru\n");
        return NULL;
    }
    return subor;
}

int skontrolujRegistraciu(int socket, client_t* cli) {

    FILE *subor = otvorSubor("prihlaseny.txt");

    char buffMeno[128]; // zo suboru
    char buffHeslo[128];
    char buffLogin[128]; // to čo zada pouzivatel

    bzero(buffLogin, sizeof(buffLogin));
    read(socket, buffLogin, sizeof(buffLogin));

    while (fscanf(subor, "%s %*s", buffMeno) != EOF) {
        if (strcmp(buffMeno, buffLogin) == 0) {
            printf("INFO:   Pouzivatel s tymto loginom: %s uz existuje!\n", buffLogin);
            write(socket, "error", sizeof("error"));
            fclose(subor);
            skontrolujRegistraciu(socket, cli);
            return 0;
        }
    }

    write(socket, "ok", 2);

    bzero(buffHeslo, sizeof(buffHeslo));
    read(socket, buffHeslo, sizeof(buffHeslo));

    if (strncmp(buffHeslo, "error", 5) == 0) {
        printf("INFO:   Zle zadane heslo\n");
        skontrolujRegistraciu(socket, cli);
        return 0;
    }

    fprintf(subor, buffLogin);
    fprintf(subor, " ");
    fprintf(subor, buffHeslo);
    fprintf(subor, "\n");
    fclose(subor);

    strcpy(cli->aktualnePrihlaseny, buffLogin);
    printf("INFO:   Pouzivatel %s uspesne registrovany!\n", cli->aktualnePrihlaseny);
    cli->jePrihlaseny = 1;
    return 1;
}

int skontrolujPrihlasenie(int socket, client_t* cli) {
    FILE *subor = otvorSubor("prihlaseny.txt");

    char buffZadaneMeno[128];
    char buffZadaneHeslo[128];
    char buffSuborHeslo[128];
    char buffSuborMeno[128];

    bzero(buffZadaneMeno, sizeof(buffZadaneMeno));
    read(socket, buffZadaneMeno, sizeof(buffZadaneMeno));

    int nachadzaSaUzivatel = 0;
    int index = 0;
    while(index < cli_count) {
        if(strcmp(clients[index]->aktualnePrihlaseny, buffZadaneMeno) == 0) {
            nachadzaSaUzivatel = 1;
            break;
        } else {
            index++;
        }
    }

    int pocitadlo = 0;
    while (fscanf(subor, "%s %s", buffSuborMeno, buffSuborHeslo) != EOF && nachadzaSaUzivatel == 0) {
        if (strcmp(buffSuborMeno, buffZadaneMeno) == 0) {
            write(socket, "ok", 2);
            while (pocitadlo < 3) {
                bzero(buffZadaneHeslo, sizeof(buffZadaneHeslo));
                read(socket, buffZadaneHeslo, sizeof(buffZadaneHeslo));

                if (strcmp(buffZadaneHeslo, buffSuborHeslo) == 0) {
                    printf("INFO:   Prihlasenie uzivatela %s uspesne!\n",buffZadaneMeno);
                    write(socket, "ok", 2);
                    fclose(subor);
                    strcpy(cli->aktualnePrihlaseny, buffZadaneMeno);
                    cli->jePrihlaseny = 1;
                    return 1;
                } else {
                    pocitadlo++;
                    write(socket, "error", sizeof("error"));
                }
            }
        }
    }
    printf("INFO:   Pouzivatelovi: %s sa nepodarilo prihlasit!\n", buffZadaneMeno);
    write(socket, "error", sizeof("error"));
    fclose(subor);
    skontrolujPrihlasenie(socket, cli);
    return 0;
}

int zapisDoFriendList(char* pouzivatel, char* priatel){
    char* buffPouzivatel[128];
    char* buffPriatel[128];
    FILE *suborFriendList = otvorSubor("friendList.txt");

    while(fscanf(suborFriendList,"%s %s",buffPouzivatel,buffPriatel) == 2){
        if(((strcmp(buffPouzivatel,pouzivatel) == 0) || (strcmp(buffPouzivatel,priatel) == 0))
           && ((strcmp(buffPriatel,pouzivatel) == 0) || (strcmp(buffPriatel,priatel) == 0))) {
            printf("INFO    Uz maju priatelstvo\n");
            return 0;
        }
    }
    fprintf(suborFriendList, priatel);
    fprintf(suborFriendList, " ");
    fprintf(suborFriendList, pouzivatel);
    fprintf(suborFriendList, "\n");

    fclose(suborFriendList);

    return 1;
}

int vymazanieUctu(int socket, client_t* cli) {
    char menoSuboru[128] = "prihlaseny.txt";
    char menoSuboru2[128] = "temp.txt";
    FILE *subor = fopen(menoSuboru, "a+");
    FILE *suborTmp = fopen(menoSuboru2, "a+");

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
        return 0;
    }

    bzero(buffZadaneMeno, sizeof(buffZadaneMeno));
    read(socket, buffZadaneMeno, sizeof(buffZadaneMeno));

    int nasielSa = 0;

    while (fscanf(subor, " %s %s", buffSuborMeno, buffSuborHeslo) == 2) {

        if (strcmp(buffZadaneMeno, buffSuborMeno) == 0) {
            write(socket, "ok", 2);

            bzero(buffZadaneHeslo, sizeof(buffZadaneHeslo));
            read(socket, buffZadaneHeslo, sizeof(buffZadaneHeslo));

            if (strcmp(buffZadaneHeslo, buffSuborHeslo) == 0) {
                write(socket, "ok", 2);

                nasielSa = 1;

                break;
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

        fclose(subor);
        fclose(suborTmp);
        remove(menoSuboru);
        rename(menoSuboru2, menoSuboru);
        return 1;
    } else if(nasielSa == 0) {
        printf("INFO:   Meno: %s neexistuje v subore.\n",buffZadaneMeno);
        write(socket,"error",5);
        vymazanieUctu(socket, cli);
        return 0;
    }

}


//-------------------------------------------------------------//



