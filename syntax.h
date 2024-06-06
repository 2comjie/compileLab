#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

class Syntax {
   public:
    Syntax(const std::vector<std::pair<std::string, std::vector<std::string>>>& prods,
           const std::set<std::string>& terms,
           const std::set<std::string>& nonTerms, const std::string& s)
        : productions(prods), terminals(terms), nonTerminals(nonTerms), start(s) {
        constructFirstSet();
        constructFollowSet();
        constructSelectSet();
        constructParseTable();
    }

    void displayFirstSets() const {
        std::cout << "First Sets:" << std::endl;
        for (const auto& nonTerminal : nonTerminals) {
            std::cout << nonTerminal << ": { ";
            for (const auto& symbol : firstSet.at(nonTerminal)) {
                std::cout << symbol << " ";
            }
            std::cout << "}" << std::endl;
        }
    }

    void displayFollowSets() const {
        std::cout << "\nFollow Sets:" << std::endl;
        for (const auto& nonTerminal : nonTerminals) {
            std::cout << nonTerminal << ": { ";
            for (const auto& symbol : followSet.at(nonTerminal)) {
                std::cout << symbol << " ";
            }
            std::cout << "}" << std::endl;
        }
    }

    void displaySelectSets() const {
        std::cout << "\nSelect Sets:" << std::endl;
        for (const auto& entry : selectSet) {
            std::cout << "Select[" << entry.first.first << "->";

            for (int i = 0; i < entry.first.second.size(); ++i) {
                std::cout << entry.first.second[i];
            }
            std::cout << "]={";

            int idx = 0;
            for (const auto& symbol : entry.second) {
                if (idx != entry.second.size() - 1) {
                    std::cout << symbol << ",";
                } else {
                    std::cout << symbol;
                }
                idx++;
            }
            std::cout << "}" << std::endl;
        }
    }

    void displayParseTable() const {
        std::cout << "\nParse Table:" << std::endl;

        // Display table header
        std::cout << std::setw(15) << " ";
        for (const auto& terminal : terminals) {
            std::cout << std::setw(15) << terminal;
        }
        std::cout << std::setw(15) << "#" << std::endl;

        // Display table rows
        for (const auto& nonTerminal : nonTerminals) {
            std::cout << std::setw(15) << nonTerminal;
            for (const auto& terminal : terminals) {
                if (!parseTable.at({nonTerminal, terminal}).empty()) {
                    std::string str;
                    for (auto it : parseTable.at({nonTerminal, terminal}))
                        str += it;
                    std::cout << std::setw(15) << str;
                } else {
                    std::cout << std::setw(15) << " ";
                }
            }
            // Display # column
            if (!parseTable.at({nonTerminal, "#"}).empty()) {
                std::string str;
                for (auto it : parseTable.at({nonTerminal, "#"}))
                    str += it;
                std::cout << std::setw(15) << str;
            } else {
                std::cout << std::setw(15) << " ";
            }
            std::cout << std::endl;
        }
    }

