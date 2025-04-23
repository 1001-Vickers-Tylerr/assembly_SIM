#include "functions.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <map>

OpcodeInfo parseOpcode(const std::string& opcode) {
    OpcodeInfo info; // Defined in functions.h
    info.updatesFlags = false;
    info.condition = ""; // Default: no condition 
    std::string temp = opcode;

    // Check for 'S' flag (must be at the end)
    if (!temp.empty() && temp.back() == 'S') {
        info.updatesFlags = true;
        temp = temp.substr(0, temp.length() - 1);
    }

    // Special case for BEQ to avoid misparsing as B+EQ
    if (temp == "BEQ") {
        info.baseOpcode = "BEQ";
        info.condition = "";
        return info;
    }

    // Check for condition suffix (last 2 characters)
    if (temp.length() >= 2) {
        std::string potentialCond = temp.substr(temp.length() - 2);
        if (potentialCond == "GT" || potentialCond == "GE" || potentialCond == "LT" ||
            potentialCond == "LE" || potentialCond == "EQ" || potentialCond == "NE") {
            info.condition = potentialCond;
            info.baseOpcode = temp.substr(0, temp.length() - 2);
        } else {
            info.baseOpcode = temp;
        }
    } else {
        info.baseOpcode = temp;
    }

    return info;
}

bool shouldExecute(const std::string& condition, int nzcv[]) {
    if (condition.empty()) return true; // Unconditional execution
    if (condition == "GT") return nzcv[1] == 0 && nzcv[0] == nzcv[3]; // Z == 0 && N == V
    if (condition == "GE") return nzcv[0] == nzcv[3]; // N == V
    if (condition == "LT") return nzcv[0] != nzcv[3]; // N != V
    if (condition == "LE") return nzcv[1] == 1 || nzcv[0] != nzcv[3]; // Z == 1 || N != V
    if (condition == "EQ") return nzcv[1] == 1; // Z == 1
    if (condition == "NE") return nzcv[1] == 0; // Z == 0
    return false; // Unknown condition
}

void updateNZCV(uint32_t result, uint32_t val1, uint32_t val2, bool isSubtraction, int nzcv[]) {
    // N: Negative 
    nzcv[0] = (result & 0x80000000) ? 1 : 0;
    // Z: Zero
    nzcv[1] = (result == 0) ? 1 : 0;
    // C: Carry
    if (isSubtraction) {
        nzcv[2] = (val1 >= val2) ? 1 : 0; // No borrow
    } else {
        nzcv[2] = (result < val1 || result < val2) ? 1 : 0; // Overflow for addition
    }
    // V: Overflow
    bool signVal1 = (val1 & 0x80000000) != 0;
    bool signVal2 = (val2 & 0x80000000) != 0;
    bool signResult = (result & 0x80000000) != 0;
    if (isSubtraction) {
        nzcv[3] = (signVal1 != signVal2 && signResult != signVal1) ? 1 : 0;
    } else {
        nzcv[3] = (signVal1 == signVal2 && signResult != signVal1) ? 1 : 0;
    }
}

