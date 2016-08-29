#pragma once

#include "Matrix.hpp"

template <typename T>
Matrix<T> UpperTriangularMatrix(const Matrix<T>& m) {
    Matrix<T> result(m);

    // Get the Upper Triangular Matrix
    T pivot = T(0);
    T aux = T(0);
    for (size_t diagonal = 0; diagonal < result.NumCols(); diagonal++) {
        // Swap pivot-zero rows
        pivot = result(diagonal, diagonal);
        if (pivot == T(0)) {
            for (size_t row = diagonal + 1; row < result.NumRows(); row++) {
                aux = result(diagonal, row);
                if (aux != 0) {
                    for (size_t column = 0; column < result.NumCols();
                         ++column) {
                        std::swap(result(column, row),
                                  result(column, diagonal));
                    }
                    pivot = aux;
                    break;
                }
            }
        }

        for (size_t row = diagonal + 1; row < result.NumRows(); row++) {
            aux = result(diagonal, row);
            for (size_t column = 0; column < result.NumCols(); column++) {
                result(column, row) -= result(column, diagonal) * (aux / pivot);
            }
        }
    }

    return result;
}

template <typename T>
Matrix<T> LowerTriangularMatrix(const Matrix<T>& m) {
    Matrix<T> result(m);

    // Get the Lower Triangular Matrix
    T pivot = T(0);
    T aux = T(0);
    for (int diagonal = result.NumCols() - 1; diagonal >= 0; diagonal--) {
        // Swap pivot-zero rows
        pivot = result(diagonal, diagonal);
        if (pivot == T(0)) {
            for (size_t row = diagonal + 1; row < result.NumRows(); row++) {
                aux = result(diagonal, row);
                if (aux != 0) {
                    for (size_t column = 0; column < result.NumCols();
                         ++column) {
                        std::swap(result(column, row),
                                  result(column, diagonal));
                    }
                    pivot = aux;
                    break;
                }
            }
        }

        for (int row = diagonal - 1; row >= 0; row--) {
            T aux = result(diagonal, row);
            for (size_t column = 0; column < result.NumCols(); column++) {
                result(column, row) -= result(column, diagonal) * (aux / pivot);
            }
        }
    }

    return result;
}

template <typename T>
T Determinant(const Matrix<T>& m) {
    Matrix<T> U = UpperTriangularMatrix(m);

    // Find the Determinant which is the equal of the
    // product of the diagonal values of a triangular matrix
    T result = T(1);
    for (size_t i = 0; i < U.NumCols(); i++) {
        result *= U(i, i);
    }

    return result;
}

template <typename T>
Matrix<T> Inverse(const Matrix<T>& m) {
    Matrix<T> copy(m);
    Matrix<T> result(copy.NumCols(), copy.NumRows());

    // Create an identity matrix
    for (size_t i = 0; i < result.NumCols(); ++i) {
        result(i, i) = T(1);
    }

    T pivot = T(0);
    T aux = T(0);

    for (size_t diagonal = 0; diagonal < copy.NumCols(); diagonal++) {
        // Swap pivot-zero rows
        pivot = copy(diagonal, diagonal);
        if (pivot == T(0)) {
            for (size_t row = diagonal + 1; row < copy.NumRows(); row++) {
                aux = copy(diagonal, row);
                if (aux != 0) {
                    for (size_t column = 0; column < copy.NumCols(); ++column) {
                        std::swap(copy(column, row), copy(column, diagonal));
                        std::swap(result(column, row),
                                  result(column, diagonal));
                    }
                    pivot = aux;
                    break;
                }
            }
        }

        // Divide by the pivot to make the diagonal ones
        for (size_t column = 0; column < copy.NumCols(); column++) {
            copy(column, diagonal) /= pivot;
            result(column, diagonal) /= pivot;
        }

        // Get the Upper Triangular Matrix
        for (size_t row = diagonal + 1; row < copy.NumRows(); row++) {
            aux = copy(diagonal, row);
            for (size_t column = 0; column < copy.NumCols(); column++) {
                copy(column, row) -= copy(column, diagonal) * aux;
                result(column, row) -= result(column, diagonal) * aux;
            }
        }
    }

    // Get the Lower Triangular Matrix
    for (int diagonal = copy.NumCols() - 1; diagonal >= 0; diagonal--) {
        for (int row = diagonal - 1; row >= 0; row--) {
            T aux = copy(diagonal, row);
            for (size_t column = 0; column < copy.NumCols(); column++) {
                copy(column, row) -= copy(column, diagonal) * aux;
                result(column, row) -= result(column, diagonal) * aux;
            }
        }
    }

    return result;
}
