#include <iostream>
#include "lexer.h"
#include "parser.h"

using namespace std;

int main()
{
    lexer test_lexer("plot.txt");
    test_lexer.analyze();
    test_lexer.print();

    Parser test_parser("plot.txt");
    test_parser.Analyze();
    return 0;
}
