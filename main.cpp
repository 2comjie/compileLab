#include <fstream>
#include <iostream>
#include <string>

#include "lexical.h"
#include "syntax.h"

enum TokenType {
    Number,
    Identifier,
    Separator,
    Operator,
};

const std::set<std::string> keywords = {
    "int", "float", "char", "double", "long", "void", "return", "for", "while", "if", "else"};

const std::string numberRgex = "((0|1|2|3|4|5|6|7|8|9)*.(0|1|2|3|4|5|6|7|8|9)|(0|1|2|3|4|5|6|7|8|9))(0|1|2|3|4|5|6|7|8|9)*";  // (d*.d|d)(d)*
const std::string identifierRgex = "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|_)(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|_|0|1|2|3|4|5|6|7|8|9)*";
const std::string separatorRgex = ",|;|{|}|[|]|\\(|\\)";
const std::string operatorRgex = "+|-|\\*|/|%|&|\\||=|+=|-=|\\*=|/=|==|++|--|<|>|<=|>=|&=|%=|\\|=|!=";

// 文法的产生式
std::vector<std::pair<std::string, std::vector<std::string>>> productions = {
    {"E", {"T", "E'"}},        // E -> T E'
    {"E'", {"+", "T", "E'"}},  // E' -> + T E'
    {"E'", {"-", "T", "E'"}},  // E' -> - T E'
    {"E'", {"@"}},             // E' -> ε
    {"T", {"F", "T'"}},        // T -> F T'
    {"T'", {"*", "F", "T'"}},  // T' -> * F T'
    {"T'", {"/", "F", "T'"}},  // T' -> / F T'
    {"T'", {"@"}},             // T' -> ε
    {"F", {"(", "E", ")"}},    // F -> ( E )
    {"F", {"num"}},            // F -> num
    {"F", {"id"}}              // F -> id
};
// 终结符集合
std::set<std::string>
    terminals = {"+", "-", "*", "/", "(", ")", "num", "id"};
// 非终结符集合
std::set<std::string> nonTerminals = {"E", "E'", "T", "T'", "F"};
// 开始符号
std::string startSymbol = "E";

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
    if (argc <= 1) {
        std::cout << "输入要分析的源文件" << std::endl;
        return 1;
    }

    Lexical lexical = Lexical({{numberRgex, TokenType::Number}, {identifierRgex, TokenType::Identifier}, {separatorRgex, TokenType::Separator}, {operatorRgex, TokenType::Operator}});
    std::string code = readFile(argv[1]);
    auto tokens = lexical.scan(code);

    // 细分种别代码 (int) (float) (void)
    std::vector<std::pair<std::string, std::string>> tokens1;
    for (auto it : tokens) {
        if (it.first == TokenType::Number) {
            tokens1.push_back({"num", it.second});
        } else if (it.first == TokenType::Identifier) {
            if (keywords.find(it.second) != keywords.end()) {
                tokens1.push_back({it.second, it.second});
            } else {
                tokens1.push_back({"id", it.second});
            }
        } else if (it.first == TokenType::Separator) {
            tokens1.push_back({it.second, it.second});
        } else if (it.first == TokenType::Operator) {
            tokens1.push_back({it.second, it.second});
        }
    }

    // 语法分析
    Syntax syntax = Syntax(productions, terminals, nonTerminals, startSymbol);
    syntax.parse(tokens1);
    return 0;
}
