## Overview

A simple RPC library built in C++, with:
- dynamic binding with multiple clients and servers
- full crash tolerance
- server side round robin load balancing
- client side location caching


## Compiling 

To compile the library, just run

```
make
```

To compile the client and server, since we also depend on `pthreads`:

```
g++ -L. client.o -lrpc -o client -pthread
g++ -L. server_functions.o server_function_skels.o server.o -lrpc -o server -pthread
```

## Running

```
./binder
```

Then set the environment variables `BINDER_ADDRESS` and `BINDER_PORT` as appropriate, and run:
```
./server
./client
```
