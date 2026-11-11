#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "storage.h"

using namespace std;

// Change this number to make a 4-bit, 8-bit, or other fixed-width computer.
int bit_max = 4;
constexpr int CLOCK_HZ = 2;

// Write one instruction on each line in program.asm. The first line is line 0.
//
// Ben Eater 8-bit computer instructions:
//
// NOP address  - do nothing.
//                Example: "NOP"
//
// LDA memory   - copy memory into the accumulator.
//                Example: "LDA m1" means A = m1
//
// ADD memory   - add memory to the accumulator.
//                Example: "ADD m1" means A = A + m1
//
// SUB memory   - subtract memory from the accumulator.
//                Example: "SUB m1" means A = A - m1
//
// STA memory   - copy the accumulator into memory.
//                Example: "STA m1" means m1 = A
//
// LDI place number - put a number into a register or memory cell.
//                Example: "LDI r1 7" means r1 = 7
//                Example: "LDI m1 7" means m1 = 7
//
// JMP line     - always jump to a program line.
//                Example: "JMP 0" goes back to line 0
//
// JC line      - jump only when carry is true.
//                Example: "JC 0" jumps to line 0 if an add overflowed
//
// JZ line      - jump only when the accumulator is zero.
//                Example: "JZ 0" jumps to line 0 if A is 0
//
// JNZ line     - jump only when the accumulator is not zero.
//                Example: "JNZ 0" jumps to line 0 if A is not 0
//
// JNC line     - jump only when carry is false.
//                Example: "JNC 0" jumps to line 0 if there was no carry
//
// OUT          - print the accumulator.
//                Example: "OUT"
//
// HLT          - stop the computer.
//                Example: "HLT"
//
// Extra simple instructions from the original simulator are kept:
// ADD left right destination, SUB, AND, XOR, OR, RSH, and ADI destination value.
int* storageAddress(const string& name) {
    const auto value = storage.find(name);
    return value == storage.end() ? nullptr : &value->second;
}

int maximumValue() {
    return (1 << bit_max) - 1;
}

// Store a result as a real fixed-width computer would: keep only its bits.
int fitToBitWidth(int value, bool& carry) {
    const int limit = maximumValue();
    carry = value < 0 || value > limit;
    return value & limit;
}

bool parseLineNumber(const string& text, int& lineNumber) {
    char extraCharacter;
    stringstream input(text);

    return (input >> lineNumber)
        && !(input >> extraCharacter)
        && lineNumber >= 0;
}

bool isKnownInstruction(const string& command) {
    return command == "NOP" || command == "LDA" || command == "ADD"
        || command == "SUB" || command == "STA" || command == "LDI"
        || command == "JMP" || command == "JC" || command == "JZ"
        || command == "JNZ" || command == "JNC"
        || command == "OUT" || command == "HLT"
        || command == "ADI" || command == "AND" || command == "XOR"
        || command == "OR" || command == "RSH";
}

void printState(int accumulator, bool carry, bool zero) {
    cout << "   A = " << accumulator
         << ", carry = " << boolalpha << carry
         << ", zero = " << zero << '\n';
}

