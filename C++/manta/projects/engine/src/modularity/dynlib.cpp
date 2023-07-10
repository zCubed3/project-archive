#include "dynlib.hpp"

#ifdef __linux
#include <dlfcn.h>
#endif

#include <iostream>
#include <stdexcept>

namespace Manta {
    DynLib *DynLib::Open(const std::string &path) {
        auto lib = new DynLib();

#ifdef __linux__
        lib->handle = dlopen(path.c_str(), RTLD_LAZY);
#endif

#ifdef WIN32
        lib->module = LoadLibraryA(path.c_str());
#endif

        return lib;
    }

    void *DynLib::GetFunction(const std::string &signature) {
        if (!IsValid()) {
#if __linux__
            std::cerr << dlerror() << std::endl;
#endif

            throw std::logic_error("DynLib handle was invalid!");
        }

#ifdef __linux__
        return dlsym(handle, signature.c_str());
#endif

#ifdef WIN32
        return GetProcAddress(module, signature.c_str());
#endif
    }

    bool DynLib::IsValid() {
#ifdef __linux__
        return handle != nullptr;
#endif

#ifdef WIN32
        return module != nullptr;
#endif
    }
}