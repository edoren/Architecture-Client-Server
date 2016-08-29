#include <iostream>
#include <string>
#include <zmq.hpp>

#include <Util/Serializer.hpp>

int main(int argc, char** argv) {
    const std::string endpoint = "tcp://localhost:5555";

    if (argc < 2) {
        std::cout << "usage: " << argv[0]
                  << " [sum|sub|mul|div|sqrt|exp] operands...\n";
        return 1;
    }

    std::string operation(argv[1]);

    if (argc != 4 && (operation == "sum" || operation == "sub" ||
                      operation == "mul" || operation == "div")) {
        std::cerr << "Invalid number of operands, expected 2.\n";
        return 2;
    } else if (argc != 3 && (operation == "sqrt" || operation == "exp")) {
        std::cerr << "Invalid number of operands, expected 1.\n";
        return 2;
    }

    int num_operands = argc - 2;
    std::vector<float> operands(num_operands);

    for (int i = 0; i < num_operands; ++i) {
        try {
            operands[i] = std::stof(std::string(argv[2 + i]));
        } catch (std::exception& e) {
            std::cerr << "Invalid operand, expected a number.\n";
            return 3;
        }
    }

    // initialize the 0MQ context
    zmq::context_t context(1);

    // generate a request socket
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect(endpoint);

    // send a message
    Serializer request;

    // compose a message from a operation and a operands
    request << operation;
    for (auto& operand : operands) {
        request << operand;
    }
    std::cout << "Sending operands.\n";
    socket.send(request.data(), request.size());

    float result;

    zmq::message_t msg;
    socket.recv(&msg);
    Deserializer response(static_cast<char*>(msg.data()), msg.size());

    response >> result;

    std::cout << "Result: " << result << "\n";

    return 0;
}
