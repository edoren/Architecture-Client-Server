#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>

using namespace std;
using namespace zmqpp;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Invalid arguments" << endl;
        return EXIT_FAILURE;
    }

    string address(argv[1]);
    string userName(argv[2]);
    string password(argv[3]);
    string sckt("tcp://");
    sckt += address;

    context ctx;
    socket s(ctx, socket_type::xrequest);

    cout << "Connecting to: " << sckt << endl;
    s.connect(sckt);

    message login;
    login << "login" << userName << password;
    s.send(login);

    while (true) {
    }

    return 0;
}