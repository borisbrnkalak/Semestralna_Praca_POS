//
// Created by user on 28. 12. 2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void prihlas() {
    printf("===================\n");
    printf("PRIHLASENIE\n");
    printf("===================\n");
    char* meno;
    char* heslo;
    printf("\n");
    printf("Zadajte meno: \n");
    scanf("%s", &meno);
    printf("Zadajte heslo: \n");
    scanf("%s", &heslo);

    printf("Prihlasil sa pouzivatel: %s, s heslom %s\n", &meno, &heslo);

}

void registruj() {
    printf("===================\n");
    printf("REGISTRACIA\n");
    printf("===================\n");

    char* meno;
    char* heslo;
    char* potvrdenie;

    printf("\n");
    printf("Zadajte nove meno: \n");
    scanf("%s", &meno);
    printf("Zadajte nove heslo: \n");
    scanf("%s", &heslo);
    printf("Overte heslo: \n");
    scanf("%s", &potvrdenie);

    if(strcmp(&heslo, &potvrdenie) == 0) {
        printf("Registracia uspesna!!\n");
    } else {
        printf("Hesla nie su rovnake!!\n");
    }
}

void autentifikacia() {
    printf("Zadajte volbu, co chcete aby sa stalo:\n");
    printf("1 -- prihlasenie\n");
    printf("2 -- registracia\n");
    printf("---------------------------------------------\n");
    int volba;

    scanf("%d", &volba);


    if(volba == 1) {
        prihlas();
    } else if(volba == 2) {
        registruj();
    } else {
        printf("Zle si napisal, ty gej\n");
    }
}



