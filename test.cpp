#include "syntax.h"

int main(void) {
    std::set<std::string> N = {"E", "E`", "T", "T`", "F"};
    std::set<std::string> T = {"+", "*", "i", "(", ")"};
    std::vector<std::string> P = {
        "E T E`",
        "E` + T E`",
        "E` @",
        "T F T`",
        "T` * F T`",
        "T` @",
        "F ( E )",
        "F i"};
    std::string S = "E";
    Syntax sy = Syntax(P, N, T, S);
    return 0;
}