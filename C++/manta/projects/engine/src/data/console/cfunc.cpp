#include "cfunc.hpp"

#include <stdexcept>

namespace Manta::Console {
    CFunc::CFunc(cfunc_executor executor) {
        this->executor = executor;
    }

    void CFunc::Execute(EngineContext *context, const std::vector<std::string> &args) {
        if (executor == nullptr)
            throw std::logic_error("Executor fptr was a nullptr!");

        executor(context, args);
    }
}