//
// Created by user on 28. 12. 2021.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


void prihlas() {
    printf("===================\n");
    printf("PRIHLASENIE\n");
    printf("===================\n");
    char* meno;
    char* heslo;
    char buffMeno[128];
    char buffHeslo[128];
    bool prihlasilSa = false;


    printf("\n");
    printf("Zadajte meno: \n");
    scanf("%s", &meno);
    printf("Zadajte heslo: \n");
    scanf("%s", &heslo);

    if(prihlasilSa) {
        printf("Prihlasil sa pouzivatel: %s, s heslom %s\n", &meno, &heslo);
    } else {
        printf("Tento uzivatel neexistuje, skuste to znova.\n");
        prihlas(socket);
    }

}

void registruj() {
    printf("===================\n");
    printf("REGISTRACIA\n");
    printf("===================\n");

    char* meno;
    char* heslo;
    char* potvrdenie;
    char buffPrihlaseny[128];
    bool jePrihlaseny = false;

    printf("\n");
    printf("Zadajte nove meno: \n");
    scanf("%s", &meno);
    printf("Zadajte nove heslo: \n");
    scanf("%s", &heslo);
    printf("Overte heslo: \n");
    scanf("%s", &potvrdenie);

    if(strcmp(&heslo, &potvrdenie) == 0) {
        FILE* subor;

        subor = fopen("prihlaseny.txt", "a+");

        while (fscanf(subor, "%s %*s", buffPrihlaseny) != EOF) {
            if(strcmp(buffPrihlaseny, &meno) == 0) {
                printf("Pouzivatel s tymto loginom uz existuje!\n");
                jePrihlaseny = true;
                break;
            } else {
                break;
            }
        }

        if(jePrihlaseny) {
            registruj(socket);
        } else {

            fprintf(subor, "%s", &meno);
            fprintf(subor, " ");
            fprintf(subor, "%s", &heslo);
            fprintf(subor, "\n");
            printf("Registracia uspesna!!\n");
        }

        fclose(subor);

    } else {
        printf("Hesla nie su rovnake!!\n");
        registruj();
    }
}

void autentifikacia(int socket) {
    printf("Zadajte volbu, co chcete aby sa stalo:\n");
    printf("1 -- prihlasenie\n");
    printf("2 -- registracia\n");
    printf("---------------------------------------------\n");
    int volba;

    scanf("%d", &volba);

    write(socket, "test", strlen("test"));
    if(volba == 1) {
        prihlas();
    } else if(volba == 2) {
        registruj(socket);
    } else if(volba == 3) {
        exit(5);
    }else {
            autentifikacia(socket);
        }
    }