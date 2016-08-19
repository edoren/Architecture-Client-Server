#include <cctype>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

template <typename T>
class Matrix {
public:
    Matrix() : rows_(0), cols_(0), data_(0) {}

    Matrix(size_t rows, size_t cols)
          : rows_(rows), cols_(cols), data_(rows * cols) {}

    Matrix<T> operator*(const Matrix<T>& other) {
        if (NumCols() != other.NumRows()) {
            std::stringstream stream;
            stream << "can't multiply matrices of size [" << NumRows() << ", "
                   << NumCols() << "] and [" << other.NumRows() << ", "
                   << other.NumCols() << "]\n";
            throw std::runtime_error(stream.str());
        }

        Matrix<T> result(NumRows(), other.NumCols());

        std::cout << "rows:" << NumRows() << " cols:" << NumCols() << "\n";
        std::cout << "rows:" << other.NumRows() << " cols:" << other.NumCols() << "\n";

        for (size_t j = 0; j < NumRows(); j++) {
            for (size_t i = 0; i < other.NumCols(); i++) {
                for (size_t k = 0; k < NumCols(); k++) {
                    result(i, j) += (*this)(k, j) * other(i, k);
                }
            }
        }

        return result;
    }

    void SetSize(size_t rows, size_t cols) {
        rows_ = rows;
        cols_ = cols;
        data_.resize(rows_ * cols_);
    }

    bool SetData(const std::vector<T>& other) {
        if (other.size() == data_.size()) {
            data_ = other;
            return true;
        }
        return false;
    }

    T& operator()(size_t x, size_t y) {
        return data_[cols_ * y + x];
    }

    const T& operator()(size_t x, size_t y) const {
        return data_[cols_ * y + x];
    }

    size_t NumRows() const {
        return rows_;
    }

    size_t NumCols() const {
        return cols_;
    }

private:
    size_t rows_;
    size_t cols_;
    std::vector<T> data_;
};

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

    for(auto& lol : v)
        std::cout << lol << ",";
    std::cout << "\n";

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
            std::cout << matrix1_data << "\n";
            std::cout << matrix2_data << "\n";

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
