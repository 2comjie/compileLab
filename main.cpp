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

std::vector<std::pair<std::string, std::vector<std::string>>> productions = {
    {"E", {"T", "E'"}},
    {"E'", {"+", "T", "E'"}},
    {"E'", {"@"}},  // 使用 "@" 表示空产生式 ε
    {"T", {"F", "T'"}},
    {"T'", {"*", "F", "T'"}},
    {"T'", {"@"}},  // 使用 "@" 表示空产生式 ε
    {"F", {"(", "E", ")"}},
    {"F", {"num"}}};
std::set<std::string> terminals = {"+", "*", "(", ")", "num"};
std::set<std::string> nonTerminals = {"E", "E'", "T", "T'", "F"};
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

    // for (auto it : tokens1) {
    //     std::cout << it.first << " " << it.second << std::endl;
    // }

    // 语法分析
    Syntax syntax = Syntax(productions, terminals, nonTerminals, startSymbol);
    // syntax.displayFirstSets();
    // syntax.displayFollowSets();
    // syntax.displaySelectSets();
    // syntax.displayParseTable();
    syntax.parse(tokens1);
    return 0;
}

// std::stack<std::string> stack;
//         tokens.push_back({"#", "#"});
//         stack.push("#");    // 输入结束符
//         stack.push(start);  // 开始符号
//         size_t idx = 0;

//         bool error = false;
//         while (!stack.empty()) {
//             if (idx == tokens.size()) {
//                 error = true;
//                 std::cerr << "Error: unexpected end of input." << std::endl;
//                 break;
//             }
//             std::string top = stack.top();
//             std::string currentInput = tokens[idx].first;
//             if (top == "#") {  // 栈顶为结束符
//                 if (currentInput != top) {
//                     std::cerr << "Error: unexpected end of input." << std::endl;
//                     error = true;
//                     break;
//                 } else {
//                     break;
//                 }
//             } else if (terminals.find(top) != terminals.end()) {  // 栈顶为终结符
//                 // 如果 top == currentInput
//                 if (top == currentInput) {
//                     // 弹出栈顶元素，currentInput后移
//                     stack.pop();
//                     idx++;
//                 } else {
//                     std::cerr << "Error: unexpected symbol '" << tokens[idx].second << "',expected" << top << std::endl;
//                     // 恐慌模式，如果栈顶的终结符和当前输入符号不匹配，则弹出栈顶的终结符
//                     stack.pop();
//                     error = true;
//                 }
//             } else if (nonTerminals.find(top) != nonTerminals.end()) {  // 栈顶是非终结符

//                 if (parseTable.find({top, currentInput}) == parseTable.end()) {
//                     std::cerr << top << " " << currentInput << std::endl;
//                     exit(1);
//                 }

//                 auto p = parseTable.at({top, currentInput});  // 查预测分析表 M[A,a] = p

//                 if (p.size() == 0) {  // 如果p为空，则检测到错误，根据恐慌模式，忽略输入符号a
//                     std::cerr << "Error: no rule for " << top << " with '" << tokens[idx].second << "'" << std::endl;
//                     idx++;
//                     error = true;
//                 } else if (p[0] == "synch") {  // 如果p是同步词法单元，则弹出栈顶的非终结符，试图继续分析后面的语法部分
//                     std::cout << "synch" << std::endl;
//                     stack.pop();
//                 } else {  // 用p替换栈顶符号
//                     stack.pop();
//                     for (int i = p.size() - 1; i >= 0; --i) {
//                         // 入栈
//                         if (p[i] == "@") {
//                             continue;
//                         } else {
//                             stack.push(p[i]);
//                         }
//                     }
//                 }
//             } else {
//                 std::cerr << "error" << std::endl;
//                 exit(1);
//             }
//         }

//         if (!error) {
//             std::cout << "No Error!" << std::endl;
//             std::cout << idx << std::endl;
//         }

//         return !error;