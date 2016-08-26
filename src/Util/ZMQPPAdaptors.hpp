#pragma once

#include <zmqpp/zmqpp.hpp>

#include <Util/Serializer.hpp>

zmqpp::message& operator<<(zmqpp::message& msg, const Buffer& buffer) {
    msg.add_raw(reinterpret_cast<const void*>(buffer.data()), buffer.size());
    return msg;
}

zmqpp::message& operator>>(zmqpp::message& msg, Buffer& buffer) {
    size_t part = msg.read_cursor();
    const char* data = static_cast<const char*>(msg.raw_data(part));
    size_t len = msg.size(part);
    msg.next();
    buffer.write(data, len);
    return msg;
}
