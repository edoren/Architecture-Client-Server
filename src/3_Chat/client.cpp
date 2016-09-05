#include <csignal>
#include <cstdio>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <deque>
#include <sstream>
#include <string>
#include <thread>

#include <SFML/Audio.hpp>

#include <Util/Serializer.hpp>
#include <Util/ZMQWrapper.hpp>

#include "ServerCodes.hpp"

static volatile std::sig_atomic_t gSignalStatus = 0;

static void gSignalHandler(int signal_value) {
    gSignalStatus = signal_value;
}

std::string TrimSpaces(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    if (first == std::string::npos && last == std::string::npos)
        return std::string();
    return str.substr(first, (last - first + 1));
}

class ChatClient {
public:
    ChatClient(const std::string& ip, size_t port)
          : context_(1), socket_(context_, ZMQ_DEALER) {
        // Connect to the server
        socket_.connect("tcp://" + ip + ':' + std::to_string(port));

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
        socket_.send(request);
        return true;
    }

    bool Login(const std::string& username, const std::string password) {
        if (!username_.empty() || username.empty() || password.empty())
            return false;
        Serializer request;
        request << "login" << username << password;
        socket_.send(request);
        return true;
    }

    bool Logout() {
        if (username_.empty()) return false;
        Serializer request;
        request << "logout" << username_;
        socket_.send(request);
        return true;
    }

    bool AddContact(const std::string& contact) {
        if (username_.empty() || contact.empty()) return false;
        Serializer request;
        request << "add_contact" << username_ << token_ << contact;
        socket_.send(request);
        return true;
    }

    bool Whisper(const std::string& recipient, const std::string content) {
        std::string tcontent = TrimSpaces(content);
        if (username_.empty() || recipient.empty() || tcontent.empty())
            return false;
        Serializer request;
        request << "whisper" << username_ << token_ << recipient << tcontent;
        socket_.send(request);
        return true;
    }

    bool CreateGroup(const std::string& group_name) {
        if (username_.empty() || group_name.empty()) return false;
        Serializer request;
        request << "create_group" << username_ << token_ << group_name;
        socket_.send(request);
        return true;
    }

    bool JoinGroup(const std::string& group_name) {
        if (username_.empty()) return false;
        Serializer request;
        request << "join_group" << username_ << token_ << group_name;
        socket_.send(request);
        return true;
    }

    bool MessageGroup(const std::string& group_name,
                      const std::string content) {
        std::string tcontent = TrimSpaces(content);
        if (username_.empty() || group_name.empty() || tcontent.empty())
            return false;
        Serializer request;
        request << "msg_group" << username_ << token_ << group_name << tcontent;
        socket_.send(request);
        return true;
    }

    bool SendVoiceMessage(const std::string& recipient, size_t channels,
                          size_t sample_rate,
                          const std::vector<int16_t>& samples) {
        if (username_.empty() || recipient.empty()) return false;

        Serializer request;
        request << "voice_msg" << username_ << token_ << recipient << channels
                << sample_rate << samples;
        socket_.send(request);
        return true;
    }

    bool SendCallData(const std::string& recipient,
                      const std::vector<int16_t>& samples) {
        if (username_.empty() || recipient.empty()) return false;

        Serializer request;
        request << "call" << username_ << token_ << recipient << samples;
        socket_.send(request);
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

    std::string username_;  // The username of the currently logged user
    std::string token_;     // The user request token

    std::thread listener_;  // The listener of responses and updates
};

static std::string HELP(R"(Usage:
    /register [username] [password]      Register a new user in the chat
    /login [username] [password]         Login user to the chat
    /logout                              Logout the currently logged user
    /msg [recipient] [content]           Send a text message to another user
    /create_group [group_name]           Create a new group
    /join_group [group_name]             Join an existent group
    /msg_group [group_name] [content]    Send a text message to a group
    /record [recipient]                  Record and send a voice message to a user
    /play                                Play the last received voice message
    /call [recipient]                    Call a user
)");

class RecorderLOL : public sf::SoundRecorder {
public:
    RecorderLOL(std::mutex& queue_mutex,
                std::deque<std::vector<int16_t>>& samples_queue)
          : queue_mutex_(queue_mutex), samples_queue_(samples_queue) {}

