//
// Created by user on 30. 12. 2021.
//

#ifndef SEMESTRALNA_PRACA_POS_CLIENT_H
#define SEMESTRALNA_PRACA_POS_CLIENT_H

int prihlas(int socket);
int registruj(int socket);
int chatovanie(int socket);
int zmazanieUctu(int socket);
int ziadostiPriatelov(int socket);
void autentifikacia(int socket);
int menuPouzivatela(int socket);
int vypnut(int socket);
int pridajPriatela(int socket);
int zoznamPriatelov(int socket);
int odstranPriatela(int socket);

#endif //SEMESTRALNA_PRACA_POS_CLIENT_H
