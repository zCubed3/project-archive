#ifndef SAPPHIRE_CONSOLE_H
#define SAPPHIRE_CONSOLE_H

#include <string>
#include <unordered_map>
#include <functional>

#include <engine/console/console_stream.h>

class Console {
public:
    using CFuncCallback = std::function<void(const std::vector<std::string>&)>;
    using CVarSetCallback = std::function<void(const std::string&)>;

    // There are two types of console entry
    // CVar and CFunc
    struct ConsoleEntry {
        std::string name;
        std::string help;

        // If function is nullptr we instead just set the local variable
        CFuncCallback function = nullptr;

        CVarSetCallback set_callback = nullptr;
        std::string value;
    };

protected:
    Console() = delete;

public:
    static std::unordered_map<std::string, ConsoleEntry> entries;
    static ConsoleStream console_cout;

    static void add_cfunc(const std::string& name, CFuncCallback callback, const std::string& help = "");
    static void add_cvar(const std::string& name, const std::string& value, const std::string& help = "");

    static void execute(const std::vector<std::string>& args);

    static void register_defaults();

    static void log(const std::string& message, ConsoleStream::MessageSeverity severity = ConsoleStream::MESSAGE_SEVERITY_NONE);
    static void log_warning(const std::string& message);
    static void log_error(const std::string& message);
};

#endif