    ~RecorderLOL() {
        stop();
    }

private:
    virtual bool onStart() {
        return true;
    }

    virtual bool onProcessSamples(const int16_t* samples,
                                  std::size_t sample_count) {
        if (sample_count) {
            std::lock_guard<std::mutex> lk(queue_mutex_);
            samples_queue_.emplace_back(samples, samples + sample_count);
        }
        return true;
    }

    virtual void onStop() {}

private:
    std::mutex& queue_mutex_;
    std::deque<std::vector<int16_t>>& samples_queue_;
};

class PlayerLOL : public sf::SoundStream {
public:
    PlayerLOL(std::mutex& queue_mutex,
              std::deque<std::vector<int16_t>>& samples_queue)
          :
            queue_mutex_(queue_mutex),
            samples_queue_(samples_queue) {
        initialize(1, 44100);
    }

private:
    virtual bool onGetData(sf::SoundStream::Chunk& data) {
        if (getStatus() != sf::SoundStream::Playing) return false;

        if (!samples_queue_.empty()) {
            std::lock_guard<std::mutex> lk(queue_mutex_);
            samples_queue_.pop_front();
        }

        while (samples_queue_.empty() &&
               getStatus() == sf::SoundStream::Playing) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (!samples_queue_.empty()) {
            data.samples = &samples_queue_.front()[0];
            data.sampleCount = samples_queue_.front().size();
        }

        return true;
    }

    virtual void onSeek(sf::Time /*timeOffset*/) {}

private:
    std::mutex& queue_mutex_;
    std::deque<std::vector<int16_t>>& samples_queue_;
};

class ChatCLI : public ChatClient {
public:
    ChatCLI(const std::string& ip, size_t port)
          : ChatClient(ip, port),
            responses_arrived_(0),
            outgoing_call_(false) {}

