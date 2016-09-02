#include <csignal>
#include <cstdio>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <Util/Serializer.hpp>
#include <Util/ZMQWrapper.hpp>

static volatile std::sig_atomic_t gSignalStatus = 0;

static void gSignalHandler(int signal_value) {
    gSignalStatus = signal_value;
}

std::string TrimSpaces(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

class ChatClient {
public:
    ChatClient(const std::string& endpoint)
          : context_(1), socket_(context_, ZMQ_DEALER) {
        // Connect to the server
        socket_.connect(endpoint);

        is_running_ = true;
        listener_ = std::thread(&ChatClient::ResponseListener, this);
    }

    ~ChatClient() {
        Logout();
        is_running_ = false;
        listener_.join();
        socket_.close();
    }

    bool Register(const std::string& username, const std::string password) {
        if (username.empty() || password.empty()) return false;
        Serializer request;
        request << "register" << username << password;
        if (socket_.send(request)) last_action_ = "register";
        return true;
    }

    bool Login(const std::string& username, const std::string password) {
        if (!username_.empty() || username.empty() || password.empty())
            return false;
        Serializer request;
        request << "login" << username << password;
        if (socket_.send(request)) last_action_ = "login";
        return true;
    }

    bool AddContact(const std::string& contact) {
        if (username_.empty() || contact.empty()) return false;
        Serializer request;
        request << "add_contact" << username_ << token_ << contact;
        if (socket_.send(request)) last_action_ = "add_contact";
        return true;
    }

    bool Logout() {
        if (username_.empty()) return false;
        Serializer request;
        request << "logout" << username_;
        if (socket_.send(request)) last_action_ = "logout";
        return true;
    }

    bool Whisper(const std::string& recipient, const std::string content) {
        std::string tcontent = TrimSpaces(content);
        if (username_.empty() || recipient.empty() || tcontent.empty())
            return false;
        Serializer request;
        request << "whisper" << username_ << token_ << recipient << tcontent;
        if (socket_.send(request)) last_action_ = "whisper";
        return true;
    }

    bool CreateGroup(const std::string& group_name) {
        if (username_.empty()) return false;
        Serializer request;
        request << "create_group" << username_ << token_ << group_name;
        if (socket_.send(request)) last_action_ = "create_group";
        return true;
    }

private:
    void ResponseListener() {
        Deserializer server_msg;
        while (is_running_) {
            if (!socket_.recv(server_msg, ZMQ_NOBLOCK)) continue;
            std::string type;
            server_msg >> type;
            if (type == "response") {
                HandleResponse(server_msg);
            } else if (type == "update") {
                HandleUpdate(server_msg);
            } else {
                // Ignore the message
            }
        }
    }

    void HandleResponse(Deserializer& response) {
        bool status;
        response >> status;

        if (!status) {
            std::string error;
            response >> error;
            if (!error.empty()) std::cout << error << "\n";
            return;
        }

        if (last_action_ == "login") {
            response >> username_;
            response >> token_;
            std::cout << "User " << username_ << " successfully logged in.\n";
            std::cout << "Token: " << token_ << "\n";
        } else if (last_action_ == "register") {
            std::cout << "Register successful.\n";
        } else if (last_action_ == "logout") {
            std::cout << "Logout successful.\n";
            username_.clear();
            token_.clear();
        } else if (last_action_ == "whisper") {
            // ???
        } else if (last_action_ == "create_group") {
            std::cout << "Group creation successful.\n";
        }

        last_action_.clear();
    }

    void HandleUpdate(Deserializer& response) {
        std::string type;
        response >> type;

        if (type == "whisper") {
            std::string sender, content;
            response >> sender >> content;
            std::cout << "whisper from " << sender << ": " << content << "\n";
        }
    }

private:
    bool is_running_;

    zmq::context_t context_;  // ZMQ context
    zmqw::socket socket_;     // Client socket

    std::string username_;     // The username of the currently logged user
    std::string token_;        // The user request token
    std::string last_action_;  // The last request performed by the user

    // TODO: Action queue

    std::vector<std::string> messages_;  // Messages received from other users

    std::thread listener_;  // The listener of responses and updates
};

bool HandleCommands(ChatClient& client, const std::string& line) {
    std::stringstream stream(line);
    std::string action;
    stream >> action;
    if (action == "/exit") {
        client.Logout();
        return false;
    } else if (action == "/login") {
        std::string username, password;
        stream >> username >> password;
        client.Login(username, password);
    } else if (action == "/add") {
        std::string contact;
        stream >> contact;
        client.AddContact(contact);
    } else if (action == "/logout") {
        client.Logout();
    } else if (action == "/register") {
        std::string username, password;
        stream >> username >> password;
        client.Register(username, password);
    } else if (action == "/msg" || action == "/w") {
        std::string receiver, content;
        stream >> receiver;
        std::getline(stream, content);
        client.Whisper(receiver, content);
    } else if (action == "/create_group") {
        std::string group_name;
        stream >> group_name;
        client.CreateGroup(group_name);
    } else {
        std::cout << "Action not supported or implemented.\n";
    }

    return true;
}

#ifdef _WIN32
#include <windows.h>
#define fileno _fileno
#endif

int main(/*int argc, char* argv[]*/) {
    // The state of the client aplication
    std::string endpoint("tcp://localhost:4242");
    ChatClient client(endpoint);
    std::cout << "Connecting to " << endpoint << '\n';

    std::signal(SIGINT, gSignalHandler);
    std::signal(SIGTERM, gSignalHandler);

    zmq::pollitem_t items[] = {{nullptr, fileno(stdin), ZMQ_POLLIN, 0}};

    while (true) {
        try {
            zmq::poll(items, 1, -1);
        } catch (zmq::error_t& e) {
        }
        if (gSignalStatus != 0) {
            std::cout << "\nInterrupt signal received, killing client...\n";
            break;
        }
        // Wait for the login response
        if (items[0].revents & ZMQ_POLLIN) {
            std::string line;
            std::getline(std::cin, line);

            if (line.empty()) continue;

            if (line[0] == '/') {
                if (!HandleCommands(client, line)) break;
            } else {
                std::cout << line << '\n';
            }
        }
    }

    std::cout << "Client closed.\n";

    return 0;
}
