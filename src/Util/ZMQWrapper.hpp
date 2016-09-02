#pragma once

#include <zmq.hpp>

namespace zmqw {

// Wrapper classes around C++ ZMQ

class socket : public zmq::socket_t {
public:
    // Inherite contructors
    using zmq::socket_t::socket_t;

    template <typename T>
    bool recv(T& obj, int flags = 0) {
        zmq::message_t msg;
        bool result = zmq::socket_t::recv(&msg, flags);
        if (result) {
            obj = T(static_cast<char*>(msg.data()), msg.size());
        }
        return result;
    }

    template <typename T>
    bool send(const T& obj, int flags = 0) {
        return zmq::socket_t::send(obj.data(), obj.size(), flags);
    }
};

}