    // 下推自动机
    bool parse(std::vector<std::pair<std::string, std::string>>& tokens) const {
        std::stack<std::string> stk;
        stk.push("#");    // 输入结束符
        stk.push(start);  // 开始符号

        tokens.push_back({"#", "#"});  // 在输入流的结尾添加结束符，便于对比

        size_t index = 0;
        while (!stk.empty()) {
            std::string top = stk.top();
            std::string token = tokens[index].first;

            auto tmp = stk;
            while (!tmp.empty()) {
                std::cout << tmp.top() << " ";
                tmp.pop();
            }
            std::cout << std::endl;

            if (terminals.find(top) != terminals.end() || top == "#") {
                if (top == token) {
                    // 匹配终结符
                    stk.pop();
                    index++;
                } else {
                    std::cout << "Syntax error: unexpected token " << tokens[index].second << ", expected " << top << std::endl;
                    return false;
                }
            } else if (nonTerminals.find(top) != nonTerminals.end()) {
                if (!parseTable.at({top, token}).empty()) {
                    // 使用对应的产生式替换栈顶的非终结符
                    stk.pop();
                    const auto& production = parseTable.at({top, token});
                    if (!(production.size() == 1 && production[0] == "@")) {  // 如果不是产生式@，则逆序加入栈中
                        for (auto it = production.rbegin(); it != production.rend(); ++it) {
                            stk.push(*it);
                        }
                    }
                } else {
                    std::cout << "Syntax error: no production rule for (" << top << ", " << tokens[index].second << ")" << std::endl;
                    // 尝试同步消费输入记号或跳过输入查看同步点
                    bool foundSync = false;
                    while (!stk.empty() && (!parseTable.at({stk.top(), token}).empty() && parseTable.at({stk.top(), token})[0] == "synch")) {
                        stk.pop();
                        foundSync = true;
                    }
                    if (!foundSync) {
                        return false;
                    }
                }
            } else {
                std::cout << "Syntax error: invalid symbol " << top << std::endl;
                return false;
            }
        }

        if (stk.empty() && tokens[index - 1].first == "#") {
            std::cout << "Parsing successful!" << std::endl;
            return true;  // 成功解析
        } else {
            std::cout << "Syntax error: unexpected end of input" << std::endl;
            return false;
        }
    }

   private:
    std::vector<std::pair<std::string, std::vector<std::string>>>
        productions;
    std::set<std::string> terminals;
    std::set<std::string> nonTerminals;
    std::string start;
    std::map<std::string, std::set<std::string>> firstSet;
    std::map<std::string, std::set<std::string>> followSet;
    std::map<std::pair<std::string, std::vector<std::string>>, std::set<std::string>> selectSet;
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> parseTable;  // 预测分析表

