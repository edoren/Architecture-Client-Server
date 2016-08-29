#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <zmq.hpp>

#include <Util/Serializer.hpp>

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

    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_REQ);

    std::cout << "Connecting to: " << sckt << std::endl;
    socket.connect(sckt);

    // Send the login request
    Serializer request;
    request << "login" << userName << password;
    socket.send(request.data(), request.size());

    zmq::pollitem_t items[] = {{socket, 0, ZMQ_POLLIN, 0},
                               {nullptr, fileno(stdin), ZMQ_POLLIN, 0}};

    while (true) {
        zmq::message_t msg;
        zmq::poll(items, 2, -1);
        // Wait for the login response
        if (items[0].revents & ZMQ_POLLIN) {
            socket.recv(&msg);
            Deserializer response(static_cast<char*>(msg.data()), msg.size());
            std::string token;
            response >> token;
            std::cout << "Login Token: " << token << "\n";
        }
        if (items[1].revents & ZMQ_POLLIN) {
            std::string line;
            std::getline(std::cin, line);
            std::cout << line << '\n';
        }
    }

    return 0;
}
