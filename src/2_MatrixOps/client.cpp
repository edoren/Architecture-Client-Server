#include <iostream>
#include <string>
#include <zmq.hpp>

#include <Util/Serializer.hpp>

int main(int argc, char** argv) {
    const std::string endpoint = "tcp://localhost:4242";

    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " [mul|det|inverse] matrices...\n";
        return 1;
    }

    std::string operation(argv[1]);

    if (argc != 4 && operation == "mul") {
        std::cerr << "Invalid number of matrices, expected 1.\n";
        return 2;
    } else if (argc != 3 && (operation == "det" || operation == "inverse")) {
        std::cerr << "Invalid number of matrices, expected 1.\n";
        return 2;
    }

    int num_matrices = argc - 2;
    std::vector<std::string> matrices(num_matrices);

    for (int i = 0; i < num_matrices; ++i) {
        matrices[i] = std::string(argv[2 + i]);
    }

    // initialize the 0MQ context
    zmq::context_t context;

    // generate a request socket
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect(endpoint);

    // send a message
    Serializer request;

    // compose a message from a operation and a matrices
    request << operation;
    for (auto& matrix : matrices) {
        request << matrix;
    }
    std::cout << "Sending matrices.\n";
    socket.send(request.data(), request.size());

    std::string result;

    zmq::message_t msg;
    socket.recv(&msg);
    Deserializer response(static_cast<char*>(msg.data()), msg.size());

    response >> result;

    std::cout << "Result: " << result << "\n";

    return 0;
}
