cmake_minimum_required(VERSION 3.7)
project(Semestralna_Praca_POS C)

set(CMAKE_C_STANDARD 99)

add_executable(server server.h server.c)
add_executable(client client.h client.c)