#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <zmq.hpp>

#include <Util/Serializer.hpp>
#include <Util/Matrix.hpp>
#include <Util/LinearAlgebra.hpp>

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
        if (std::isdigit(token) || token == '.' || token == '-') {
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
    zmq::context_t context;

    // generate a reply socket
    zmq::socket_t socket(context, ZMQ_REP);

    // bind to the socket
    std::cout << "Binding to " << endpoint << "...\n";
    socket.bind(endpoint);

    while (true) {
        // receive the message and process it
        zmq::message_t msg;
        socket.recv(&msg);
        Deserializer request(static_cast<char*>(msg.data()), msg.size());
        Serializer response;

        std::cout << "Receiving message...\n";

        std::string operation;
        request >> operation;
        std::cout << "Operation: " << operation << "\n";

        std::stringstream stream;

        if (operation == "mul") {
            std::string matrix1_data, matrix2_data;
            request >> matrix1_data >> matrix2_data;

            Matrix<float> matrix1, matrix2;

            ParseMatrix(matrix1_data, matrix1);
            ParseMatrix(matrix2_data, matrix2);
            std::cout << "First Matrix: " << matrix1 << "\n";
            std::cout << "Second Matrix: " << matrix2 << "\n";

            Matrix<float> result = matrix1 * matrix2;

            stream << result;
            std::cout << "Sent: " << result << "\n";
        } else if (operation == "det") {
            std::string matrix_data;
            request >> matrix_data;

            Matrix<float> matrix;

            ParseMatrix(matrix_data, matrix);

            float value = Determinant(matrix);
            std::cout << "Matrix: " << matrix << "\n";

            stream << value;
            std::cout << "Sent: " << value << "\n";
        } else if (operation == "inverse") {
            std::string matrix_data;
            request >> matrix_data;

            Matrix<float> matrix;

            ParseMatrix(matrix_data, matrix);

            Matrix<float> result = Inverse(matrix);
            std::cout << "Matrix: " << matrix << "\n";

            stream << result;
            std::cout << "Sent: " << result << "\n";
        }

        response << stream.str();

        socket.send(response.data(), response.size());
    }
}
