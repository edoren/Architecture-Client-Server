#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include <Util/Matrix.hpp>

template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& m) {
    os << "[";
    for (size_t j = 0; j < m.NumRows(); j++) {
        // if (j != 0) os << " ";
        os << "[";
        for (size_t i = 0; i < m.NumCols(); i++) {
            os << m(i, j);
            if (i != m.NumCols() - 1) os << ",";
            // if (i != m.NumCols() - 1) os << ", ";
        }
        os << "]";
        // if (j != m.NumRows() - 1) os << "\n";
    }
    os << "]";
    return os;
}

int ParseMatrix(const std::string& str, Matrix<float>& m) {
    int ncols = -1;
    int nrows = 0;
    int num_brackets = 0;

    int num_numbers = 0;

    std::vector<float> v;

    std::string number_str;

    auto add_number = [&number_str, &v]() -> bool {
        float number = std::stof(number_str);
        v.push_back(number);
        number_str.clear();
        return true;
    };

    for (auto& token : str) {
        if (std::isdigit(token) || token == '.') {
            number_str.push_back(token);
        } else if (token == ',') {
            add_number();
            num_numbers += 1;
        } else if (token == '[') {
            num_brackets++;
        } else if (token == ']') {
            num_brackets--;
            if (!number_str.empty()) {
                num_numbers += 1;
                nrows += 1;

                add_number();

                if (ncols == -1) {
                    ncols = num_numbers;
                } else if (num_numbers != ncols) {
                    // inconsistent row size
                    return 1;
                }

                num_numbers = 0;
            }
        }
    }

    if (num_brackets != 0) {
        // missmatched brackets
        return 2;
    }

    m.SetSize(nrows, ncols);
    m.SetData(v);

    return 0;
}

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
        request >> operation;
        std::cout << "Operation: " << operation << "\n";

        Matrix<float> result;

        if (operation == "mul") {
            std::string matrix1_data, matrix2_data;
            request >> matrix1_data >> matrix2_data;
            std::cout << "First Matrix: " << matrix1_data << "\n";
            std::cout << "Second Matrix: " << matrix2_data << "\n";

            Matrix<float> matrix1, matrix2;

            ParseMatrix(matrix1_data, matrix1);
            ParseMatrix(matrix2_data, matrix2);

            result = matrix1 * matrix2;
        } else if (operation == "det") {
            // TODO: :v
        }

        std::stringstream stream;
        stream << result;

        response << stream.str();
        socket.send(response);
        std::cout << "Sent: " << result << "\n";
    }
}
