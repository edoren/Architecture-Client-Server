#pragma once

#include <cstdint>
#include <cstring>

#include "MsgPackAdaptors.hpp"

using Buffer = msgpack::sbuffer;

class Serializer {
public:
    Serializer(Buffer& stream) : packer_(stream) {}

    template <typename D>
    Serializer& operator<<(const D& data) {
        packer_.pack(data);
        return *this;
    }

private:
    msgpack::packer<Buffer> packer_;
};

class Deserializer {
public:
    Deserializer(const char* data, size_t size) {
        unpacker_.reserve_buffer(size);
        std::memcpy(unpacker_.buffer(), data, size);
        unpacker_.buffer_consumed(size);
    }

    Deserializer(const Buffer& buffer)
          : Deserializer(buffer.data(), buffer.size()) {}

    template <typename D>
    Deserializer& operator>>(D& data) {
        msgpack::object_handle message;
        if (unpacker_.next(message)) {
            data = message.get().as<D>();
        }
        return *this;
    }

private:
    msgpack::unpacker unpacker_;
};
