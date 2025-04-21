#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>

struct OpcodeInfo {
    std::string baseOpcode;
    std::string condition;
    bool updatesFlags; // True if 'S' is present
};

OpcodeInfo parseOpcode(const std::string& opcode);
bool shouldExecute(const std::string& condition, int nzcv[]);
void updateNZCV(uint32_t result, uint32_t val1, uint32_t val2, bool isSubtraction, int nzcv[]);
size_t processOperation(const std::string& opcode, const std::vector<std::string>& operands,
                       std::string registers[], std::string memory[], int nzcv[],
                       const std::map<std::string, size_t>& labels, size_t currentPC);
std::string toHexString(uint32_t value);
uint32_t getValue(const std::string& op, std::string registers[], const std::map<std::string, int>& registerMap);
void printArrays(std::string registers[], std::string memory[], int nzcv[]);

#endif