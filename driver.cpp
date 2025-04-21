// Author: Tyler Vickers
// CS219 Programming Project 3

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <map>
#include <string>
#include "functions.h"

int main() {
    std::string registers[12] = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};
    std::string memory[5];
    int nzcv[4] = {0, 0, 0, 0};
    std::vector<std::string> instructions;
    std::map<std::string, size_t> labels;

    std::ifstream inputFile("PP3_input.txt");
    std::string line;

    if (!inputFile.is_open()) {
        std::cout << "Error opening input file\n";
        return 1;
    }

    // Read and store instructions, identify labels
    while (std::getline(inputFile, line)) {
        // Just an edge case, not needed but included
        if (line.empty()) continue;

        // Trim leading/trailing whitespace from the line
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string firstWord;
        iss >> firstWord;

        // Check if the first word is a label 
        std::string restOfLine;
        std::getline(iss, restOfLine);
        restOfLine.erase(0, restOfLine.find_first_not_of(" \t"));
        restOfLine.erase(restOfLine.find_last_not_of(" \t") + 1);

        // Check if restOfLine starts with a valid opcode
        std::istringstream restIss(restOfLine);
        std::string potentialOpcode;
        restIss >> potentialOpcode;
        std::map<std::string, bool> validOpcodes = {
            {"ADD", true}, {"SUB", true}, {"CMP", true}, {"MOV", true}, {"AND", true},
            {"ORR", true}, {"EOR", true}, {"LDR", true}, {"STR", true}, {"LSL", true},
            {"LSR", true}, {"MVN", true}, {"BEQ", true}
        };
        bool isLabel = !restOfLine.empty() && validOpcodes.count(potentialOpcode);

        if (isLabel) {
            // First word is a label (e.g., SKIP)
            std::string label = firstWord;
            labels[label] = instructions.size();
            // Store the instruction (rest of the line)
            if (!restOfLine.empty()) {
                instructions.push_back(restOfLine);
            } else {
                instructions.push_back("");
            }
        } else {
            // Not a label, store the entire line
            instructions.push_back(line);
        }
    }
    inputFile.close();

    // This is for debugging. The SKIP branch was not being taken to,
    // Visualizing the labels helped see where instructions were being taken
    std::cout << "Labels map:\n";
    for (const auto& [label, index] : labels) {
        std::cout << "Label: " << label << ", Index: " << index << "\n";
    }
    std::cout << "Instructions:\n";
    for (size_t i = 0; i < instructions.size(); ++i) {
        std::cout << "Index " << i << ": " << instructions[i] << "\n";
    }

    // Process instructions using a program counter
    size_t pc = 0;
    while (pc < instructions.size()) {
        // Skip empty instructions
        if (instructions[pc].empty()) {
            pc++;
            continue;
        }

        std::istringstream iss(instructions[pc]);
        std::vector<std::string> words;
        std::string word;

        // Split by spaces and commas
        while (iss >> word) {
            std::string token;
            std::istringstream tokenStream(word);
            while (std::getline(tokenStream, token, ',')) {
                if (!token.empty()) {
                    words.push_back(token);
                }
            }
        }

        if (words.empty()) {
            pc++;
            continue;
        }

        std::string opcode = words[0];
        std::vector<std::string> operands(words.begin() + 1, words.end());
        // Process operation and get the next PC
        pc = processOperation(opcode, operands, registers, memory, nzcv, labels, pc);
    }

    return 0;
}