//
// Created by user on 30. 12. 2021.
//

#ifndef SEMESTRALNA_PRACA_POS_SERVER_H
#define SEMESTRALNA_PRACA_POS_SERVER_H

FILE* otvorSubor(char* nazov);
int skontrolujRegistraciu(int socket);
int skontrolujPrihlasenie(int socket);
int vymazanieUctu(int socket);


#endif //SEMESTRALNA_PRACA_POS_SERVER_H
