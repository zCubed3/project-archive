#ifndef SAPPHIRE_CONSOLE_STREAM_H
#define SAPPHIRE_CONSOLE_STREAM_H

#include <fstream>
#include <iostream>
#include <deque>

#include <core/platforms/platform.h>

class ConsoleStream {
public:
    enum MessageSeverity {
        MESSAGE_SEVERITY_NONE,
        MESSAGE_SEVERITY_WARNING,
        MESSAGE_SEVERITY_ERROR
    };

    struct ConsoleMessage {
        MessageSeverity severity;
        std::string message;
    };

    // TODO: Queue messages up instead
    std::deque<ConsoleMessage> message_queue;

protected:
    std::ofstream log_file = std::ofstream("engine.log");

public:
    ConsoleStream& log_message(const std::string& message, MessageSeverity severity) {
        ConsoleMessage msg {};
        msg.message = message;
        msg.severity = severity;

        // Forward to immediate logging
        write_severity(message, severity);
        endl();

        message_queue.push_back(msg);

        return *this;
    }

    template<class T>
    ConsoleStream& write(T val) {
        log_file << val;
        std::cout << val;

        return *this;
    }

    template<class T>
    ConsoleStream& write_severity(T val, MessageSeverity severity) {
        switch (severity) {
            default:
                break;

            case MESSAGE_SEVERITY_WARNING:
                Platform::get_singleton()->set_console_color(Platform::CONSOLE_COLOR_YELLOW);

                log_file << "[WARNING]: ";
                std::cout << "[WARNING]: ";
                break;
        }

        log_file << val;
        std::cout << val;

        Platform::get_singleton()->set_console_color(Platform::CONSOLE_COLOR_WHITE);

        return *this;
    }

    ConsoleStream& endl() {
        log_file << std::endl;
        std::cout << std::endl;

        return *this;
    }

    void clear() {
        message_queue.clear();
    }
};


#endif
