#include "functions.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <map>

void processOperation(const std::string &opcode, const std::vector<std::string> &operands, std::string registers[], std::string memory[], int nzcv[]) {
    uint32_t result;
    bool isValid = true;
    std::string output = opcode + " ";
    for (const std::string & op : operands) {
        output += op + " ";
    }
    output += ": ";

    std::map<std::string, int> opcodeMap = {
        {"ADD", 1}, {"SUB", 2}, {"CMP", 3}, {"MOV", 4}, {"AND", 5}, {"OR", 6},
        {"XOR", 7}, {"LOAD", 8}, {"STORE", 9}, {"BAL", 10}, {"BEQ", 11}, {"BNE", 12}
    };

    std::map<std::string, int> registerMap = {
        {"R0", 0}, {"R1", 1}, {"R2", 2}, {"R3", 3},
        {"R4", 4}, {"R5", 5}, {"R6", 6}, {"R7", 7}
    };

    std::map<std::string, int> memoryMap = {
        {"0x100", 0}, {"0x104", 1}, {"0x108", 2}, {"0x10C", 3}, {"0x110", 4}
    };

    // Convert to opcode to mapping value, if doesn't exist -> 0
    int opcodeVal = opcodeMap.count(opcode) ? opcodeMap[opcode] : 0;

    // Validate operand count without returning early
    if ((opcodeVal > 0 && opcodeVal <= 2) || (opcodeVal > 4 && opcodeVal <= 7)) {
        if (operands.size() != 3) {
            std::cout << output << std::endl;
            std::cout << "Invalid Operand Count\n\n";
            return;
        }
    }
    else if ((opcodeVal > 2 && opcodeVal <= 3) || (opcodeVal > 7 && opcodeVal <= 9)) {
        if (operands.size() != 2) {
            std::cout << "Invalid Operand Count\n\n";
            return;
        }
    }
    else if (opcodeVal > 9 && opcodeVal <= 12) {
        if (operands.size() != 1) {
            std::cout << "Invalid Operand Count\n\n";
            return;
        }
    }
    else if (opcodeVal == 0) {
        std::cout << "Unsupported opcode\n\n";
        return;
    }


    int registerIndex;

    switch (opcodeVal) {
        case 1: // ADD
            try {
                registerIndex = registerMap.at(operands[0]);  // Destination
                std::cout << output << std::endl;
                // Check if Operand2 is an immediate value (starts with '#')
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    break;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 + val2;
                registers[registerIndex] = toHexString(result);
                printArrays(registers, memory);
                std::cout << std::endl;
            } catch (const std::exception& e) {
                output += "Error: " + std::string(e.what());
                std::cout << std::endl;
            }
            break;
        case 2: // SUB
            try {
                registerIndex = registerMap.at(operands[0]);  // Destination
                std::cout << output << std::endl;
                // Check if Operand2 is an immediate value (starts with '#')
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    break;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 - val2;
                registers[registerIndex] = toHexString(result);
                printArrays(registers, memory);
                std::cout << std::endl;
            } catch (const std::exception& e) {
                output += "Error: " + std::string(e.what());
                std::cout << std::endl;
            }
            break;       
        case 3: // CMP
            try {
                std::cout << output << std::endl;
                // Operand1 must be a register, not an immediate value
                if (operands[0][0] == '#') {
                    std::cout << "Invalid Instruction, first operand cannot be immediate\n\n";
                    break;
                }
                uint32_t val1 = getValue(operands[0], registers, registerMap);  
                uint32_t val2 = getValue(operands[1], registers, registerMap);  
                uint32_t result = val1 - val2;
        
                // Update Z flag 
                nzcv[1] = (result == 0) ? 1 : 0;  
        
                printArrays(registers, memory);
                std::cout << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            break;
        case 4: // MOV
            try {
                registerIndex = registerMap.at(operands[0]);  // Destination
                uint32_t val = getValue(operands[1], registers, registerMap);
                registers[registerIndex] = toHexString(val);
                std::cout << output << std::endl;
                printArrays(registers, memory);
                std::cout << std::endl;
            } catch (const std::exception& e) {
                output += "Error: " + std::string(e.what());
                std::cout << std::endl;
            }
            break;
        case 5: // AND
            try {
                registerIndex = registerMap.at(operands[0]);  // Destination
                std::cout << output << std::endl;
                // Check if Operand2 is an immediate value (starts with '#')
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    break;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 & val2;
                registers[registerIndex] = toHexString(result);
                printArrays(registers, memory);
                std::cout << std::endl;
            } catch (const std::exception& e) {
                output += "Error: " + std::string(e.what());
                std::cout << std::endl;
            }            
            break;
        case 6: // OR
            try {
                registerIndex = registerMap.at(operands[0]);  // Destination
                std::cout << output << std::endl;
                // Check if Operand2 is an immediate value (starts with '#')
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    break;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 | val2;
                registers[registerIndex] = toHexString(result);
                printArrays(registers, memory);
                std::cout << std::endl;
            } catch (const std::exception& e) {
                output += "Error: " + std::string(e.what());
                std::cout << std::endl;
            }            
            break;
        case 7: // XOR
            try {
                registerIndex = registerMap.at(operands[0]);  // Destination
                std::cout << output << std::endl;
                // Check if Operand2 is an immediate value (starts with '#')
                if (operands[1][0] == '#') {
                    std::cout << "Invalid Instruction, second operand cannot be immediate\n\n";
                    break;
                }
                uint32_t val1 = getValue(operands[1], registers, registerMap);
                uint32_t val2 = getValue(operands[2], registers, registerMap);
                result = val1 ^ val2;
                registers[registerIndex] = toHexString(result);
                printArrays(registers, memory);
                std::cout << std::endl;
            } catch (const std::exception& e) {
                output += "Error: " + std::string(e.what());
                std::cout << std::endl;
            }            
            break;            
            
        case 8: // LOAD
            try {
                registerIndex = registerMap.at(operands[0]);  // Destination 
                std::cout << output << std::endl;
        
                // Check if Operand2 is a valid memory address
                if (memoryMap.count(operands[1])) {
                    int memoryIndex = memoryMap.at(operands[1]);  
                    // If memory location is empty, treat it as 0; otherwise, convert from hex string
                    uint32_t value = (memory[memoryIndex].empty() ? 0 : std::stoul(memory[memoryIndex], nullptr, 16));
                    registers[registerIndex] = toHexString(value);  
                    printArrays(registers, memory);
                    std::cout << std::endl;
                } else {
                    std::cout << "Invalid Instruction. Memory out-of-range.\n\n";
                }
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            break;
        case 9: // STORE
            try {
                registerIndex = registerMap.at(operands[0]);  // Source
                std::cout << output << std::endl;
        
                // Check if Operand2 is a valid memory address
                if (memoryMap.count(operands[1])) {
                    int memoryIndex = memoryMap.at(operands[1]);  
                    // Get the value from the source register and store it in memory
                    uint32_t value = std::stoul(registers[registerIndex], nullptr, 16);
                    memory[memoryIndex] = toHexString(value);  
                    printArrays(registers, memory);
                    std::cout << std::endl;
                } else {
                    std::cout << "Invalid Instruction. Memory out-of-range.\n\n";
                }
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n\n";
            }
            break;                
            case 10: // BAL
            std::cout << output << std::endl;
            std::cout << "Branch will be taken to " << operands[0] << "\n\n";
            break;
        
        case 11: // BEQ
            std::cout << output << std::endl;
            if (nzcv[1] == 1) {  
                std::cout << "Branch will be taken to " << operands[0] << "\n\n";
            } else {
                std::cout << "Branch will not be taken to " << operands[0] << "\n\n";
            }
            break;
        
        case 12: // BNE
            std::cout << output << std::endl;
            if (nzcv[1] == 0) {  
                std::cout << "Branch will be taken to " << operands[0] << "\n\n";
            } else {
                std::cout << "Branch will not be taken to " << operands[0] << "\n\n";
            }
            break;
        case 0:
            output += "Unsupported Operation";
            isValid = false;
            break;
    }
}

std::string toHexString(uint32_t value) {
    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << value;
    return ss.str();
}

uint32_t getValue(const std::string& op, std::string registers[], const std::map<std::string, int>& registerMap) {
    if (op[0] == '#') { 
        std::string num = op.substr(1);  // Remove '#'
        return std::stoul(num, nullptr, 16);
    } else if (registerMap.count(op)) {  // Register
        return std::stoul(registers[registerMap.at(op)], nullptr, 16);
    }
    throw std::invalid_argument("Invalid operand: " + op);
}

void printArrays(std::string registers[], std::string memory[]) {
    // Print register array
    std::cout << "Register array:" << std::endl;
    std::cout << "R0 = 0x" << registers[0] << " R1 = 0x" << registers[1] 
              << " R2 = 0x" << registers[2] << " R3 = 0x" << registers[3] 
              << " R4 = 0x" << registers[4] << " R5 = 0x" << registers[5] 
              << " R6 = 0x" << registers[6] << " R7 = 0x" << registers[7] 
              << std::endl;

    // Print memory array
    std::cout << "Memory array:" << std::endl;
    std::cout << "0x100 = 0x" << (memory[0].empty() ? "0" : memory[0]) 
              << " 0x104 = 0x" << (memory[1].empty() ? "0" : memory[1]) 
              << " 0x108 = 0x" << (memory[2].empty() ? "0" : memory[2]) 
              << " 0x10C = 0x" << (memory[3].empty() ? "0" : memory[3]) 
              << " 0x110 = 0x" << (memory[4].empty() ? "0" : memory[4]) 
              << std::endl;
}