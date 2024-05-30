#ifndef __SYNTAX_H__
#define __SYNTAX_H__
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
class Syntax {
   private:
    struct Production {
        std::string l;
        std::vector<std::string> r;

        bool operator==(const Production& o) const {
            return o.l == l && o.r == r;
        }
    };

    struct Grammer {
       public:
        std::string S;              // 文法开始符号
        std::vector<Production> P;  // 产生式集合
        std::set<std::string> T;    // 终结符集合
        std::set<std::string> N;    // 非终结符集
    };

   private:
    Grammer G;
    std::map<std::string, std::set<std::string>> firstMap;

   public:
    Syntax(std::vector<std::string>& P, std::set<std::string>& N, std::set<std::string>& T, std::string& S) {
        G.N = N;
        for (auto s : P) {
            int i = 0;
            for (int i = 0; i < s.size(); i++) {
                std::istringstream iss(s);
                std::vector<std::string> tokens;
                std::string token;
                while (iss >> token) {
                    tokens.push_back(token);
                }
                Production p;
                p.l = tokens[0];
                for (int i = 1; i < tokens.size(); i++) {
                    p.r.push_back(tokens[i]);
                }
                G.P.push_back(p);
            }
        }
        G.T = T;
        G.S = S;
        initFirstMap();  // 构造first集

        for (auto it : firstMap) {
            std::cout << it.first << " {";
            for (auto f : it.second) {
                std::cout << f << " ";
            }
            std::cout << "}" << std::endl;
        }
    }

    ~Syntax() {
    }

   private:
    void initFirstMap() {
        for (auto n : G.N) {
            firstMap.insert({n, std::set<std::string>()});
        }

        bool done = false;
        while (!done) {
            done = true;
            for (auto symbal : G.N) {
                auto first = getFirst(symbal);
                if (first.size() > firstMap[symbal].size()) {
                    firstMap[symbal] = first;
                    done = false;
                }
            }
        }
    }

    std::set<std::string> getFirst(std::string symbal) {
        std::set<std::string> first = firstMap[symbal];
        for (auto p : G.P) {
            if (p.l == symbal) {
                std::string rFirst = p.r[0];
                if (G.T.find(rFirst) != G.T.end() || rFirst == "@") {
                    first.insert(rFirst);
                } else {
                    if (G.N.find(p.r[0]) != G.N.end()) {
                        for (auto c : firstMap[p.r[0]]) {
                            if (c != "@") {
                                first.insert(c);
                            }
                        }
                    }

                    int k = 0;
                    while (k < p.r.size()) {
                        if (G.N.find(p.r[k]) != G.N.end()) {
                            k++;
                        } else {
                            break;
                        }
                    }
                    if (k != p.r.size())
                        continue;

                    int j = 0;
                    int i = -1;
                    while (j < p.r.size()) {
                        if (firstMap[p.r[j]].find("@") != firstMap[p.r[j]].end()) {
                            i = ++j;
                        } else {
                            break;
                        }
                    }

                    if (i != -1) {
                        for (auto c : firstMap[p.r[i]]) {
                            if (c != "@") {
                                first.insert(c);
                            }
                        }
                    }

                    if (i == k) {
                        first.insert("@");
                    }
                }
            }
        }

        return first;
    }

    // follow集

    // select集

    // 构建预测分析表

    // 构建AST
};

#endif  // __SYNTAX_H__
