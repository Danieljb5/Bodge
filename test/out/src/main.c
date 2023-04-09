#include "main.h"

signed int main(signed int argc, unsigned char** argv)
{
    if(argc<=2)
    {
        return 1;

    }
    _4test_3add_n_n(1, 2);
    _4test_3and_n_n(2, 3);
    _4test_9remainder_n_n(3, 2);
    _4test_5print_np("Hello, world!");
    _4test_4Data d;
    d.id = 5;
    _4test_5Data2 d2;
    d2.d.id = 42;

}
