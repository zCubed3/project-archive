#ifndef MANTA_DYNLIB_HPP
#define MANTA_DYNLIB_HPP

#include <string>

#include <defines.hpp>

#ifdef WIN32
#include <Windows.h>
#endif

// Wrapper for .so and .dll files!
namespace Manta {
    class DynLib {
    public:
        static DynLib* Open(const std::string& path);

        void* GetFunction(const std::string& signature);

        template<typename tfunc>
        tfunc GetFunction(const std::string& signature) {
            return reinterpret_cast<tfunc>(GetFunction(signature));
        }

        bool IsValid();

    protected:
#ifdef __linux__
        void* handle;
#endif

#ifdef WIN32
        HMODULE module;
#endif
    };
}

#endif
