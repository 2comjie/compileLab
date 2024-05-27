#ifndef __UTIL_H__
#define __UTIL_H__
#include <iostream>
#include <stack>
#include <string>
#include <vector>
class Rgex {
   public:
    struct RgexToken {
        char c;   // 符号
        bool op;  // 是不是正则的运算符
        RgexToken(char c, bool op) : c(c), op(op) {}
    };

    static int priority(char c) {
        if (c == '|')
            return 1;
        else if (c == '.')
            return 2;
        else if (c == '*')
            return 3;
        else
            return -1;
    }

    // 转逆波兰表达式
    static std::vector<RgexToken> toRPN(const std::string& infix) {
        std::stack<char> st;
        std::vector<RgexToken> tokens;

        // 1.先全部转成token
        for (int i = 0; i < infix.length(); ++i) {
            char c = infix[i];
            // 处理转义字符
            if (c == '\\')
                tokens.push_back(RgexToken(infix[++i], false));
            else if (c == '*' || c == '|' || c == '(' || c == ')')
                tokens.push_back(RgexToken(c, true));
            else
                tokens.push_back(RgexToken(c, false));
        }

        // 2.处理缺少'.'运算符
        for (int i = 0; i < tokens.size(); ++i) {
            RgexToken token = tokens[i];
            if (token.op) {
                if (i + 1 < tokens.size() && token.c != '(' && tokens[i + 1].c != ')' && token.c != '|' && tokens[i + 1].c != '|' && tokens[i + 1].c != '*')
                    tokens.insert(tokens.begin() + ++i, RgexToken('.', true));
                else if (i + 1 < tokens.size() && !tokens[i + 1].op && token.c != '(' && token.c != '|')
                    tokens.insert(tokens.begin() + ++i, RgexToken('.', true));
            } else {
                if (i + 1 < tokens.size())
                    if (tokens[i + 1].op && tokens[i + 1].c == '(')
                        tokens.insert(tokens.begin() + ++i, RgexToken('.', true));
                    else if (!tokens[i + 1].op)
                        tokens.insert(tokens.begin() + ++i, RgexToken('.', true));
            }
        }

        // 3. 转后缀表达式
        std::vector<RgexToken> res;

        std::stack<RgexToken> operStack;

        for (int i = 0; i < tokens.size(); ++i) {
            RgexToken token = tokens[i];
            if (!token.op) {
                res.push_back(token);
            } else if (token.c == '(') {
                operStack.push(token);
            } else if (token.c == ')') {
                while (operStack.top().c != '(') {
                    res.push_back(operStack.top());
                    operStack.pop();
                }
                operStack.pop();
            } else {
                while (operStack.size() != 0 && priority(token.c) <= priority(operStack.top().c)) {
                    res.push_back(operStack.top());
                    operStack.pop();
                }
                operStack.push(token);
            }
        }

        while (!operStack.empty()) {
            res.push_back(operStack.top());
            operStack.pop();
        }

        return res;
    }
};

#endif  // __UTIL_H__