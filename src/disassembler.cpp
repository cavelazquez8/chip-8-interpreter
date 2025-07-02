#include "disassembler.h"
#include <sstream>
#include <iomanip>

std::string Disassembler::disassembleInstruction(std::uint16_t opcode) {
    std::uint8_t first_nibble = (opcode & 0xF000) >> 12;
    std::uint8_t x = (opcode & 0x0F00) >> 8;
    std::uint8_t y = (opcode & 0x00F0) >> 4;
    std::uint8_t n = opcode & 0x000F;
    std::uint8_t nn = opcode & 0x00FF;
    std::uint16_t nnn = opcode & 0x0FFF;
    
    switch (first_nibble) {
        case 0x0:
            switch (nn) {
                case 0xE0: return "CLS";
                case 0xEE: return "RET";
                default: return "SYS " + formatHex(nnn);
            }
            
        case 0x1: return "JP " + formatHex(nnn);
        case 0x2: return "CALL " + formatHex(nnn);
        case 0x3: return "SE " + formatRegister(x) + ", " + formatHex(nn, 2);
        case 0x4: return "SNE " + formatRegister(x) + ", " + formatHex(nn, 2);
        case 0x5: return "SE " + formatRegister(x) + ", " + formatRegister(y);
        case 0x6: return "LD " + formatRegister(x) + ", " + formatHex(nn, 2);
        case 0x7: return "ADD " + formatRegister(x) + ", " + formatHex(nn, 2);
        
        case 0x8:
            switch (n) {
                case 0x0: return "LD " + formatRegister(x) + ", " + formatRegister(y);
                case 0x1: return "OR " + formatRegister(x) + ", " + formatRegister(y);
                case 0x2: return "AND " + formatRegister(x) + ", " + formatRegister(y);
                case 0x3: return "XOR " + formatRegister(x) + ", " + formatRegister(y);
                case 0x4: return "ADD " + formatRegister(x) + ", " + formatRegister(y);
                case 0x5: return "SUB " + formatRegister(x) + ", " + formatRegister(y);
                case 0x6: return "SHR " + formatRegister(x) + " {, " + formatRegister(y) + "}";
                case 0x7: return "SUBN " + formatRegister(x) + ", " + formatRegister(y);
                case 0xE: return "SHL " + formatRegister(x) + " {, " + formatRegister(y) + "}";
                default: return "UNK " + formatHex(opcode);
            }
            
        case 0x9: return "SNE " + formatRegister(x) + ", " + formatRegister(y);
        case 0xA: return "LD I, " + formatHex(nnn);
        case 0xB: return "JP V0, " + formatHex(nnn);
        case 0xC: return "RND " + formatRegister(x) + ", " + formatHex(nn, 2);
        case 0xD: return "DRW " + formatRegister(x) + ", " + formatRegister(y) + ", " + std::to_string(n);
        
        case 0xE:
            switch (nn) {
                case 0x9E: return "SKP " + formatRegister(x);
                case 0xA1: return "SKNP " + formatRegister(x);
                default: return "UNK " + formatHex(opcode);
            }
            
        case 0xF:
            switch (nn) {
                case 0x07: return "LD " + formatRegister(x) + ", DT";
                case 0x0A: return "LD " + formatRegister(x) + ", K";
                case 0x15: return "LD DT, " + formatRegister(x);
                case 0x18: return "LD ST, " + formatRegister(x);
                case 0x1E: return "ADD I, " + formatRegister(x);
                case 0x29: return "LD F, " + formatRegister(x);
                case 0x33: return "LD B, " + formatRegister(x);
                case 0x55: return "LD [I], " + formatRegister(x);
                case 0x65: return "LD " + formatRegister(x) + ", [I]";
                default: return "UNK " + formatHex(opcode);
            }
            
        default: return "UNK " + formatHex(opcode);
    }
}

std::vector<DisassembledInstruction> Disassembler::disassembleMemory(
    const std::uint8_t* memory, 
    std::uint16_t start_address, 
    std::uint16_t count,
    std::uint16_t current_pc) {
    
    std::vector<DisassembledInstruction> instructions;
    instructions.reserve(count);
    
    for (std::uint16_t i = 0; i < count; ++i) {
        std::uint16_t address = start_address + (i * 2);
        std::uint16_t opcode = (memory[address] << 8) | memory[address + 1];
        
        DisassembledInstruction instr;
        instr.address = address;
        instr.opcode = opcode;
        instr.mnemonic = disassembleInstruction(opcode);
        instr.is_current_pc = (address == current_pc);
        
        // Add description based on opcode
        std::uint8_t first_nibble = (opcode & 0xF000) >> 12;
        switch (first_nibble) {
            case 0x0: 
                if ((opcode & 0x00FF) == 0xE0) instr.description = "Clear screen";
                else if ((opcode & 0x00FF) == 0xEE) instr.description = "Return from subroutine";
                else instr.description = "System call";
                break;
            case 0x1: instr.description = "Jump to address"; break;
            case 0x2: instr.description = "Call subroutine"; break;
            case 0x3: instr.description = "Skip if register equals value"; break;
            case 0x4: instr.description = "Skip if register not equals value"; break;
            case 0x5: instr.description = "Skip if registers equal"; break;
            case 0x6: instr.description = "Set register to value"; break;
            case 0x7: instr.description = "Add value to register"; break;
            case 0x8: instr.description = "Arithmetic operation"; break;
            case 0x9: instr.description = "Skip if registers not equal"; break;
            case 0xA: instr.description = "Set index register"; break;
            case 0xB: instr.description = "Jump to V0 + address"; break;
            case 0xC: instr.description = "Random number AND value"; break;
            case 0xD: instr.description = "Draw sprite"; break;
            case 0xE: instr.description = "Key operation"; break;
            case 0xF: instr.description = "Timer/Memory operation"; break;
            default: instr.description = "Unknown instruction"; break;
        }
        
        instructions.push_back(instr);
    }
    
    return instructions;
}

std::string Disassembler::formatHex(std::uint16_t value, int width) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << value;
    return ss.str();
}

std::string Disassembler::formatRegister(std::uint8_t reg) {
    if (reg < 16) {
        std::stringstream ss;
        ss << "V" << std::hex << std::uppercase << static_cast<int>(reg);
        return ss.str();
    }
    return "V?";
}