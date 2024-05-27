#ifndef __LEXICAL_H__
#define __LEXICAL_H__

#include <bitset>
#include <cmath>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stack>

#include "util.h"
// 词法分析器
class Lexical {
   private:
    struct ID {
        std::set<long long> tokens;

        ID(std::set<long long> ids) {
            tokens = ids;
        }

        ~ID() {
        }

        bool operator<(const ID& other) const {
            return tokens < other.tokens;
        }
    };

    class Node {
       public:
        ID id;
        std::multimap<char, Node*> edges;
        bool end;
        int type;

        Node(bool end, int type) : id({NODE_COUNT++}), end(end), type(type) {
        }

        Node(ID id, bool end, int type) : id(id), end(end), type(type) {
        }

        ~Node() {
        }

        static long long NODE_COUNT;
    };

    class NFA {
       public:
        Node* start;
        Node* end;

        NFA(Node* start, Node* end) : start(start), end(end) {
        }

        NFA(char c, int type) : start(nullptr), end(nullptr) {
            start = new Node(false, type);
            end = new Node(true, type);
            start->edges.insert({c, end});
        }

        ~NFA() {
        }

       public:
        static NFA* rgexToNFA(const std::string& rgex, int type) {
            auto tokens = Rgex::toRPN(rgex);
            for (auto t : tokens)
                if (!t.op && t.c != '@')
                    charSet.insert(t.c);

            std::stack<NFA*> nfaStack;

            // 表达式运算
            for (int i = 0; i < tokens.size(); ++i) {
                if (tokens[i].op) {
                    if (tokens[i].c == '|') {
                        NFA* a = nfaStack.top();
                        nfaStack.pop();
                        NFA* b = nfaStack.top();
                        nfaStack.pop();
                        nfaStack.push(NFA::nfaOr(a, b));

                        delete a, b;
                    } else if (tokens[i].c == '.') {
                        NFA* b = nfaStack.top();
                        nfaStack.pop();
                        NFA* a = nfaStack.top();
                        nfaStack.pop();
                        nfaStack.push(NFA::nfaDot(a, b));

                        delete a, b;
                    } else if (tokens[i].c == '*') {
                        NFA* a = nfaStack.top();
                        nfaStack.pop();
                        nfaStack.push(NFA::nfaStar(a));

                        delete a;
                    } else {
                    }
                } else {
                    nfaStack.push(new NFA(tokens[i].c, type));  // 为单个字符构建NFA，rgex: a ===> S->a
                }
            }

            return nfaStack.top();
        }

        static NFA* merge(NFA* a, NFA* b) {
            Node* start = new Node(false, a->start->type | b->start->type);
            start->edges.insert({'@', a->start});
            start->edges.insert({'@', b->start});

            return new NFA(start, nullptr);
        }

       private:
        static NFA* nfaOr(NFA* a, NFA* b) {
            a->end->end = false;
            b->end->end = false;

            // 1. 新建一个初始状态和一个终止状态
            Node* s1 = new Node(false, a->start->type | b->start->type);
            Node* s2 = new Node(true, a->end->type | b->end->type);

            // 2. 将s1用空边指向 a.start, b.start
            s1->edges.insert({'@', a->start});
            s1->edges.insert({'@', b->start});

            // 3. 将 a.end, b.end 用空边指向s2
            a->end->edges.insert({'@', s2});
            b->end->edges.insert({'@', s2});

            return new NFA(s1, s2);
        }

        static NFA* nfaStar(NFA* a) {
            a->end->end = false;

            // 1. 新建一个初始状态s1和一个终止状态s2
            Node* s1 = new Node(false, a->end->type);
            Node* s2 = new Node(true, a->end->type);

            // 2. 让s1用空边指向a.start，s2
            s1->edges.insert({'@', a->start});
            s1->edges.insert({'@', s2});

            // 3. 让a.end用空边指向s2, a.start
            a->end->edges.insert({'@', s2});
            a->end->edges.insert({'@', a->start});

            return new NFA(s1, s2);
        }

        static NFA* nfaDot(NFA* a, NFA* b) {
            // 1. 用空边将a的终结状态和b的初始状态连接
            a->end->end = false;
            a->end->edges.insert({'@', b->start});

            return new NFA(a->start, b->end);
        }

       public:
        static std::set<char> charSet;  // 非终结符集
    };

    class DFA {
       public:
        Node* start = nullptr;

       public:
        ~DFA() {
        }

       private:
        DFA(Node* start) : start(start) {
        }

