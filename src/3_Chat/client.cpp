#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Invalid arguments" << std::endl;
        return EXIT_FAILURE;
    }

    std::string address(argv[1]);
    std::string userName(argv[2]);
    std::string password(argv[3]);
    std::string sckt("tcp://");
    sckt += address;

    zmqpp::context ctx;
    zmqpp::socket s(ctx, zmqpp::socket_type::xrequest);

    std::cout << "Connecting to: " << sckt << std::endl;
    s.connect(sckt);

    zmqpp::message login;
    login << "login" << userName << password;
    s.send(login);

    while (true) {
    }

    return 0;
}
