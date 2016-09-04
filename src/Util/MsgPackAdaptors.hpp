#pragma once

#define MSGPACK_API_VERSION 2
#include <msgpack.hpp>

template <typename T>
class Matrix;

enum class ServerCodes : int;

// User defined class template specialization
namespace msgpack {
inline namespace v2 {
namespace adaptor {

template <typename T>
struct convert<Matrix<T>> {
    const msgpack::object& operator()(const msgpack::object& o,
                                      Matrix<T>& m) const {
        if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
        if (o.via.array.size != 3) throw msgpack::type_error();
        m = Matrix<T>(o.via.array.ptr[0].as<size_t>(),
                      o.via.array.ptr[1].as<size_t>());
        m.SetData(o.via.array.ptr[2].as<std::vector<T>>());
        return o;
    }
};

template <typename T>
struct pack<Matrix<T>> {
    template <typename Stream>
    packer<Stream>& operator()(msgpack::packer<Stream>& o,
                               const Matrix<T>& m) const {
        // packing member variables as an array.
        o.pack_array(3);
        o.pack(m.NumCols());
        o.pack(m.NumRows());
        o.pack(m.GetData());
        return o;
    }
};

template <>
struct convert<ServerCodes> {
    const msgpack::object& operator()(const msgpack::object& o,
                                      ServerCodes& m) const {
        if (o.type != msgpack::type::POSITIVE_INTEGER &&
            o.type != msgpack::type::NEGATIVE_INTEGER)
            throw msgpack::type_error();
        m = static_cast<ServerCodes>(o.via.i64);
        return o;
    }
};

template <>
struct pack<ServerCodes> {
    template <typename Stream>
    packer<Stream>& operator()(msgpack::packer<Stream>& o,
                               const ServerCodes& m) const {
        // packing member variables as an array.
        o.pack(static_cast<int>(m));
        return o;
    }
};

}  // namespace adaptor
}  // namespace v2
}  // namespace msgpack
