#include "console.h"

//
// Functions
//
void help_command(const std::vector<std::string>& args) {
    if (args.empty()) {
        Console::log("Provide the name of a cvar or cfunc for help! Ex: 'help quit'");
    } else {
        auto iter = Console::entries.find(args[0]);

        if (iter != Console::entries.end()) {
            Console::log(iter->second.help);
        }
    }
}

//
// Console implementation
//
std::unordered_map<std::string, Console::ConsoleEntry> Console::entries = {};
ConsoleStream Console::console_cout = {};

void Console::add_cfunc(const std::string &name, CFuncCallback callback, const std::string& help) {
#ifdef DEBUG
    log("Registered CFunc: " + name);
#endif

    ConsoleEntry entry {};

    entry.name = name;
    entry.help = help;
    entry.function = callback;

    entries.emplace(name, entry);
}

void Console::add_cvar(const std::string &name, const std::string &value, const std::string& help) {
#ifdef DEBUG
    log("Registered CVar: " + name);
#endif

    ConsoleEntry entry {};

    entry.name = name;
    entry.help = help;
    entry.value = value;

    entries.emplace(name, entry);
}

void Console::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        log_warning("Args vector was empty!");
    }

    auto iter = entries.find(args[0]);

    if (iter != entries.end()) {
        if (iter->second.function != nullptr) {
            std::vector<std::string> inputs = args;
            inputs.erase(inputs.begin());

            iter->second.function(inputs);
        } else {
            if (args.size() >= 2) {
                iter->second.value = args[1];
            } else {
                iter->second.value = "";
            }

            if (iter->second.set_callback != nullptr) {
                iter->second.set_callback(iter->second.value);
            }
        }
    } else {
        log_warning("No entry found for '" + args[0] + "'!");
    }
}

void Console::register_defaults() {
    add_cfunc("help", help_command, "Provides info for the given command");
}

void Console::log(const std::string &message, ConsoleStream::MessageSeverity severity) {
    console_cout.log_message(message, severity);
}

void Console::log_warning(const std::string &message) {
    log(message, ConsoleStream::MESSAGE_SEVERITY_WARNING);
}

void Console::log_error(const std::string &message) {
    log(message, ConsoleStream::MESSAGE_SEVERITY_ERROR);
}