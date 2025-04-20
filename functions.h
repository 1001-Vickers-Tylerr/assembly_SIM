#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>

void processOperation(const std::string& opcode, const std::vector<std::string> &operands, std::string registers[], std::string memory[], int nzcv[]);
std::string toHexString(uint32_t value);
uint32_t getValue(const std::string& op, std::string registers[], const std::map<std::string, int>& registerMap);
void printArrays(std::string registers[], std::string memory[]);

#endif