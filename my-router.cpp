#include <iostream>
#include "Router.h"

int main(int argc, char** arg)
{
    Router myRouter("127.0.0.1", arg[1][0], "Init.txt");
    return 0;
}
