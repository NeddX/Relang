#ifndef ALVM_RUNTIME_UTILS_H
#define ALVM_RUNTIME_UTILS_H

#include <sdafx.h>

namespace rlang::alvm::utils {
    namespace ninvoke {
        /*void* Load(const char* dlib)
        {
            void* handle = dlopen(dlib, RLTD_LAZY);
            return handle;
        }*/
    } // namespace rlang::alvm::utils::ninvoke

    namespace gterm {
        // Source: https://stackoverflow.com/questions/448944/c-non-blocking-keyboard-input
        extern struct termios orig_termios;

        void reset_terminal_mode();
        void set_conio_terminal_mode();
        int kbhit();
        int getch();
    } // namespace rlang::alvm::utils::gterm
} // namespace rlang::alvm

#endif // ALVM_RUNTIME_UTILS_H