       public:
        static DFA* NFAtoDFA(NFA* nfa) {
            std::queue<std::set<Node*>> workList;
            std::set<std::set<Node*>> visited;
            std::map<ID, Node*> dfaNodes;

            std::set<Node*> startSet = epsilonClosure({nfa->start});
            ID startId = createStateId(startSet);
            Node* startNode = new Node(startId, containsFinalState(startSet), calueType(startSet));
            dfaNodes.insert({startId, startNode});
            workList.push(startSet);

            while (!workList.empty()) {
                std::set<Node*> currentSet = workList.front();
                workList.pop();
                if (visited.find(currentSet) != visited.end()) continue;
                visited.insert(currentSet);

                for (char c : NFA::charSet) {
                    // 1. 去除空边，构建子集
                    std::set<Node*> nextSet = epsilonClosure(move(currentSet, c));
                    if (nextSet.empty()) continue;
                    if (visited.find(nextSet) == visited.end())
                        workList.push(nextSet);

                    ID fromId = createStateId(currentSet);
                    ID toId = createStateId(nextSet);

                    Node* from = dfaNodes[fromId];
                    Node* to = nullptr;
                    if (dfaNodes.find(toId) == dfaNodes.end()) {
                        to = new Node(toId, containsFinalState(nextSet), calueType(nextSet));
                        dfaNodes.insert({toId, to});
                    } else {
                        to = dfaNodes[toId];
                    }

                    // 2. 添加一条DFA的边(from->to)
                    from->edges.insert({c, to});
                }
            }

            return new DFA(startNode);
        }

       private:
        static std::set<Node*> epsilonClosure(const std::set<Node*>& inputSet) {
            std::set<Node*> resultSet;
            std::stack<Node*> stack;

            for (Node* node : inputSet)
                stack.push(node);

            while (!stack.empty()) {
                Node* current = stack.top();
                stack.pop();
                resultSet.insert(current);

                auto range = current->edges.equal_range('@');
                for (auto it = range.first; it != range.second; it++) {
                    Node* n = (*it).second;
                    if (resultSet.find(n) == resultSet.end())
                        stack.push(n);
                }
            }
            return resultSet;
        }

        static bool containsFinalState(const std::set<Node*>& stateSet) {
            for (Node* node : stateSet)
                if (node->end) return true;
            return false;
        }

        static std::set<Node*> move(const std::set<Node*>& inputSet, char input) {
            std::set<Node*> resultSet;
            for (auto node : inputSet) {
                auto range = node->edges.equal_range(input);
                for (auto it = range.first; it != range.second; ++it)
                    resultSet.insert((*it).second);
            }
            return resultSet;
        }

        static ID createStateId(const std::set<Node*>& stateSet) {
            std::set<long long> ids;
            for (auto node : stateSet)
                for (int token : node->id.tokens)
                    ids.insert(token);
            return ID(ids);
        }

        static int calueType(std::set<Node*> stateSet) {
            int type = 0;
            for (auto n : stateSet)
                type |= n->type;
            return type;
        }
    };

   private:
    DFA* dfa;

   public:
    Lexical(std::vector<std::pair<std::string, int>> rgexList) : dfa(nullptr) {
        if (rgexList.size() == 0)
            exit(1);
        NFA* nfa = NFA::rgexToNFA(rgexList[0].first, 1 << rgexList[0].second);
        if (rgexList[0].second >= 32 || rgexList[0].second < 0)
            exit(1);
        for (size_t i = 1; i < rgexList.size(); ++i) {
            if (rgexList[i].second >= 32 || rgexList[i].second < 0)
                exit(1);
            NFA* tmp = NFA::rgexToNFA(rgexList[i].first, 1 << rgexList[i].second);  // type = 1 * 2^ t ==> t = log2(type)
            NFA* merged = NFA::merge(nfa, tmp);
            delete nfa;
            delete tmp;
            nfa = merged;
        }

        dfa = DFA::NFAtoDFA(nfa);
        delete nfa;
    }

    ~Lexical() {
    }

    std::vector<std::pair<int, std::string>> scan(const std::string& code) {
        std::vector<std::pair<int, std::string>> tokens;
        size_t pos = 0;
        while (pos < code.size()) {
            Node* n = dfa->start;
            size_t startPos = pos;
            int type = 0;
            for (size_t i = pos; i < code.size(); ++i) {
                char c = code[i];
                auto it = n->edges.find(c);
                if (it == n->edges.end()) {
                    break;
                } else if ((*it).second->end) {
                    pos = i;
                    type = (*it).second->type;
                }
                n = (*it).second;
            }
            tokens.push_back({log(type & -type) / log(2), code.substr(startPos, ++pos - startPos)});
        }
        return tokens;
    }
};

long long Lexical::Node::NODE_COUNT = 0;
std::set<char> Lexical::NFA::charSet;  // 非终结符集
#endif                                 // __LEXICAL_H__