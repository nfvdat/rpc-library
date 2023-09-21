### Simple RPC library in C++

Features:
- dynamic binding with multiple clients and servers
- full crash tolerance
- server side round robin load balancing
- client side location caching


#### Compiling library

```
make
```

#### Compiling client and server

```
g++ -L. client.o -lrpc -o client -pthread
g++ -L. server_functions.o server_function_skels.o server.o -lrpc -o server -pthread
```

#### Running

```
./binder
```

Followed by setting the appropriate environment variables BINDER_ADDRESS
and BINDER_PORT, and running `./server` and `./client`.
