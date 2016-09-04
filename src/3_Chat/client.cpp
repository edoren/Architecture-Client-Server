#include <csignal>
#include <cstdio>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <SFML/Audio.hpp>

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
        if (username_.empty() || group_name.empty()) return false;
        Serializer request;
        request << "create_group" << username_ << token_ << group_name;
        if (socket_.send(request)) last_action_ = "create_group";
        return true;
    }

    bool JoinGroup(const std::string& group_name) {
        if (username_.empty()) return false;
        Serializer request;
        request << "join_group" << username_ << token_ << group_name;
        if (socket_.send(request)) last_action_ = "join_group";
        return true;
    }

    bool MessageGroup(const std::string& group_name,
                      const std::string content) {
        std::string tcontent = TrimSpaces(content);
        if (username_.empty() || group_name.empty() || tcontent.empty())
            return false;
        Serializer request;
        request << "msg_group" << username_ << token_ << group_name << tcontent;
        if (socket_.send(request)) last_action_ = "msg_group";
        return true;
    }

    bool SendVoiceMessage(const std::string& recipient, size_t channels,
                          size_t sample_rate,
                          const std::vector<int16_t>& samples) {
        if (recipient.empty()) return false;

        Serializer request;
        request << "voice_msg" << username_ << token_ << recipient << channels
                << sample_rate << samples;
        if (socket_.send(request)) last_action_ = "voice_msg";
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

protected:
    virtual void HandleResponse(Deserializer& /*response*/) = 0;

    virtual void HandleUpdate(Deserializer& /*update*/) = 0;

protected:
    bool is_running_;

    zmq::context_t context_;  // ZMQ context
    zmqw::socket socket_;     // Client socket

    std::string username_;     // The username of the currently logged user
    std::string token_;        // The user request token
    std::string last_action_;  // The last request performed by the user

    std::thread listener_;  // The listener of responses and updates
};

class ChatCLI : public ChatClient {
public:
    using ChatClient::ChatClient;  // Inherite contructors

public:
    bool HandleCommands(const std::string& line) {
        std::stringstream stream(line);
        std::string action;
        stream >> action;
        if (action == "/exit") {
            Logout();
            return false;
        } else if (action == "/login") {
            std::string username, password;
            stream >> username >> password;
            Login(username, password);
        } else if (action == "/add") {
            std::string contact;
            stream >> contact;
            AddContact(contact);
        } else if (action == "/logout") {
            Logout();
        } else if (action == "/register") {
            std::string username, password;
            stream >> username >> password;
            Register(username, password);
        } else if (action == "/msg" || action == "/w") {
            std::string receiver, content;
            stream >> receiver;
            std::getline(stream, content);
            Whisper(receiver, content);
        } else if (action == "/create_group") {
            std::string group_name;
            stream >> group_name;
            CreateGroup(group_name);
        } else if (action == "/join_group") {
            std::string group_name;
            stream >> group_name;
            JoinGroup(group_name);
        } else if (action == "/msg_group") {
            std::string group_name, content;
            stream >> group_name;
            std::getline(stream, content);
            MessageGroup(group_name, content);
        } else if (action == "/record") {
            std::string recipient;
            stream >> recipient;
            RecordAndSend(recipient);
        } else if (action == "/play") {
            sf::Sound sound(last_voice_msg_);
            sound.play();
            std::cout << "Playing... ";
            while (sound.getStatus() == sf::Sound::Playing) {
                sf::sleep(sf::milliseconds(100));
            }
            std::cout << "Done.\n";
        } else {
            std::cout << "Action not supported or implemented.\n";
        }

        return true;
    }

private:
    bool RecordAndSend(const std::string& recipient) {
        if (!sf::SoundRecorder::isAvailable()) {
            std::cerr
                << "Sorry, audio capture is not supported by your system\n";
            return false;
        }

        if (recipient.empty()) {
            std::cerr << "Usage: /record [recipient]\n";
            return false;
        }

        unsigned int sample_rate = 44100;

        sf::SoundBufferRecorder recorder;

        recorder.start(sample_rate);
        std::cout << "Recording... press enter to stop";
        std::cin.ignore(10000, '\n');
        recorder.stop();

        const sf::SoundBuffer& buffer = recorder.getBuffer();

        std::vector<int16_t> samples(
            buffer.getSamples(), buffer.getSamples() + buffer.getSampleCount());

        SendVoiceMessage(recipient, buffer.getChannelCount(),
                         buffer.getSampleRate(), samples);

        std::cout << "Sent.";

        return true;
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
        } else if (last_action_ == "join_group") {
            std::cout << "Group join successful.\n";
        }

        last_action_.clear();
    }

    void HandleUpdate(Deserializer& response) {
        std::string type;
        response >> type;

        if (type == "whisper") {
            std::string sender, content;
            response >> sender >> content;
            if (sender == username_)
                return;  // Ignore the message if is sent by the user
            std::cout << "[whisper] " << sender << ": " << content << "\n";
        } else if (type == "msg_group") {
            std::string group_name, sender, content;
            response >> group_name >> sender >> content;
            if (sender == username_)
                return;  // Ignore the message if is sent by the user
            std::cout << "[" << group_name << "] " << sender << ": " << content
                      << "\n";
        } else if (type == "voice_msg") {
            std::string sender;
            size_t channels, sample_rate;
            std::vector<int16_t> samples;
            response >> sender >> channels >> sample_rate >> samples;
            last_voice_msg_.loadFromSamples(samples.data(), samples.size(),
                                            channels, sample_rate);
            std::cout << "[ALERT] " << sender
                      << " sent you a voice message, /play to listen it.\n";
        }
    }

private:
    sf::SoundBuffer last_voice_msg_;
};

#ifdef _WIN32
#include <windows.h>
#define fileno _fileno
#endif

int main(/*int argc, char* argv[]*/) {
    // The state of the client aplication
    std::string endpoint("tcp://localhost:4242");
    ChatCLI client(endpoint);
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
            std::cout << "\nInterrupt signal received, killing ..\n";
            break;
        }
        // Wait for the login response
        if (items[0].revents & ZMQ_POLLIN) {
            std::string line;
            std::getline(std::cin, line);

            if (line.empty()) continue;

            if (line[0] == '/') {
                if (!client.HandleCommands(line)) break;
            } else {
                std::cout << line << '\n';
            }
        }
    }

    std::cout << "Client closed.\n";

    return 0;
}
