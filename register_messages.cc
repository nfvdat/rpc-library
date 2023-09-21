#include "register_messages.h"

using namespace std;

Register::Register(const string& _server_hostname, int _server_port,
        const string& _function_name, int* _arg_types) {
    server_hostname = _server_hostname;
    server_port = _server_port;
    function_name = _function_name;

    int num_args = numberOfArgs(_arg_types);
    arg_types = new int[num_args + 1];
    arg_types[num_args] = 0;
    for (int i = 0; i < num_args; i++)
        arg_types[i] = _arg_types[i];
}

Register::~Register() {
    delete [] arg_types;
}

string Register::getServerHostname() const { return server_hostname; }
int Register::getServerPort() const { return server_port; }
string Register::getFunctionName() const { return function_name; }
int* Register::getArgTypes() const { return arg_types; }
