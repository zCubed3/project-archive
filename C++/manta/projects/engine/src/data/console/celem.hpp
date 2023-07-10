#ifndef MANTA_CELEM_HPP
#define MANTA_CELEM_HPP

#include <vector>
#include <string>

namespace Manta {
    class EngineContext;
}

namespace Manta::Console {
    class CElem {
    public:
        ~CElem();

        virtual void Execute(EngineContext *context, const std::vector<std::string> &args) = 0;
    };
}

#endif
