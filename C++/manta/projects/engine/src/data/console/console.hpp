#ifndef MANTA_CONSOLE_HPP
#define MANTA_CONSOLE_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include "celem.hpp"

namespace Manta::Console {
    class Console {
    public:
        void RegisterElement(const std::string& key, CElem* elem);
        void DoCommandLine(const std::vector<std::string>& argv);
        void DoString(const std::string& cmd);

        void TryExecute(EngineContext* context, const std::string& key, const std::vector<std::string>& args);

    protected:
        std::unordered_map<std::string, CElem*> elements;
    };
}

#endif