    ~ChatCLI() {
        outgoing_call_ = false;
    }

public:
    bool HandleCommands(const std::string& line) {
        std::stringstream stream(line);
        std::string action;
        stream >> action;

        bool request_sent = false;

        if (action == "/exit") {
            Logout();
            return false;
        } else if (action == "/help") {
            std::cout << HELP;
        } else if (action == "/register") {
            std::string username, password;
            stream >> username >> password;
            request_sent = Register(username, password);
        } else if (action == "/login") {
            std::string username, password;
            stream >> username >> password;
            request_sent = Login(username, password);
        } else if (action == "/logout") {
            request_sent = Logout();
        } else if (action == "/add") {
            std::string contact;
            stream >> contact;
            request_sent = AddContact(contact);
        } else if (action == "/msg" || action == "/w") {
            std::string recipient, content;
            stream >> recipient;
            std::getline(stream, content);
            request_sent = Whisper(recipient, content);
        } else if (action == "/create_group") {
            std::string group_name;
            stream >> group_name;
            request_sent = CreateGroup(group_name);
        } else if (action == "/join_group") {
            std::string group_name;
            stream >> group_name;
            request_sent = JoinGroup(group_name);
        } else if (action == "/msg_group") {
            std::string group_name, content;
            stream >> group_name;
            std::getline(stream, content);
            request_sent = MessageGroup(group_name, content);
        } else if (action == "/record") {
            std::string recipient;
            stream >> recipient;
            request_sent = RecordAndSend(recipient);
        } else if (action == "/play") {
            sf::Sound sound(last_voice_msg_);
            sound.play();
            std::cout << "Playing... ";
            while (sound.getStatus() == sf::Sound::Playing) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            std::cout << "Done.\n";
        } else if (action == "/call") {
            std::string recipient;
            stream >> recipient;
            call_samples_mutex_.lock();
            call_samples_.clear();
            call_samples_mutex_.unlock();
            std::thread thread(&ChatCLI::DoCall, this, recipient);
            std::cout << "Calling... press enter to stop";
            std::cin.ignore(10000, '\n');
            outgoing_call_ = false;
            thread.join();
        } else {
            std::cout << "Action not supported or implemented.\n";
        }

        // Wait for response
        if (request_sent == true) {
            std::unique_lock<std::mutex> lk(cv_mutex_);
            cv_.wait(lk, [&] { return responses_arrived_ > 0; });
            responses_arrived_--;
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

        return true;
    }

    void DoCall(const std::string& other) {
        if (!sf::SoundRecorder::isAvailable()) {
            std::cerr
                << "Sorry, audio capture is not supported by your system\n";
            return;
        }

        std::mutex queue_mutex;
        std::deque<std::vector<int16_t>> outgoing_;

        RecorderLOL recorder(queue_mutex, outgoing_);
        PlayerLOL player(call_samples_mutex_, call_samples_);

        recorder.start(44100);
        player.play();

        outgoing_call_ = true;
        while (outgoing_call_) {
            if (!outgoing_.empty()) {
                std::lock_guard<std::mutex> lk(queue_mutex);
                if (!SendCallData(other, outgoing_.front())) {
                    std::cout << "Please login first.\n";
                    break;
                }
                outgoing_.pop_front();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        player.stop();
        recorder.stop();
    }

    void HandleResponse(Deserializer& response) {
        std::string action;
        ServerCodes status;
        response >> action >> status;

        if (status != ServerCodes::SUCCESS) {
            std::cout << "Command failed with error code "
                      << static_cast<int>(status) << ": " << status << '\n';

            if (action == "call") {
                outgoing_call_ = false;
            }

            return;
        }

        if (action == "login") {
            response >> username_;
            response >> token_;
            std::cout << "User " << username_ << " successfully logged in.\n";
            std::cout << "Token: " << token_ << '\n';
        } else if (action == "register") {
            std::cout << "Register successful.\n";
        } else if (action == "logout") {
            std::cout << "Logout successful.\n";
            username_.clear();
            token_.clear();
        } else if (action == "whisper") {
            // ???
        } else if (action == "create_group") {
            std::cout << "Group creation successful.\n";
        } else if (action == "join_group") {
            std::cout << "Group join successful.\n";
        } else if (action == "voice_msg") {
            std::cout << "Voice message sent.\n";
        } else if (action == "call") {
        }

        if (action != "call") responses_arrived_++;
        cv_.notify_one();
    }

    void HandleUpdate(Deserializer& response) {
        std::string type;
        response >> type;

        if (type == "whisper") {
            std::string sender, content;
            response >> sender >> content;
            if (sender == username_)
                return;  // Ignore the message if is sent by the user
            std::cout << "[whisper] " << sender << ": " << content << '\n';
        } else if (type == "msg_group") {
            std::string group_name, sender, content;
            response >> group_name >> sender >> content;
            if (sender == username_)
                return;  // Ignore the message if is sent by the user
            std::cout << "[" << group_name << "] " << sender << ": " << content
                      << '\n';
        } else if (type == "voice_msg") {
            std::string sender;
            size_t channels, sample_rate;
            std::vector<int16_t> samples;
            response >> sender >> channels >> sample_rate >> samples;
            last_voice_msg_.loadFromSamples(samples.data(), samples.size(),
                                            channels, sample_rate);
            std::cout << "[ALERT] " << sender
                      << " sent you a voice message, /play to listen it."
                      << '\n';
        } else if (outgoing_call_ && type == "call") {
            std::string sender;
            std::vector<int16_t> samples;
            response >> sender >> samples;
            call_samples_mutex_.lock();
            call_samples_.push_back(std::move(samples));
            call_samples_mutex_.unlock();
        }
    }

private:
    int responses_arrived_;

    std::mutex cv_mutex_;
    std::condition_variable cv_;

    sf::SoundBuffer last_voice_msg_;

    bool outgoing_call_;
    std::mutex call_samples_mutex_;
    std::deque<std::vector<int16_t>> call_samples_;  // Hold the call samples
};

#ifdef _WIN32
#include <windows.h>
#define fileno _fileno
#endif

int main(/*int argc, char* argv[]*/) {
    // The state of the client aplication
    std::string ip = "localhost";
    size_t port = 4242;
    ChatCLI client(ip, port);
    std::cout << "Connecting to chat in " << ip << ":" << port << '\n';
    std::cout << "Use /help for more information\n";

    std::signal(SIGINT, gSignalHandler);
    std::signal(SIGTERM, gSignalHandler);

    zmq::pollitem_t items[] = {{nullptr, fileno(stdin), ZMQ_POLLIN, 0}};

    while (true) {
        std::cout << "> " << std::flush;
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
