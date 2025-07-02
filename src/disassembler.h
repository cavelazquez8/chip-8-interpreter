#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <string>
#include <vector>
#include <cstdint>

struct DisassembledInstruction {
    std::uint16_t address;
    std::uint16_t opcode;
    std::string mnemonic;
    std::string description;
    bool is_current_pc = false;
};

class Disassembler {
public:
    static std::string disassembleInstruction(std::uint16_t opcode);
    static std::vector<DisassembledInstruction> disassembleMemory(
        const std::uint8_t* memory, 
        std::uint16_t start_address, 
        std::uint16_t count,
        std::uint16_t current_pc = 0);
        
private:
    static std::string formatHex(std::uint16_t value, int width = 4);
    static std::string formatRegister(std::uint8_t reg);
};

#endif // DISASSEMBLER_H