    // 构建first集
    void constructFirstSet() {
        for (const auto& nonTerminal : nonTerminals) {
            firstSet[nonTerminal] = {};
        }

        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& prod : productions) {
                const std::string& nonTerminal = prod.first;
                const std::vector<std::string>& rhs = prod.second;
                for (const auto& symbol : rhs) {
                    if (terminals.find(symbol) != terminals.end() || symbol == "@") {
                        if (firstSet[nonTerminal].find(symbol) == firstSet[nonTerminal].end()) {
                            firstSet[nonTerminal].insert(symbol);
                            changed = true;
                        }
                        break;
                    } else {
                        bool hasEpsilon = false;
                        for (const auto& firstSymbol : firstSet[symbol]) {
                            if (firstSymbol != "@") {
                                if (firstSet[nonTerminal].find(firstSymbol) == firstSet[nonTerminal].end()) {
                                    firstSet[nonTerminal].insert(firstSymbol);
                                    changed = true;
                                }
                            } else {
                                hasEpsilon = true;
                            }
                        }
                        if (!hasEpsilon) {
                            break;
                        }
                    }
                }
            }
        }
    }

    // 构建follow集
    void constructFollowSet() {
        for (const auto& nonTerminal : nonTerminals) {
            followSet[nonTerminal] = {};
        }

        followSet[start].insert("#");  // #表示输入结束

        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& prod : productions) {
                const std::string& nonTerminal = prod.first;
                const std::vector<std::string>& rhs = prod.second;
                for (size_t i = 0; i < rhs.size(); ++i) {
                    if (nonTerminals.find(rhs[i]) != nonTerminals.end()) {
                        if (i < rhs.size() - 1) {
                            const std::string& nextSymbol = rhs[i + 1];
                            if (terminals.find(nextSymbol) != terminals.end()) {
                                if (followSet[rhs[i]].find(nextSymbol) == followSet[rhs[i]].end()) {
                                    followSet[rhs[i]].insert(nextSymbol);
                                    changed = true;
                                }
                            } else {
                                bool hasEpsilon = false;
                                for (const auto& firstSymbol : firstSet[nextSymbol]) {
                                    if (firstSymbol != "@") {
                                        if (followSet[rhs[i]].find(firstSymbol) == followSet[rhs[i]].end()) {
                                            followSet[rhs[i]].insert(firstSymbol);
                                            changed = true;
                                        }
                                    } else {
                                        hasEpsilon = true;
                                    }
                                }
                                if (hasEpsilon) {
                                    for (const auto& followSymbol : followSet[nonTerminal]) {
                                        if (followSet[rhs[i]].find(followSymbol) == followSet[rhs[i]].end()) {
                                            followSet[rhs[i]].insert(followSymbol);
                                            changed = true;
                                        }
                                    }
                                }
                            }
                        } else {
                            for (const auto& followSymbol : followSet[nonTerminal]) {
                                if (followSet[rhs[i]].find(followSymbol) == followSet[rhs[i]].end()) {
                                    followSet[rhs[i]].insert(followSymbol);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 构建select集
    void constructSelectSet() {
        for (const auto& prod : productions) {
            // A->a
            // 如果a=>@，则Select(A->a) = First(a)
            const std::string& nonTerminal = prod.first;
            const std::vector<std::string>& rhs = prod.second;

            // 1. 获取产生式A->a First(a)
            bool hasEpsilon = true;
            std::set<std::string> firstOfRHS;
            for (const auto& symbol : rhs) {
                if (terminals.find(symbol) != terminals.end()) {  // 终结符
                    firstOfRHS.insert(symbol);
                    hasEpsilon = false;
                    break;
                } else if (symbol == "@") {
                    firstOfRHS.insert("@");
                } else {  // 非终结符
                    for (const auto& firstSymbol : firstSet[symbol]) {
                        if (firstSymbol != "@") {
                            firstOfRHS.insert(firstSymbol);
                        }
                    }
                    if (firstSet[symbol].find("@") == firstSet[symbol].end()) {
                        hasEpsilon = false;
                        break;
                    }
                }
            }

            if (hasEpsilon) {
                firstOfRHS.insert("@");
            }

            // 2. 如果 A->a, a !-> @
            if (firstOfRHS.find("@") == firstOfRHS.end()) {
                // Select(A->a) = First(a)
                selectSet[prod] = firstOfRHS;
            } else {
                // Select(A->a) = {First(a)-{@}}+{Follow(A)}
                selectSet[prod] = firstOfRHS;
                selectSet[prod].erase("@");
                for (auto it : followSet[nonTerminal]) {
                    selectSet[prod].insert(it);
                }
            }
        }
    }

    // 构建预测分析表
    void constructParseTable() {
        // 1. 初始化预测分析表
        std::set<std::string> tmp = terminals;
        tmp.insert("#");
        for (auto nonTerminal : nonTerminals) {
            for (auto terminal : tmp) {
                parseTable[{nonTerminal, terminal}] = std::vector<std::string>();
            }
        }

        // 2. 构建预测分析表
        // Select[E->TE']={(,i}
        for (auto entry : selectSet) {
            std::string nonTerminal = entry.first.first;
            for (auto terminal : entry.second) {
                // table[E,i] = E->TE`
                if (parseTable[{nonTerminal, terminal}].size() != 0) {
                    std::cout << "不是LL(1)文法" << std::endl;
                    exit(1);
                }
                parseTable[{nonTerminal, terminal}] = entry.first.second;
            }
        }

        // 添加同步词法单元到预测分析表
        for (const auto& nonTerminal : nonTerminals) {
            // 把表中的非终结符的follow集合中的元素并且非空的设置成synch
            for (const auto& terminal : followSet[nonTerminal]) {
                if (parseTable[{nonTerminal, terminal}].size() == 0) {
                    parseTable[{nonTerminal, terminal}] = {"synch"};
                }
            }
        }
    }
};

#endif  // __SYNTAX_H__