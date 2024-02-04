#ifndef BLEND_RUNTIME_UTILS_H
#define BLEND_RUNTIME_UTILS_H

#include <sdafx.h>

namespace relang::blend::utils {
    namespace ninvoke {
        /*void* Load(const char* dlib)
        {
            void* handle = dlopen(dlib, RLTD_LAZY);
            return handle;
        }*/
    } // namespace relang::blend::utils::ninvoke

    namespace gterm {
#ifdef BLEND_PLATFORM_UNIX
        // Source: https://stackoverflow.com/questions/448944/c-non-blocking-keyboard-input
        extern struct termios orig_termios;

        void reset_terminal_mode();
        void set_conio_terminal_mode();
        int kbhit();
        int getch();
#endif
    } // namespace relang::blend::utils::gterm
} // namespace relang::blend

#endif // BLEND_RUNTIME_UTILS_H
