#pragma once

#include <cstdint>
#include <cstring>

#include "MsgPackAdaptors.hpp"

class Serializer {
public:
    Serializer() : buffer_(), packer_(buffer_) {}

    template <typename D>
    Serializer& operator<<(const D& data) {
        packer_.pack(data);
        return *this;
    }

    char* data() {
        return buffer_.data();
    }

    const char* data() const {
        return buffer_.data();
    }

    size_t size() const {
        return buffer_.size();
    };

private:
    msgpack::sbuffer buffer_;
    msgpack::packer<msgpack::sbuffer> packer_;
};

class Deserializer {
public:
    Deserializer(const char* data, size_t size) {
        unpacker_.reserve_buffer(size);
        std::memcpy(unpacker_.buffer(), data, size);
        unpacker_.buffer_consumed(size);
    }

    Deserializer(const msgpack::sbuffer& buffer)
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