size_t processOperation(const std::string& opcode, const std::vector<std::string>& operands,
                        std::string registers[], std::string memory[], int nzcv[],
                        const std::map<std::string, size_t>& labels, size_t currentPC) {
    // Parse the opcode
    OpcodeInfo info = parseOpcode(opcode);
    
    // Build output string 
    std::string output = opcode;
    for (size_t i = 0; i < operands.size(); ++i) {
        output += (i == 0 ? " " : ", ") + operands[i];
    }
    std::cout << output << "\n";

    // Check if the instruction should execute based on condition
    if (!shouldExecute(info.condition, nzcv)) {
        std::cout << "Instruction not executed due to condition\n";
        printArrays(registers, memory, nzcv);
        std::cout << "\n";
        return currentPC + 1;
    }

    // Map opcode to operation
    std::map<std::string, int> opcodeMap = {
        {"ADD", 1}, {"SUB", 2}, {"CMP", 3}, {"MOV", 4}, {"AND", 5}, {"ORR", 6},
        {"EOR", 7}, {"LDR", 8}, {"STR", 9}, {"LSL", 10}, {"LSR", 11}, {"MVN", 12},
        {"BEQ", 13}
    };

    int opcodeVal = opcodeMap.count(info.baseOpcode) ? opcodeMap[info.baseOpcode] : 0;

    // Define register and memory maps
    std::map<std::string, int> registerMap = {
        {"R0", 0}, {"R1", 1}, {"R2", 2}, {"R3", 3},
        {"R4", 4}, {"R5", 5}, {"R6", 6}, {"R7", 7},
        {"R8", 8}, {"R9", 9}, {"R10", 10}, {"R11", 11}
    };
    std::map<std::string, int> memoryMap = {
        {"0x100", 0}, {"0x104", 1}, {"0x108", 2}, {"0x10C", 3}, {"0x110", 4}
    };

    // Validate operand count
    if ((opcodeVal >= 1 && opcodeVal <= 2) || (opcodeVal >= 5 && opcodeVal <= 7) ||
        (opcodeVal >= 10 && opcodeVal <= 11)) {
        if (operands.size() != 3) {
            std::cout << "Invalid Operand Count\n\n";
            return currentPC + 1;
        }
    } else if ((opcodeVal >= 3 && opcodeVal <= 4) || (opcodeVal >= 8 && opcodeVal <= 9) ||
               opcodeVal == 12) {
        if (operands.size() != 2) {
            std::cout << "Invalid Operand Count\n\n";
            return currentPC + 1;
        }
    } else if (opcodeVal == 13) {
        if (operands.size() != 1) {
            std::cout << "Invalid Operand Count\n\n";
            return currentPC + 1;
        }
    } else {
        std::cout << "Unsupported opcode: " << info.baseOpcode << "\n\n";
        return currentPC + 1;
    }

    uint32_t result;
    int registerIndex;

    switch (opcodeVal) {
        case 1: // ADD
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    return currentPC + 1;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 + val2;
                registers[registerIndex] = toHexString(result);
                if (info.updatesFlags) {
                    updateNZCV(result, val1, val2, false, nzcv);
                }
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 2: // SUB
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    return currentPC + 1;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 - val2;
                registers[registerIndex] = toHexString(result);
                if (info.updatesFlags) {
                    updateNZCV(result, val1, val2, true, nzcv);
                }
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 3: // CMP
            try {
                if (operands[0][0] == '#') {
                    std::cout << "Invalid Instruction, first operand cannot be immediate\n\n";
                    return currentPC + 1;
                }
                uint32_t val1 = getValue(operands[0], registers, registerMap);
                uint32_t val2 = getValue(operands[1], registers, registerMap);
                result = val1 - val2;
                updateNZCV(result, val1, val2, true, nzcv);
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 4: // MOV
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                uint32_t val = getValue(operands[1], registers, registerMap);
                registers[registerIndex] = toHexString(val);
                if (info.updatesFlags) {
                    updateNZCV(val, val, 0, false, nzcv);
                }
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 5: // AND
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    return currentPC + 1;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 & val2;
                registers[registerIndex] = toHexString(result);
                if (info.updatesFlags) {
                    updateNZCV(result, val1, val2, false, nzcv);
                }
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 6: // ORR
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    return currentPC + 1;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 | val2;
                registers[registerIndex] = toHexString(result);
                if (info.updatesFlags) {
                    updateNZCV(result, val1, val2, false, nzcv);
                }
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 7: // EOR
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    return currentPC + 1;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 ^ val2;
                registers[registerIndex] = toHexString(result);
                if (info.updatesFlags) {
                    updateNZCV(result, val1, val2, false, nzcv);
                }
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 8: // LDR
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                // Check for pointer
                std::string memOperand = operands[1];
                if (memOperand.front() == '[' && memOperand.back() == ']') {
                    std::string reg = memOperand.substr(1, memOperand.length() - 2); // Remove [ ]
                    if (registerMap.count(reg)) {
                        uint32_t addr = std::stoul(registers[registerMap.at(reg)], nullptr, 16);
                        std::stringstream ss;
                        ss << std::hex << addr;
                        std::string addrStr = "0x" + ss.str();
                        if (memoryMap.count(addrStr)) {
                            int memoryIndex = memoryMap.at(addrStr);
                            uint32_t value = (memory[memoryIndex].empty() ? 0 : std::stoul(memory[memoryIndex], nullptr, 16));
                            registers[registerIndex] = toHexString(value);
                            printArrays(registers, memory, nzcv);
                            std::cout << "\n";
                        } else {
                            std::cout << "Invalid Instruction. Memory address " << addrStr << " out-of-range.\n\n";
                        }
                    } else {
                        std::cout << "Invalid Instruction. Invalid register in memory operand.\n\n";
                    }
                } else if (memoryMap.count(memOperand)) {
                    int memoryIndex = memoryMap.at(memOperand);
                    uint32_t value = (memory[memoryIndex].empty() ? 0 : std::stoul(memory[memoryIndex], nullptr, 16));
                    registers[registerIndex] = toHexString(value);
                    if (info.updatesFlags) {
                        updateNZCV(value, value, 0, false, nzcv);
                    }
                    printArrays(registers, memory, nzcv);
                    std::cout << "\n";
                } else {
                    std::cout << "Invalid Instruction. Memory out-of-range.\n\n";
                }
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 9: // STR
            try {
                registerIndex = registerMap.at(operands[0]); // Source
                // Check for pointer
                std::string memOperand = operands[1];
                if (memOperand.front() == '[' && memOperand.back() == ']') {
                    std::string reg = memOperand.substr(1, memOperand.length() - 2); // Remove [ ]
                    if (registerMap.count(reg)) {
                        uint32_t addr = std::stoul(registers[registerMap.at(reg)], nullptr, 16);
                        std::stringstream ss;
                        ss << std::hex << addr;
                        std::string addrStr = "0x" + ss.str();
                        if (memoryMap.count(addrStr)) {
                            int memoryIndex = memoryMap.at(addrStr);
                            uint32_t value = std::stoul(registers[registerIndex], nullptr, 16);
                            memory[memoryIndex] = toHexString(value);
                            if (info.updatesFlags) {
                                updateNZCV(value, value, 0, false, nzcv);
                            }
                            printArrays(registers, memory, nzcv);
                            std::cout << "\n";
                        } else {
                            std::cout << "Invalid Instruction. Memory address " << addrStr << " out-of-range.\n\n";
                        }
                    } else {
                        std::cout << "Invalid Instruction. Invalid register in memory operand.\n\n";
                    }
                } else if (memoryMap.count(memOperand)) {
                    int memoryIndex = memoryMap.at(memOperand);
                    uint32_t value = std::stoul(registers[registerIndex], nullptr, 16);
                    memory[memoryIndex] = toHexString(value);
                    if (info.updatesFlags) {
                        updateNZCV(value, value, 0, false, nzcv);
                    }
                    printArrays(registers, memory, nzcv);
                    std::cout << "\n";
                } else {
                    std::cout << "Invalid Instruction. Memory out-of-range.\n\n";
                }
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 10: // LSL
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    return currentPC + 1;
                }
                if (operands[2][0] != '#') {
                    std::cout << "Invalid Instruction, third operand must be immediate\n\n";
                    return currentPC + 1;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t shift = std::stoul(operands[2].substr(1), nullptr, 16);
                result = val1 << shift;
                registers[registerIndex] = toHexString(result);
                if (info.updatesFlags) {
                    updateNZCV(result, val1, 0, false, nzcv);
                }
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 11: // LSR
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    return currentPC + 1;
                }
                if (operands[2][0] != '#') {
                    std::cout << "Invalid Instruction, third operand must be immediate\n\n";
                    return currentPC + 1;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t shift = std::stoul(operands[2].substr(1), nullptr, 16);
                result = val1 >> shift;
                registers[registerIndex] = toHexString(result);
                if (info.updatesFlags) {
                    updateNZCV(result, val1, 0, false, nzcv);
                }
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 12: // MVN
            try {
                registerIndex = registerMap.at(operands[0]); // Destination
                uint32_t val = getValue(operands[1], registers, registerMap);
                result = ~val;
                registers[registerIndex] = toHexString(result);
                if (info.updatesFlags) {
                    updateNZCV(result, val, 0, false, nzcv);
                }
                printArrays(registers, memory, nzcv);
                std::cout << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            return currentPC + 1;

        case 13: // BEQ
            try {
                if (nzcv[1] == 1 && labels.count(operands[0])) {
                    std::cout << "Branch will be taken to " << operands[0] << "\n";
                    printArrays(registers, memory, nzcv);
                    std::cout << "\n";
                    return labels.at(operands[0]);
                } else {
                    std::cout << "Branch will not be taken to " << operands[0] << "\n";
                    printArrays(registers, memory, nzcv);
                    std::cout << "\n";
                    return currentPC + 1;
                }
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
                return currentPC + 1;
            }

        default:
            std::cout << "Unsupported operation\n\n";
            return currentPC + 1;
    }
}

// Convert to hex string for printing/storing
std::string toHexString(uint32_t value) {
    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << value;
    return ss.str();
}

// Takes a value, immediate or from register
// And convert to uint32
uint32_t getValue(const std::string& op, std::string registers[], const std::map<std::string, int>& registerMap) {
    if (op[0] == '#') { 
        std::string num = op.substr(1);  // Remove '#'
        return std::stoul(num, nullptr, 16);
    } else if (registerMap.count(op)) {  // Register
        return std::stoul(registers[registerMap.at(op)], nullptr, 16);
    }
    throw std::invalid_argument("Invalid operand: " + op);
}

void printArrays(std::string registers[], std::string memory[], int nzcv[]) {
    std::cout << "Register array:\n";
    std::cout << "R0 = 0x" << registers[0] << " R1 = 0x" << registers[1]
              << " R2 = 0x" << registers[2] << " R3 = 0x" << registers[3]
              << " R4 = 0x" << registers[4] << " R5 = 0x" << registers[5]
              << " R6 = 0x" << registers[6] << " R7 = 0x" << registers[7] << "\n";
    std::cout << "R8 = 0x" << registers[8] << " R9 = 0x" << registers[9]
              << " R10 = 0x" << registers[10] << " R11 = 0x" << registers[11] << "\n";
    
    std::cout << "NZCV: " << nzcv[0] << nzcv[1] << nzcv[2] << nzcv[3] << "\n";
    
    std::cout << "Memory array:\n";
    std::cout << "0x100 = 0x" << (memory[0].empty() ? "00000000" : memory[0]) << " "
              << "0x104 = 0x" << (memory[1].empty() ? "00000000" : memory[1]) << " "
              << "0x108 = 0x" << (memory[2].empty() ? "00000000" : memory[2]) << " "
              << "0x10C = 0x" << (memory[3].empty() ? "00000000" : memory[3]) << " "
              << "0x110 = 0x" << (memory[4].empty() ? "00000000" : memory[4]) << "\n";
}