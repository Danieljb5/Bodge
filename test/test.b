using "stdio.h"
using "stdlib.h"

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

    template <T>
    group DynamicArray
    {
        T* array;
        u64_t size;
        u64_t reserved;

        init()
        {
            this.size = 0;
            this.reserved = 8;
            this.array = c:malloc(this.reserved * c:sizeof(T));
        }

        push_back(T elem)
        {
            if(this.size == this.reserved)
            {
                this.reserved = (this.reserved * 3) / 2 + 8;
                this.array = c:realloc(this.array, this.reserved * c:sizeof(T));
            }
            T* arrayElem = this.array + (this.size * c:sizeof(T));
            *arrayElem = elem;
            this.size++;
        }
    }
}
