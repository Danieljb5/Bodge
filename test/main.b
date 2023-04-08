using test

main(i32_t argc, char** argv): i32_t
{
    if(argc <= 2)
    {
        return 1;
    }

    test:add(1, 2);
    test:and(2, 3);
    test:remainder(3, 2);

    test:print("Hello, world!");

    test:Data d;
    d.id = 5;

    test:Data2 d2;
    d2.d.id = 42;
}