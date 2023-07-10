#include "stdelems.hpp"

#include <vector>
#include <string>
#include <iostream>

#include "console.hpp"
#include "cfunc.hpp"

#include <data/engine_context.hpp>

namespace Manta::Console {
    bool echo_cmd_func(EngineContext* context, const std::vector<std::string>& args) {
        for (const auto& arg : args)
            std::cout << arg << std::endl;

        return true;
    }

    void RegisterCommonCElems(Console* console) {
        console->RegisterElement("echo", new CFunc(echo_cmd_func));
    }
}