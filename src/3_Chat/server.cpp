#include <cassert>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <zmqpp/zmqpp.hpp>

class User {
private:
    std::string name;
    std::string password;
    std::string netId;
    bool connected;
    std::list<std::string> contacts;

public:
    User() {}
    User(const std::string& name, const std::string& pwd, const std::string& id)
          : name(name), password(pwd), netId(id), connected(false) {}

    bool isPassword(const std::string& pwd) const {
        return password == pwd;
    }
    void connect(const std::string& id) {
        connected = true;
        netId = id;
    }

    bool inContacts(std::string& user) {
        for (size_t i = 0; i < contacts.size(); ++i) {
            if (contacts.front() == user) return true;
            contacts.pop_front();
        }
        return false;
    }

    bool isConnected(void) {
        return connected;
    }
};

class ServerState {
private:
    // connected users
    std::unordered_map<std::string, User> users;

public:
    ServerState() {}

    void newUser(const std::string& name, const std::string& pwd,
                 const std::string& id) {
        users[name] = User(name, pwd, id);
    }

    bool login(const std::string& name, const std::string& pwd,
               const std::string& id) {
        if (users.count(name)) {
            // User is registered
            bool ok = users[name].isPassword(pwd);
            if (ok) users[name].connect(id);
            return ok;
        }
        return false;
    }

    User getUser(std::string& name) {
        if (users.count(name))
            return users[name];
        else {
            return User();  // No estoy seguro si esto funciona
        }
    }
};

void login(zmqpp::message& msg, const std::string& sender,
           ServerState& server) {
    std::string userName;
    msg >> userName;
    std::string password;
    msg >> password;
    if (server.login(userName, password, sender)) {
        std::cout << "User " << userName << " joins the chat server"
                  << std::endl;
    } else {
        std::cerr << "Wrong user/password " << std::endl;
    }
}

void sendMessage(zmqpp::message& msg, const std::string& /*sender*/,
                 ServerState& server) {
    std::string nameSender;  // origen del mensaje
    msg >> nameSender;

    std::string nameDest;  // Nombre de usuario destino o grupo
    msg >> nameDest;

    if (server.getUser(nameSender).inContacts(nameDest) &&
        server.getUser(nameDest).isConnected()) {  // Envio uno a uno
        std::string text;
        msg >> text;
        // aqui se supone que se hace el envio a un solo usuario por el
        // zmqpp::socket
        // } else if () {  // Envio uno a muchos
    } else {
        std::cerr << "User not found in contacts/offline" << std::endl;
    }
}

void dispatch(zmqpp::message& msg, ServerState& server) {
    assert(msg.parts() > 2);
    std::string sender;
    msg >> sender;

    std::string action;
    msg >> action;

    if (action == "login") {
        login(msg, sender, server);

    } else if (action == "newUser") {
        std::string name;
        msg >> name;

        std::string pwd;
        msg >> pwd;

        server.newUser(name, pwd, sender);

    } else if (action == "zmqpp::message") {
        sendMessage(msg, sender, server);
    } else {
        std::cerr << "Action not supported/implemented" << std::endl;
    }
}

int main(/*int argc, char* argv[]*/) {
    const std::string endpoint = "tcp://*:4242";
    zmqpp::context ctx;

    zmqpp::socket s(ctx, zmqpp::socket_type::xreply);
    s.bind(endpoint);

    ServerState state;
    state.newUser("sebas", "123", "");
    while (true) {
        zmqpp::message req;
        s.receive(req);
        dispatch(req, state);
    }

    std::cout << "Finished." << std::endl;
}

// Formato para enviar mensaje luego de haber sido logueado
// Id | action | Name | Name destino | mensaje de texto
