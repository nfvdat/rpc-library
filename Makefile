CXX=g++
CXXFLAGS=-std=c++11 -Wall -O2 -g -pthread

# Update this to generate new executables.
EXE=register_messages_test.exe locate_messages_test.exe \
    execute_messages_test.exe binder.exe server_example.exe \
    signature_test.exe client_example.exe

HDR= arg.h error.h execute_messages.h locate_messages.h message.h \
     protocol_macros.h register_messages.h rpc.h \
     libserver.h signature.h tcp_client.h tcp_server.h \
     tcp_socket.h terminate_messages.h
OBJ=$(HDR:.h=.o)

all: $(EXE) binder

binder: binder.exe
	cp binder.exe binder

librpc.a: $(OBJ)
	ar rs librpc.a $(OBJ)

%.o: %.cc $(HDR)
	$(CXX) $(CXXFLAGS) -c $<

%.exe: %.o librpc.a
	$(CXX) $(CXXFLAGS) -L. -o $@ $< -lrpc

clean:
	rm -f $(OBJ) $(EXE) *.o

.PHONY: all clean
