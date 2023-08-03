#ifndef ALVM_RUNTIME_UTILS_H
#define ALVM_RUNTIME_UTILS_H

#include <sdafx.h>

namespace rlang::alvm::utils {
    namespace ninvoke
    {
        void* Load(const char* dlib)
        {
            void* handle = dlopen(dlib, RLTD_LAZY);
            return handle;
        }
    } // namespace rlang::alvm::utils::ninvoke
} // namespace rlang::alvm

#endif // ALVM_RUNTIME_UTILS_H
