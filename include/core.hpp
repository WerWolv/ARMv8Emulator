#pragma once

#include <arm.hpp>

#include "register.hpp"
#include "address_space.hpp"
#include <functional>
#include <optional>

namespace arm {

    namespace ui { class Window; }

    #define INSTRUCTION(mask, pattern, name) InstructionPattern{ mask, pattern, &Core::name, #name }
    #define INSTRUCTION_DECL(name) void name(const inst_t &inst, const u8 &Rd, const u8 &Rn, const u8 &Rm, const bool &sf, const u8 &imm3, const u8 &imm6, const u16 &imm12, const u8 &shift, const u8 &size)
    #define INSTRUCTION_DEF(name) void Core::name(const inst_t &inst, const u8 &Rd, const u8 &Rn, const u8 &Rm, const bool &sf, const u8 &imm3, const u8 &imm6, const u16 &imm12, const u8 &shift, const u8 &size)

    class Core;
    using InstructionHandler = void (Core::*)(const inst_t &inst, const u8 &Rd, const u8 &Rn, const u8 &Rm, const bool &sf, const u8 &imm3, const u8 &imm6, const u16 &imm12, const u8 &shift, const u8 &size);

    struct InstructionPattern {
        u32 mask;
        u32 pattern;
        InstructionHandler type;
        const char *name;
    };

    struct PSTATE {
        u8 N : 1;
        u8 Z : 1;
        u8 C : 1;
        u8 V : 1;
        u8 D : 1;
        u8 A : 1;
        u8 I : 1;
        u8 F : 1;
        u8 SS : 1;
        u8 IL : 1;
        u8 EL : 2;
        u8 RW : 1;
        u8 SP : 1;
    };

    struct SPSR64 {
        u8 M : 5;
        u8 unused5 : 1;
        u8 F : 1;
        u8 I : 1;
        u8 A : 1;
        u8 D : 1;
        u16 unused10 : 10;
        u8 IL : 1;
        u8 SS : 1;
        u8 unused22 : 6;
        u8 V : 1;
        u8 C : 1;
        u8 Z : 1;
        u8 N : 1;
    };

    struct SPSR32 {
        u8 M : 5;
        u8 T : 1;
        u8 F : 1;
        u8 I : 1;
        u8 A : 1;
        u8 E : 1;
        u8 IT2 : 6;
        u8 GE : 4;
        u8 IL : 1;
        u8 unused21 : 3;
        u8 J : 1;
        u8 IT1 : 2;
        u8 Q : 1;
        u8 V : 1;
        u8 C : 1;
        u8 Z : 1;
        u8 N : 1;
    };

    constexpr u8 NumBreakpoints = 0x10;
    constexpr u8 TemporarySteppingBreakpointId = NumBreakpoints;

    class Core {
    public:
        Core(AddressSpace *addressSpace);

        void reset();
        void halt();
        void tick();

        [[nodiscard]] inst_t prefetch(const addr_t &pc) const;
        [[nodiscard]] InstructionHandler decode(const inst_t &instruction);
        void execute(const InstructionHandler &type, const inst_t &instruction);

        /* Debug commands */
        void enterDebugMode();
        void exitDebugMode();
        void breakCore();
        void continueCore();
        [[nodiscard]] u8 setBreakpoint(addr_t address);
        void removeBreakpoint(u8 breakpointId);
        void singleStep();
        void dumpRegisters();

        constexpr core::RegisterDouble& GPZR(u8 R) {
            return GPR[R];
        }

        constexpr core::RegisterDouble& GPSP(u8 R) {
            return GPR[R < 31 ? R : 32 + PSTATE.EL];
        }

    private:
        friend class arm::ui::Window;

        void setNZCVFlags(u32 oldValue, u32 newValue);
        void setNZCVFlags(u64 oldValue, u64 newValue);
        [[nodiscard]] bool doesConditionHold(u8 cond) const;
        [[nodiscard]] u64 decodeImmediateWMask(u32 N, u32 imms, u32 immr);

        bool m_halted = false;
        AddressSpace *m_addressSpace = nullptr;

        /* Debug */
        bool m_broken = false;
        bool m_debugMode = false;
        std::array<std::optional<addr_t>, NumBreakpoints + 1> m_breakpoints;
        InstructionPattern m_currInstruction;

        /* Core Registers */

        core::GPRegister GPR;
        core::RegisterSingle PC;

        PSTATE PSTATE;


        /* Floating Point Registers*/

        core::RegisterSingle FPCR;
        core::RegisterSingle FPSR;


        /* System Registers */

        core::ELRegister ACTLR;
        core::ELRegister CCSIDR;
        core::ELRegister CLIDR;
        core::ELRegister CNTFRQ;
        core::ELRegister CNTPCT;
        core::ELRegister CNTKCTL;
        core::ELRegister CNTP_CVAL;
        core::ELRegister CPACR;
        core::ELRegister CSSELR;
        core::ELRegister CNTP_CTL;
        core::ELRegister CTR;
        core::ELRegister DCZID;
        core::ELRegister ELR;
        core::ELRegister ESR;
        core::ELRegister FAR;
        core::ELRegister HCR;
        core::ELRegister MAIR;
        core::ELRegister MIDR;
        core::ELRegister MPIDR;
        core::ELRegister RVBAR;
        core::ELRegister SCR;
        core::ELRegister SCTLR;
        core::ELRegister SPSR;
        core::ELRegister TCR;
        core::ELRegister TPIDR;
        core::ELRegister TPIDRRO;
        core::ELRegister TTBR0;
        core::ELRegister TTBR1;
        core::ELRegister VBAR;
        core::ELRegister VTCR;
        core::ELRegister VTTBR;


        /* Instruction Handlers */
        constexpr auto getInstructionPatternLUT() const;

        INSTRUCTION_DECL(NOP);
        INSTRUCTION_DECL(ADD_IMMEDIATE);
        INSTRUCTION_DECL(ADD_SHIFTED_REGISTER);
        INSTRUCTION_DECL(ADDS_IMMEDIATE);
        INSTRUCTION_DECL(SUB_IMMEDIATE);
        INSTRUCTION_DECL(SUB_SHIFTED_REGISTER);
        INSTRUCTION_DECL(SUBS_IMMEDIATE);
        INSTRUCTION_DECL(SUBS_SHIFTED_REGISTER);
        INSTRUCTION_DECL(SUBS_EXTENDED_REGISTER);
        INSTRUCTION_DECL(ORR_IMMEDIATE);
        INSTRUCTION_DECL(ORR_SHIFTED_REGISTER);
        INSTRUCTION_DECL(MOVNZK);
        INSTRUCTION_DECL(B);
        INSTRUCTION_DECL(B_COND);
        INSTRUCTION_DECL(BL);
        INSTRUCTION_DECL(CCMN_IMMEDIATE);
        INSTRUCTION_DECL(CCMN_REGISTER);
        INSTRUCTION_DECL(SVC);
        INSTRUCTION_DECL(ADRP);
        INSTRUCTION_DECL(AND_IMMEDIATE);
        INSTRUCTION_DECL(AND_SHIFTED_REGISTER);
        INSTRUCTION_DECL(ANDS_IMMEDIATE);
        INSTRUCTION_DECL(ANDS_SHIFTED_REGISTER);
        INSTRUCTION_DECL(STR_IMMEDIATE);
        INSTRUCTION_DECL(STR_REGISTER);
        INSTRUCTION_DECL(LDR_IMMEDIATE);
        INSTRUCTION_DECL(LDR_REGISTER);
        INSTRUCTION_DECL(CBZ);

    };

}