// Execute one instruction. Return false when the program should stop.
bool executeInstruction(
    const string& line,
    int& programCounter,
    int& accumulator,
    bool& carry,
    bool& zero
) {
    const size_t commentStart = line.find(';');
    const string instruction = line.substr(0, commentStart);
    string command;
    string firstOperand;
    string secondOperand;
    string destination;
    stringstream input(instruction);

    input >> command >> firstOperand >> secondOperand >> destination;
    if (command.empty()) {
        return true;
    }

    if (!isKnownInstruction(command)) {
        cerr << "unknown instruction: " << command << '\n';
        return false;
    }

    if (command == "HLT") {
        cout << "   computer stopped\n";
        return false;
    }

    if (command == "NOP") {
        return true;
    }

    if (command == "JMP" || command == "JC" || command == "JZ"
        || command == "JNZ" || command == "JNC") {
        int destinationLine;
        if (!parseLineNumber(firstOperand, destinationLine)) {
            cerr << "bad jump destination in: " << line << '\n';
            return false;
        }

        const bool shouldJump = command == "JMP"
            || (command == "JC" && carry)
            || (command == "JZ" && zero)
            || (command == "JNZ" && !zero)
            || (command == "JNC" && !carry);

        if (shouldJump) {
            programCounter = destinationLine;
            cout << "   jumping to line " << destinationLine << '\n';
        }
        return true;
    }

    if (command == "OUT") {
        cout << "   output: " << accumulator << '\n';
        return true;
    }

    // Three operands mean register/memory arithmetic:
    // ADD r1 m1 r2 means r2 = r1 + m1.
    if ((command == "ADD" || command == "SUB")
        && !secondOperand.empty() && !destination.empty()) {
        int* firstValue = storageAddress(firstOperand);
        int* secondValue = storageAddress(secondOperand);
        int* destinationValue = storageAddress(destination);

        if (!firstValue || !secondValue || !destinationValue) {
            cerr << "bad register or memory name in: " << line << '\n';
            return false;
        }

        const int result = command == "ADD"
            ? *firstValue + *secondValue
            : *firstValue - *secondValue;
        *destinationValue = fitToBitWidth(result, carry);
        zero = *destinationValue == 0;
        cout << "   " << destination << " = " << *destinationValue << '\n';
        cout << "   carry = " << boolalpha << carry
             << ", zero = " << zero << '\n';
        return true;
    }

    if (command == "LDI") {
        int number;
        if (!parseLineNumber(secondOperand, number)) {
            cerr << "bad number in: " << line << '\n';
            return false;
        }

        int* destination = storageAddress(firstOperand);
        if (!destination) {
            cerr << "bad destination in: " << line << '\n';
            return false;
        }

        *destination = fitToBitWidth(number, carry);
        zero = *destination == 0;
        cout << "   " << firstOperand << " = " << *destination << '\n';
        cout << "   carry = " << boolalpha << carry
             << ", zero = " << zero << '\n';
        return true;
    }

    if (command == "LDA" || command == "ADD" || command == "SUB"
        || command == "STA") {
        int* memory = storageAddress(firstOperand);
        if (!memory) {
            cerr << "bad memory name in: " << line << '\n';
            return false;
        }

        if (command == "LDA") {
            accumulator = *memory;
        } else if (command == "STA") {
            *memory = accumulator;
        } else {
            const int calculation = command == "ADD"
                ? accumulator + *memory
                : accumulator - *memory;
            accumulator = fitToBitWidth(calculation, carry);
        }

        zero = accumulator == 0;
        printState(accumulator, carry, zero);
        return true;
    }

    // The original register-to-register instructions still work here.
    int* firstValue = storageAddress(firstOperand);
    int* secondValue = storageAddress(secondOperand);
    int* destinationValue = storageAddress(destination);
    const bool writesToFirst = command == "ADI" || command == "LDI";

    if (!firstValue || !secondValue || (!writesToFirst && !destinationValue)) {
        cerr << "bad register or memory name in: " << line << '\n';
        return false;
    }

    int result;
    if (command == "ADI") {
        result = *firstValue + *secondValue;
        *firstValue = fitToBitWidth(result, carry);
    } else if (command == "LDI") {
        *firstValue = fitToBitWidth(*secondValue, carry);
    } else if (command == "ADD") {
        result = *firstValue + *secondValue;
        *destinationValue = fitToBitWidth(result, carry);
    } else if (command == "SUB") {
        result = *firstValue - *secondValue;
        *destinationValue = fitToBitWidth(result, carry);
    } else if (command == "AND") {
        *destinationValue = *firstValue & *secondValue;
    } else if (command == "XOR") {
        *destinationValue = *firstValue ^ *secondValue;
    } else if (command == "OR") {
        *destinationValue = *firstValue | *secondValue;
    } else if (command == "RSH") {
        *destinationValue = *firstValue >> *secondValue;
    }

    const int resultValue = writesToFirst ? *firstValue : *destinationValue;
    zero = resultValue == 0;
    cout << "   result = " << resultValue << '\n';
    printState(accumulator, carry, zero);
    return true;
}

int main(int argc, char* argv[]) {
    filesystem::path assemblyPath = "program.asm";
    if (argc > 0) {
        const filesystem::path executablePath = argv[0];
        if (!executablePath.parent_path().empty()) {
            assemblyPath = executablePath.parent_path() / "program.asm";
        }
    }

    ifstream assemblyFile(assemblyPath);
    if (!assemblyFile) {
        cerr << "could not open " << assemblyPath << '\n';
        return 1;
    }

    vector<string> program;
    string line;
    while (getline(assemblyFile, line)) {
        const size_t commentStart = line.find(';');
        const string instruction = line.substr(0, commentStart);

        if (!instruction.empty()
            && instruction.find_first_not_of(" \t\r\n") != string::npos) {
            program.push_back(instruction);
        }
    }

    int programCounter = 0;
    int accumulator = 0;
    bool carry = false;
    bool zero = false;
    const auto clockTick = chrono::milliseconds(1000 / CLOCK_HZ);

    while (programCounter >= 0
        && programCounter < static_cast<int>(program.size())) {
        const string& instruction = program[programCounter];
        cout << "[pc=" << programCounter << "] " << instruction << '\n';
        ++programCounter;

        if (!executeInstruction(
                instruction, programCounter, accumulator, carry, zero)) {
            break;
        }

        this_thread::sleep_for(clockTick);
    }

    cout << "program finished\n";
    return 0;
}
