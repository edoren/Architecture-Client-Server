#include <cassert>
#include <csignal>

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <Util/Serializer.hpp>
#include <Util/UUID.hpp>
#include <Util/ZMQWrapper.hpp>

#include "ServerCodes.hpp"

static volatile std::sig_atomic_t gSignalStatus = 0;

static void gSignalHandler(int signal_value) {
    gSignalStatus = signal_value;
}

using NetIdentity = std::string;

class User {
public:
    User() {}

    User(const std::string& username, const std::string& password)
          : username_(username), password_(password) {}

    const std::string& GetUsername() const {
        return username_;
    }

    bool IsPassword(const std::string& password) const {
        return password_ == password;
    }

    bool AddContact(const std::string& user) {
        return contacts_.insert(user).second;
    }

private:
    std::string username_;
    std::string password_;
    std::unordered_set<std::string> contacts_;
};

class Group {
public:
    Group() {}

    Group(const std::string& name, const std::string& owner)
          : name_(name), owner_(owner) {}

    bool AddMember(const std::string& username) {
        return members_.insert(username).second;
    }

    bool RemoveMember(const std::string& username) {
        return members_.erase(username) > 0;
    }

    bool IsMember(const std::string& username) const {
        auto it = members_.find(username);
        return it != members_.end();
    }

    const std::string& GetName() const {
        return name_;
    }

private:
    std::string name_;
    std::string owner_;
    std::unordered_set<std::string> members_;
};

class DataBase {
public:
    User& AddUser(User&& user) {
        return users_[user.GetUsername()] = user;
    }

    Group& AddGroup(Group&& group) {
        return groups_[group.GetName()] = group;
    }

    User& GetUser(const std::string& username) {
        return users_.find(username)->second;
    }

    bool UserExists(const std::string& username) const {
        auto it = users_.find(username);
        return it != users_.end();
    }

    bool GroupExists(const std::string& group_name) const {
        auto it = groups_.find(group_name);
        return it != groups_.end();
    }

private:
    std::unordered_map<std::string, User> users_;
    std::unordered_map<std::string, Group> groups_;
};

class ServerState {
public:
    ServerState(zmqw::socket& socket, DataBase& db)
          : socket_(socket), database_(db) {}

    zmqw::socket& GetSocket() {
        return socket_;
    }

    ServerCodes Register(const std::string& username,
                         const std::string& password) {
        if (database_.UserExists(username))
            return ServerCodes::USER_ALREADY_EXIST;
        database_.AddUser({username, password});
        return ServerCodes::SUCCESS;
    }

    ServerCodes Login(const NetIdentity& identity, const std::string& username,
                      const std::string& password) {
        // Check if an user is already connected with this identity
        if (IdentityConnected(identity))
            return ServerCodes::IDENTITY_ALREADY_CONNECTED;

        // Check if the user exist in the database
        if (!database_.UserExists(username))
            return ServerCodes::USER_DOES_NOT_EXIST;

        // Check if the user password match
        User* user = &database_.GetUser(username);
        if (!user->IsPassword(password))
            return ServerCodes::USER_WRONG_PASSWORD;

        // Add the identity to the ServerState set
        identities_.insert(identity);

        if (UserConnected(username)) {
            // Add the new user identity user
            users_[username].identities.push_back(identity);
        } else {
            // Add the user to the server and register its identity
            users_[username] = {user, UUID::UUID4().AsString(), {identity}};
        }

        return ServerCodes::SUCCESS;
    }

    ServerCodes Logout(const NetIdentity& identity,
                       const std::string& username) {
        // Check if the identity is not connected
        if (!IdentityConnected(identity))
            return ServerCodes::IDENTITY_ALREADY_CONNECTED;

        // Check is the user is not connected
        if (!UserConnected(username)) return ServerCodes::USER_NOT_CONNECTED;

        // Check if the user own the identity
        std::vector<NetIdentity>& identities = users_[username].identities;
        auto it = std::find(identities.begin(), identities.end(), identity);
        if (it == identities.end()) return ServerCodes::USER_INCORRECT_IDENTITY;

        identities.erase(it);         // Remove from the user identities
        identities_.erase(identity);  // Remove from the server identities

        // If identities left in the server remove the user
        if (identities.empty() && identities_.count(identity) == 0) {
            users_.erase(username);
        }

        return ServerCodes::SUCCESS;
    }

    ServerCodes AddContact(const std::string& username,
                           const std::string& token,
                           const std::string& contact) {
        // Check if the user exist in the database
        if (!database_.UserExists(username) || !database_.UserExists(contact))
            return ServerCodes::USER_DOES_NOT_EXIST;

        // Check if the user or contact are not conected
        if (!UserConnected(username)) return ServerCodes::USER_NOT_CONNECTED;

        if (GetToken(username) != token)
            return ServerCodes::USER_INCORRECT_TOKEN;

        User& user = GetUser(username);
        user.AddContact(contact);

        return ServerCodes::SUCCESS;
    }

    ServerCodes Whisper(const std::string& username, const std::string& token,
                        const std::string& recipient,
                        const std::string& content) {
        if (!UserConnected(username) || !UserConnected(recipient))
            return ServerCodes::USER_NOT_CONNECTED;

        if (GetToken(username) != token)
            return ServerCodes::USER_INCORRECT_TOKEN;

        Serializer update;
        update << "update"
               << "whisper" << username << content;
        for (auto& identities : GetIdentities(recipient)) {
            socket_.send(identities, ZMQ_SNDMORE);
            socket_.send(update);
        }

        return ServerCodes::SUCCESS;
    }

