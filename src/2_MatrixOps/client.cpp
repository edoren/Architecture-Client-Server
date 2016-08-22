#include <string>
#include <zmqpp/zmqpp.hpp>

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
    zmqpp::context context;

    // generate a request socket
    zmqpp::socket_type type = zmqpp::socket_type::request;
    zmqpp::socket socket(context, type);
    socket.connect(endpoint);

    // send a message
    zmqpp::message request, response;

    // compose a message from a operation and a matrices
    request << operation;
    for (auto& matrix : matrices) {
        request << matrix;
    }
    std::cout << "Sending matrices.\n";
    socket.send(request);

    std::string result;
    socket.receive(response);
    response >> result;

    std::cout << "Result: " << result << "\n";

    return 0;
}
