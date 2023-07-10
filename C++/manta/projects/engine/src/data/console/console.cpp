#include "console.hpp"

#include <stdexcept>
#include <iostream>

namespace Manta::Console {
    void Console::RegisterElement(const std::string &key, CElem *elem) {
        if (key.empty())
            throw std::logic_error("Key was empty!");

        if (elem == nullptr)
            throw std::logic_error("Elem was a nullptr!");

        if (elements.find(key) != elements.end())
            throw std::logic_error("Key was already found in the element map!");

#ifdef DEBUG
        std::cout << "Registered CElem '" << key << "'" << std::endl;
#endif

        elements.emplace(key, elem);
    }

    void Console::DoCommandLine(const std::vector<std::string>& argv) {
        // Parsing the command line is similar to a file except it's a linear array rather than line based!
        // We first identify if the given argument sets a CVar or invokes a CFunc
        // A CElem is invoked using '+'!

        bool finding_args = false;
        std::vector<std::string> arguments;
        std::string key;
        for (auto arg : argv) {
            if (arg.at(0) == '+') {
                if (finding_args) {
                    TryExecute(nullptr, key, arguments);
                    arguments.clear();
                }

                finding_args = true;
                key = arg.substr(1);
            } else {
                if (finding_args)
                    arguments.emplace_back(arg);
            }
        }

        if (finding_args) {
            TryExecute(nullptr, key, arguments);
        }
    }

    void Console::DoString(const std::string &cmd) {

    }

    void Console::TryExecute(EngineContext* context, const std::string &key, const std::vector<std::string>& args) {
        auto elem = elements.find(key);

        if (elem == elements.end())
            throw std::logic_error("The given key wasn't present in the element map!");

        elem->second->Execute(context, args);
    }
}