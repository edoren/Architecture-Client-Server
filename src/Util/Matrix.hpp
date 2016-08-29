#pragma once

#include <sstream>
#include <stdexcept>
#include <vector>

#include "RVector.hpp"

template <typename T>
class Matrix {
public:
    Matrix() : rows_(0), cols_(0), data_(0) {}

    Matrix(const Matrix<T>& other)
          : rows_(other.rows_), cols_(other.cols_), data_(other.data_) {}

    Matrix(size_t rows, size_t cols)
          : rows_(rows), cols_(cols), data_(rows * cols, T(0)) {}

    Matrix<T> operator*(const Matrix<T>& other) {
        if (NumCols() != other.NumRows()) {
            std::stringstream stream;
            stream << "can't multiply matrices of size [" << NumRows() << ", "
                   << NumCols() << "] and [" << other.NumRows() << ", "
                   << other.NumCols() << "]\n";
            throw std::runtime_error(stream.str());
        }

        Matrix<T> result(NumRows(), other.NumCols());

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

    const std::vector<T>& GetData() const {
        return data_;
    }

    RVector<T> GetRow(size_t pos) {
        if (pos >= NumRows()) {
            // If out of bounds return an empty RVector
            return RVector<T>();
        }

        std::vector<T*> data(NumCols(), nullptr);

        for (size_t i = 0; i < NumCols(); i++) {
            data[i] = &(*this)(i, pos);
        }

        return RVector<T>(data);
    }

    RVector<T> GetCol(size_t pos) {
        if (pos >= NumCols()) {
            // If out of bounds return an empty RVector
            return RVector<T>();
        }

        std::vector<T*> data(NumRows(), nullptr);

        for (size_t j = 0; j < NumRows(); j++) {
            data[j] = &(*this)(pos, j);
        }

        return RVector<T>(data);
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
