#include <xbyak/xbyak.h>
#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <filesystem>
#include <stack>
#include <fstream>

FILE* outf;

void printChar(uint8_t ch) {
    fprintf(outf, "%c", ch);
}

uint8_t getChar() {
    return getchar();
}

using Xbyak::Label;

struct Code : public Xbyak::CodeGenerator {
    Code(std::string code) : Xbyak::CodeGenerator(1000000)
    {
        arr_.resize(3000);
        std::stack<Label> start_labels, end_labels;
        push(r15);
        push(r12);
        push(r13);
        mov(r15, (uintptr_t)arr_.data());
        mov(r12, (uintptr_t)&printChar);
        mov(r13, (uintptr_t)&getChar);
        for (auto c : code) {
            switch (c) {
                case '>': {
                    add(r15, 1);
                    break;
                }
                case '<': {
                    sub(r15, 1);
                    break;
                }
                case '+': {
                    add(byte[r15], 1);
                    break;
                }
                case '-': {
                    sub(byte[r15], 1);
                    break;
                }
                case '.': {
                    movzx(rdi, byte[r15]);
                    call(r12);
                    break;
                }
                case ',': {
                    call(r13);
                    mov(byte[r15], rax);
                    break;
                }
                case '[': {
                    Label cur = L();
                    start_labels.push(cur);
                    movzx(rdx, byte[r15]);
                    cmp(rdx, 0);
                    Label end;
                    jz(end, T_NEAR);
                    end_labels.push(end);
                    break;
                }
                case ']': {
                    Label start = start_labels.top();
                    start_labels.pop();
                    jmp(start);
                    Label end = end_labels.top();
                    end_labels.pop();
                    L(end);
                    break;
                }
                default: {
                    break;
                }
            }
        }
        pop(r13);
        pop(r12);
        pop(r15);
        ret();
    }
private:
    std::vector<uint8_t> arr_;
};

int main(int argc, const char** argv) {
    if (argc != 2)
        return 1;
    std::string path = argv[1];
    if (!std::filesystem::is_regular_file(path)) {
        std::cout << "not a file: " << path << std::endl;
        return 1;
    }
    std::ifstream ifs(path);
    std::string input;
    if (ifs.good()) {
        std::stringstream ss;
        ss << ifs.rdbuf();
        input = ss.str();
    }
    outf = fopen("out.txt", "w");
    Code c(input);
    uint8_t* emitted = c.getCode<uint8_t*>();
    mprotect(emitted, getpagesize(), PROT_EXEC);
    ((void(*)())(emitted))();
    fclose(outf);
    return 0;
}