#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int step(float a, float b)
{
    if (b < a)
        return 0;
    return 1;
}

float unpack(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    float exponent;
    float mantissa;
    float value;
    float sgn;
    sgn = -step(128, a);
    a += 128.0 * sgn;
    exponent = step(128, b);
    b -= exponent * 128.0;
    exponent += 2.0 * a - 127.0;
    mantissa = b * 65536.0 + g * 256.0 + r;
    value = powf(-1.0, sgn) * exp2f(exponent) * (1.0 + mantissa * exp2f(-23.0));
    return value;
}

void pack(float value, int* r, int* g, int* b, int* a)
{
    if (value == 0.0)
    {
        *r = 0;
        *g = 0;
        *b = 0;
        *a = 0;
        return;
    }

    float exponent;
    float mantissa;
    float sgn;

    sgn = step(0.0, -value);
    value = fabs(value);

    exponent = floorf(log2f(value));
    mantissa = value * powf(2.0, -exponent) - 1.0;
    exponent = exponent + 127.0;

    *a = floorf(exponent / 2.0);
    exponent = exponent - *a * 2.0;
    *a = *a + 128.0 * sgn;

    *b = floorf(mantissa * 128.0);
    mantissa = mantissa - *b / 128.0;
    *b = *b + exponent * 128.0;

    *g = floorf(mantissa * 32768.0);
    mantissa = mantissa - *g / 32768.0;

    *r = floorf(mantissa * 8388608.0);
}

int main(int argc, char* argv[])
{
    float f = atof(argv[1]);
    unsigned char* p = &f;
    printf("IN: %1.f\n", f);
    printf("r: %d g: %d b: %d, a: %d\n", *p, *(p+1), *(p+2), *(p+3));
    int r, g, b, a;
    pack(f, &r, &g, &b, &a);
    printf("PACK TEST: %1.f\n", f);
    printf("r: %d g: %d b: %d, a: %d\n", r, g, b, a);

    float returned = unpack(*p, *(p+1), *(p+2), *(p+3));
    printf("OUT: %.1f\n", returned);
    return 0;
}
