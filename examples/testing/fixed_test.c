#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const int fraction_divider = 1 << 5;

float fixed_to_float(int x, int y)
{
    float n = x * 256.0 + y;
    return (float)(n / (float)fraction_divider);
}

void float_to_fixed(float f, unsigned char* x, unsigned char* y)
{
    printf("%.1f\n", f);
    f = f * (float)fraction_divider;
    printf("%.1f\n", f);
    *x = roundf(f - f / 256.0);
    printf("%.1f\n", f - f / 256.0);
    *y = roundf(f / 256.0);
}

int main(int argc, char* argv[])
{
    u_int16_t i = atoi(argv[1]);
    u_int16_t i2 = atoi(argv[2]);
    unsigned char* p = &i;
    unsigned char* p2 = &i2;
    printf("IN: %d %d\n", i, i2);
    printf("r: %d g: %d b: %d, a: %d\n", *p, *(p+1), *p2, *(p2+1));
    unsigned char r, g, b, a;

    float converted1 = fixed_to_float(i / 256, i - i / 256);
    float converted2 = fixed_to_float(i2 / 256, i2 - i2 / 256);
    printf("fixed_to_float TEST: %d %d\n", i, i2);

    printf("converted: %.1f %.1f\n", converted1, converted2);

    float_to_fixed(converted1, &r, &g);
    float_to_fixed(converted2, &b, &a);
    printf("r: %d g: %d b: %d, a: %d\n", r, g, b, a);
    return 0;
}
