#include "test.h"

#include "stdio.h"
#include "stdlib.h"
signed int _4test_3add_n_n(signed int a, signed int b)
{
    return a+b;

}

signed int _4test_3and_n_n(signed int a, signed int b)
{
    return a&b;

}

signed int _4test_9remainder_n_n(signed int a, signed int b)
{
    return a%b;

}

void _4test_5print_np(unsigned char* msg)
{
    puts(msg);

}
