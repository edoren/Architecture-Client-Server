#include <cmath>
#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>

int main(int /*argc*/, char** /*argv*/) {
    const std::string endpoint = "tcp://*:4242";

    // initialize the 0MQ context
    zmqpp::context context;

    // generate a reply socket
    zmqpp::socket_type type = zmqpp::socket_type::reply;
    zmqpp::socket socket(context, type);

    // bind to the socket
    std::cout << "Binding to " << endpoint << "...\n";
    socket.bind(endpoint);

    while (true) {
        // receive the message and process it
        zmqpp::message request, response;
        socket.receive(request);

        std::cout << "Receiving message...\n";

        std::string operation;
        float operand1;
        float operand2;
        float result;

        request >> operation;
        std::cout << "Operation: " << operation << "\n";

        if (operation == "sum") {
            request >> operand1 >> operand2;
            result = operand1 + operand2;
        } else if (operation == "sub") {
            request >> operand1 >> operand2;
            result = operand1 - operand2;
        } else if (operation == "mul") {
            request >> operand1 >> operand2;
            result = operand1 * operand2;
        } else if (operation == "div") {
            request >> operand1 >> operand2;
            if (operand2 != 0) {
                result = operand1 / operand2;
            } else {
                result = 0.f;
            }
        } else if (operation == "sqrt") {
            request >> operand1;
            result = std::sqrt(operand1);
        } else if (operation == "exp") {
            request >> operand1;
            result = std::exp(operand1);
        } else {
            std::cerr << "Invalid operation.\n";
            result = 0.f;
        }

        response << result;
        socket.send(response);
        std::cout << "Sent: " << result << "\n";
    }
}