    ServerCodes CreateGroup(const std::string& username,
                            const std::string& token,
                            const std::string& group_name) {
        if (!UserConnected(username)) return ServerCodes::USER_NOT_CONNECTED;

        if (GetToken(username) != token)
            return ServerCodes::USER_INCORRECT_TOKEN;

        if (database_.GroupExists(group_name)) return ServerCodes::GROUP_ALREADY_EXIST;

        database_.AddGroup({group_name, username});

        return ServerCodes::SUCCESS;
    }

    bool UserConnected(const std::string& username) const {
        auto it = users_.find(username);
        return (it != users_.end());
    }

    bool IdentityConnected(const NetIdentity& identity) {
        auto it = identities_.find(identity);
        return (it != identities_.end());
    }

    User& GetUser(const std::string& username) {
        return *users_.find(username)->second.user;
    }

    const std::string& GetToken(const std::string& username) {
        return users_.find(username)->second.token;
    }

    const std::vector<NetIdentity>& GetIdentities(
        const std::string& username) const {
        return users_.find(username)->second.identities;
    }

private:
    struct UserConnection {
        User* user;
        std::string token;
        std::vector<NetIdentity> identities;
    };

private:
    // Server socket
    zmqw::socket& socket_;

    // In-Memory storage
    DataBase& database_;

    // Server state
    std::unordered_set<NetIdentity> identities_;
    std::unordered_map<std::string, UserConnection> users_;
    std::unordered_map<std::string, Group*> groups_;
};

void Login(ServerState& server, const NetIdentity& identity,
           Deserializer& request, Serializer& response) {
    std::string username, password;
    request >> username >> password;
    ServerCodes result = server.Login(identity, username, password);
    if (result == ServerCodes::SUCCESS) {
        std::cout << "User '" << username << "' joins the chat server\n";
        response << true << username << server.GetToken(username);
    } else {
        std::string error_message;
        if (result == ServerCodes::IDENTITY_ALREADY_CONNECTED) {
            error_message = "An user is already connected from this session.";
        } else {
            error_message = "Error with username or password.";
        }
        response << false << error_message;
    }
}

void AddContact(ServerState& server, Deserializer& request,
                Serializer& response) {
    std::string username, token, contact;
    request >> username >> token >> contact;
    ServerCodes result = server.AddContact(username, token, contact);
    if (result == ServerCodes::SUCCESS) {
        std::cout << "User '" << username << "' added '" << contact << "'\n";
        response << true;
    } else {
        std::string error_message = "Could not add user.";
        response << false << error_message;
    }
}

void Logout(ServerState& server, const NetIdentity& identity,
            Deserializer& request, Serializer& response) {
    std::string username;
    request >> username;
    ServerCodes result = server.Logout(identity, username);
    if (result == ServerCodes::SUCCESS) {
        std::cout << "User '" << username << "' disconected.\n";
        response << true;
    } else {
        std::string error_message = "Could not logout, please login first.";
        response << false << error_message;
    }
}

void Register(ServerState& server, Deserializer& request,
              Serializer& response) {
    std::string username, password;
    request >> username >> password;
    if (server.Register(username, password) == ServerCodes::SUCCESS) {
        std::cout << "User '" << username << "' just registered\n";
        response << true;
    } else {
        std::string error_message = "Error registering: username '" + username +
                                    "' already in the database";
        response << false << error_message;
    };
}

void Whisper(ServerState& server, Deserializer& request, Serializer& response) {
    std::string username, token, recipient, content;
    request >> username >> token >> recipient >> content;
    ServerCodes result = server.Whisper(username, token, recipient, content);
    if (result == ServerCodes::SUCCESS) {
        response << true;
    } else {
        std::string error_message = "Message not sent.";
        response << false << error_message;
    }
}

void CreateGroup(ServerState& server, Deserializer& request, Serializer& response) {
    std::string username, token, group_name;
    request >> username >> token >> group_name;
    ServerCodes result = server.CreateGroup(username, token, group_name);
    if (result == ServerCodes::SUCCESS) {
        std::cout << "Group '" << group_name << "' created, owner '" << username << "'\n";
        response << true;
    } else {
        std::string error_message = "Could not create group.";
        response << false << error_message;
    }
}

void Dispatch(ServerState& server) {
    std::string identity;
    Deserializer request;
    server.GetSocket().recv(identity);
    server.GetSocket().recv(request);
    if (gSignalStatus) return;

    std::string action;
    request >> action;

    Serializer response;
    response << "response";

    if (action == "register") {
        Register(server, request, response);
    } else if (action == "login") {
        Login(server, identity, request, response);
    } else if (action == "logout") {
        Logout(server, identity, request, response);
    } else if (action == "add_contact") {
        AddContact(server, request, response);
    } else if (action == "whisper") {  // Unicast chat
        Whisper(server, request, response);
    } else if (action == "create_group") {  // Unicast chat
        CreateGroup(server, request, response);
    } else {
        std::cerr << "Action not supported/implemented\n";
    }

    server.GetSocket().send(identity, ZMQ_SNDMORE);
    server.GetSocket().send(response);
}

int main(/*int argc, char* argv[]*/) {
    zmq::context_t context(1);

    // Create the listening
    zmqw::socket socket(context, ZMQ_ROUTER);
    socket.bind("tcp://*:4242");

    std::signal(SIGINT, gSignalHandler);
    std::signal(SIGTERM, gSignalHandler);

    // Create the in memory database
    DataBase database;

    // Create and initialize the ServerState\n;
    ServerState state(socket, database);
    state.Register("edoren", "123");
    state.Register("pepe", "123");

    while (true) {
        try {
            Dispatch(state);
        } catch (zmq::error_t& e) {
        }
        if (gSignalStatus) {
            std::cout << "\nInterrupt signal received, killing server...\n";
            break;
        }
    }

    std::cout << "Server closed.\n";
    return gSignalStatus;
}
