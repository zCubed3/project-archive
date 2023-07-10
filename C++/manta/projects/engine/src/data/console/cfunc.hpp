#ifndef MANTA_CFUNC_HPP
#define MANTA_CFUNC_HPP

#include "console.hpp"

namespace Manta::Console {
    // Binds a static function to something the console can invoke!
    class CFunc : public CElem {
    public:
        typedef bool(*cfunc_executor)(EngineContext*, const std::vector<std::string>&);

        CFunc() = delete; // Prevent regular construction
        CFunc(cfunc_executor executor);
        void Execute(EngineContext *context, const std::vector<std::string> &args) override;

        cfunc_executor executor = nullptr;
    };
}

#endif
