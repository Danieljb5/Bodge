#ifndef __TEST_H
#define __TEST_H

signed int _4test_3add_n_n(signed int a, signed int b);
signed int _4test_3and_n_n(signed int a, signed int b);
signed int _4test_9remainder_n_n(signed int a, signed int b);
void _4test_5print_np(unsigned char* msg);
struct _4test_4Data_
{
    signed int id;
    signed int* data;

};
typedef struct _4test_4Data_ _4test_4Data;
struct _4test_5Data2_
{
    _4test_4Data d;

};
typedef struct _4test_5Data2_ _4test_5Data2;
#endif //__TEST_H