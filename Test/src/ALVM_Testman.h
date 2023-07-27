#ifndef ALVM_TEST_ALVM_TESTMAN_H
#define ALVM_TEST_ALVM_TESTMAN_H

#include <ALVM.h>

#include "CommonUtils.h"

namespace testman::alvm {
    void SimplePrint()
    {
        /*
        using namespace rlang::alvm;
        ALVM r;

        std::int32_t result = 0;

        std::vector<Instruction> code =
        {
        Instruction{.opcode = OpCode::Mov, .reg1 = {RegType::R0}, .reg2 = {RegType::SP}},
                Instruction{.opcode = OpCode::Db, .bytes = std::to_bytes("it workin\n")},
                Instruction{.opcode = OpCode::PStr, .reg1 = {RegType::R0}},
                Instruction{.opcode = OpCode::End}
        };

        r.Run(code, result);
        */
    }
} // namespace testman::alvm

#endif // ALVM_TEST_ALVM_TESTMAN_H
