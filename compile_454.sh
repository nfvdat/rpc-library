#!/bin/bash -x

g++ -o client.o client1.c -c
g++ server_functions.c -c
g++ server.c -c
g++ server_function_skels.c -c
g++ -L. client.o -lrpc -o client -pthread
g++ -L. server_functions.o server_function_skels.o server.o -lrpc -o server -pthread
