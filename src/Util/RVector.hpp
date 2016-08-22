#pragma once

#include <sstream>
#include <stdexcept>
#include <vector>

template <typename T>
class Matrix;

///////////////////////////////////////////////////////////////////////////////
// Reference Vector
// Used for Matrix Row or Column access
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class RVector {
public:
    RVector() : data_(0) {}

    RVector(const RVector<T>& other) : data_(other.data_) {}

private:
    RVector(std::vector<T*>&& data) : data_(data) {}

public:
    bool SetData(const std::vector<T>& other) {
        if (other.size() <= data_.size()) {
            for (int i = 0; i < other.size(); ++i) {
                *data_[i] = other[i];
            }
            return true;
        }
        return false;
    }

    size_t Size() const {
        return data_.size();
    }

    T& operator[](size_t pos) {
        return *data_[pos];
    }

    const T& operator[](size_t pos) const {
        return *data_[pos];
    }

    T& operator()(size_t pos) {
        return *data_[pos];
    }

    const T& operator()(size_t pos) const {
        return *data_[pos];
    }

public:
    friend class Matrix<T>;

private:
    std::vector<T*> data_;
};
