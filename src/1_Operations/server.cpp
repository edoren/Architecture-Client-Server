#include <cmath>
#include <iostream>
#include <string>
#include <zmq.hpp>

#include <Util/Serializer.hpp>

int main(int /*argc*/, char** /*argv*/) {
    const std::string endpoint = "tcp://*:4242";

    // initialize the 0MQ context
    zmq::context_t context(1);

    // generate a reply socket
    zmq::socket_t socket(context, ZMQ_REP);

    // bind to the socket
    std::cout << "Binding to " << endpoint << "...\n";
    socket.bind(endpoint);

    while (true) {
        // Get the serialized data
        zmq::message_t msg;
        socket.recv(&msg);
        Deserializer request(static_cast<char*>(msg.data()), msg.size());

        std::cout << "Receiving message...\n";

        std::string operation;
        float operand1 = 0;
        float operand2 = 1;
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

        // Fill the data to send
        Serializer response;
        response << result;

        socket.send(response.data(), response.size());

        std::cout << "Sent: " << result << "\n";
    }
}
