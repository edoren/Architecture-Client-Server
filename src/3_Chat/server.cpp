#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <zmqpp/zmqpp.hpp>

using namespace std;
using namespace zmqpp;

class User {
private:
    string name;
    string password;
    string netId;
    bool connected;
    list<string> contacts;

public:
    User() {}
    User(const string& name, const string& pwd, const string& id)
          : name(name), password(pwd), netId(id), connected(false) {}

    bool isPassword(const string& pwd) const {
        return password == pwd;
    }
    void connect(const string& id) {
        connected = true;
        netId = id;
    }

    bool inContacts(string& user) {
        for (int i = 0; i < contacts.size(); ++i) {
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
    unordered_map<string, User> users;

public:
    ServerState() {}

    void newUser(const string& name, const string& pwd, const string& id) {
        users[name] = User(name, pwd, id);
    }

    bool login(const string& name, const string& pwd, const string& id) {
        if (users.count(name)) {
            // User is registered
            bool ok = users[name].isPassword(pwd);
            if (ok) users[name].connect(id);
            return ok;
        }
        return false;
    }

    User getUser(string& name) {
        if (users.count(name))
            return users[name];
        else {
            return User();  // No estoy seguro si esto funciona
        }
    }
};

void login(message& msg, const string& sender, ServerState& server) {
    string userName;
    msg >> userName;
    string password;
    msg >> password;
    if (server.login(userName, password, sender)) {
        cout << "User " << userName << " joins the chat server" << endl;
    } else {
        cerr << "Wrong user/password " << endl;
    }
}

void sendMessage(message& msg, const string& sender, ServerState& server) {
    string nameSender;  // origen del mensaje
    msg >> nameSender;

    string nameDest;  // Nombre de usuario destino o grupo
    msg >> nameDest;

    if (server.getUser(nameSender).inContacts(nameDest) &&
        server.getUser(nameDest).isConnected()) {  // Envio uno a uno
        string text;
        msg >> text;
        // aqui se supone que se hace el envio a un solo usuario por el socket
        // } else if () {  // Envio uno a muchos
    } else {
        cerr << "User not found in contacts/offline" << endl;
    }
}

void dispatch(message& msg, ServerState& server) {
    assert(msg.parts() > 2);
    string sender;
    msg >> sender;

    string action;
    msg >> action;

    if (action == "login") {
        login(msg, sender, server);

    } else if (action == "newUser") {
        string name;
        msg >> name;

        string pwd;
        msg >> pwd;

        server.newUser(name, pwd, sender);

    } else if (action == "message") {
        sendMessage(msg, sender, server);
    } else {
        cerr << "Action not supported/implemented" << endl;
    }
}

int main(int argc, char* argv[]) {
    const string endpoint = "tcp://*:4242";
    context ctx;

    socket s(ctx, socket_type::xreply);
    s.bind(endpoint);

    ServerState state;
    state.newUser("sebas", "123", "");
    while (true) {
        message req;
        s.receive(req);
        dispatch(req, state);
    }

    cout << "Finished." << endl;
}

// Formato para enviar mensaje luego de haber sido logueado
// Id | action | Name | Name destino | mensaje de texto
