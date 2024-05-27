#include <fstream>
#include <iostream>
#include <string>

#include "lexical.h"

enum TokenType {
    Number,
    Identifier,
    Separator,
    Operator,
};

const std::set<std::string> keywords = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "inline", "int", "long", "register", "restrict", "return", "short",
    "signed", "sizeof", "static", "struct", "switch", "typedef",
    "union", "unsigned", "void", "volatile", "while", "_Alignas",
    "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic",
    "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local", "main"};

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("no file: " + filename);
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

int main(int argc, char* argv[]) {
    if (argc <= 1)
        return 1;
    const std::string numberRgex = "(+|-|@)((0|1|2|3|4|5|6|7|8|9)*.(0|1|2|3|4|5|6|7|8|9)|(0|1|2|3|4|5|6|7|8|9))(0|1|2|3|4|5|6|7|8|9)*";  // (+|-|@)(d*.d|d)(d)*
    const std::string identifierRgex = "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|_)(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|_|0|1|2|3|4|5|6|7|8|9)*";
    const std::string separatorRgex = ",|;|{|}|[|]|\\(|\\)";
    const std::string operatorRgex = "+|-|\\*|/|%|&|\\||=|+=|-=|\\*=|/=|==|++|--|<|>|<=|>=|&=|%=|\\|=|!=";

    Lexical lexical = Lexical({{numberRgex, TokenType::Number}, {identifierRgex, TokenType::Identifier}, {separatorRgex, TokenType::Separator}, {operatorRgex, TokenType::Operator}});

    std::string code = readFile(argv[1]);
    auto tokens = lexical.scan(code);
    for (auto it : tokens) {
        if (it.first == TokenType::Number)
            std::cout << "(数字" << "," << it.second << ")" << std::endl;
        else if (it.first == TokenType::Identifier) {
            if (keywords.find(it.second) != keywords.end())
                std::cout << "(关键字" << "," << it.second << ")" << std::endl;
            else
                std::cout << "(标识符" << "," << it.second << ")" << std::endl;
        } else if (it.first == TokenType::Separator)
            std::cout << "(分隔符" << "," << it.second << ")" << std::endl;
        else if (it.first == TokenType::Operator)
            std::cout << "(运算符" << "," << it.second << ")" << std::endl;
    }
    return 0;
}