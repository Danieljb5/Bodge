using "stdio.h"

public namespace test
{
    add(i32_t a, i32_t b): i32_t
    {
        return a + b;
    }

    and(i32_t a, i32_t b): i32_t
    {
        return a & b;
    }

    remainder(i32_t a, i32_t b): i32_t
    {
        return a % b;
    }

    print(char* msg)
    {   // using namespace 'c' just turns off name mangling
        c:puts(msg);
    }

    group Data
    {
        i32_t id;
        i32_t* data;
    }

    group Data2
    {
        test:Data d;
    }
}
