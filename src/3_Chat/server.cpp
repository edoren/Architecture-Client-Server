#include <cassert>
#include <ios>
#include <iostream>
#include <list>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

#include <zmq.hpp>

#include <Util/Serializer.hpp>
#include <Util/UUID.hpp>

class User {
public:
    User() {}
    User(const std::string& name, const std::string& pwd)
          : name_(name), password_(pwd), connected_(false) {
        token_ = UUID::UUID4().AsString();
    }

    bool IsPassword(const std::string& pwd) const {
        return password_ == pwd;
    }

    void Connect() {
        connected_ = true;
    }

    bool InContacts(const std::string& user) {
        for (size_t i = 0; i < contacts_.size(); i++) {
            if (contacts_.front() == user) return true;
            contacts_.pop_front();
        }
        return false;
    }

    const std::string& GetToken() const {
        return token_;
    }

    bool IsConnected() {
        return connected_;
    }

private:
    std::string name_;
    std::string password_;
    std::string token_;
    bool connected_;
    std::list<std::string> contacts_;
};

class ServerState {
private:
    // connected users
    std::unordered_map<std::string, User> users;

public:
    ServerState() {}

    bool Login(const std::string& name, const std::string& pwd) {
        if (users.count(name)) {
            // User is registered
            bool ok = users[name].IsPassword(pwd);
            if (ok) users[name].Connect();
            return ok;
        }
        return false;
    }

    void Register(const std::string& name, const std::string& pwd) {
        users[name] = User(name, pwd);
    }

    User GetUser(const std::string& name) {
        if (users.count(name))
            return users[name];
        else {
            return User();  // No estoy seguro si esto funciona
        }
    }
};

bool Login(const std::string& username, const std::string& password,
           ServerState& server) {
    if (server.Login(username, password)) {
        std::cout << "User " << username << " joins the chat server\n";
        return true;
    } else {
        std::cerr << "Wrong user/password\n";
        return false;
    }
}

void SendMessage(const std::string& sender, const std::string& receiver,
                 const std::string& /*text*/, ServerState& server) {
    if (server.GetUser(sender).InContacts(receiver) &&
        server.GetUser(receiver).IsConnected()) {  // Envio uno a uno
        // aqui se supone que se hace el envio a un solo usuario por el
        // zmq::socket
        // } else if () {  // Envio uno a muchos
    } else {
        std::cerr << "User not found in contacts_/offline" << std::endl;
    }
}

void Dispatch(zmq::socket_t& socket, ServerState& server) {
    zmq::message_t msg;
    socket.recv(&msg);
    Deserializer request(static_cast<char*>(msg.data()), msg.size());

    std::string action;
    request >> action;

    Serializer response;

    if (action == "login") {
        std::string username, password;
        request >> username >> password;
        Login(username, password, server);
        response << server.GetUser(username).GetToken();
    } /*else if (action == "register") {
        std::string username, password;
        request >> username >> password;
        server.Register(username, password);
    } else if (action == "message") {
        std::string username_sender, username_dest, text;  // origen del mensaje
        request >> username_sender >> username_dest >> text;
        SendMessage(username_sender, username_dest, text, server);
    }*/ else {
        std::cerr << "Action not supported/implemented" << std::endl;
    }

    socket.send(response.data(), response.size());
}

int main(/*int argc, char* argv[]*/) {
    const std::string endpoint = "tcp://*:4242";
    zmq::context_t ctx;

    zmq::socket_t socket(ctx, ZMQ_REP);
    socket.bind(endpoint);

    ServerState state;
    state.Register("edoren", "123");
    while (true) {
        Dispatch(socket, state);
    }

    std::cout << "Finished." << std::endl;
}
