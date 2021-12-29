cmake_minimum_required(VERSION 3.7)
project(Semestralna_Praca_POS C)

set(CMAKE_C_STANDARD 99)

add_executable(Semestralna_Praca_POS main.c registracia.h registracia.c vstup.h vstup.c)

add_executable(server server.c registracia.h registracia.c)
add_executable(client client.c registracia.h registracia.c)