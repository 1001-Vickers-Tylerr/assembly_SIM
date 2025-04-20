// Author: Tyler Vickers
// CS219 Programming Project 2

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include "functions.h"

int main() {
    std::string registers[12] = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};
    std::string memory[5];
    int nzcv[4] = {0, 0, 0, 0};

    std::ifstream inputFile("pp2_input.txt");
    std::string line;

    if (!inputFile.is_open()) {
        std::cout << "Error opening input file\n";
        return 1;
    }

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        std::vector<std::string> words;
        std::string word;

        // Split by spaces first
        while (iss >> word) {
            // Split by commas within each word
            std::string token;
            std::istringstream tokenStream(word);
            while (std::getline(tokenStream, token, ',')) {
                if (!token.empty()) {
                    words.push_back(token);
                }
            }
        }

        if (words.empty()) continue;

        std::string opcode = words[0];
        std::vector<std::string> operands(words.begin() + 1, words.end());
        processOperation(opcode, operands, registers, memory, nzcv);
    }

    return 0;
}