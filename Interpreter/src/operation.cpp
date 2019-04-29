#include "operation.h"
#include <iostream>
Operation::Operation(Token tok)
    : t(tok)
{
    std::cout << ".";
}