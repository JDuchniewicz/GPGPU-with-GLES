#ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
#else
    precision mediump float;
#endif

uniform sampler2D texture0;
uniform float scalar;

varying vec2 vTexCoord;

vec4 pack(float value)
{
    if (value == 0.0) return vec4(0);

    float exponent;
    float mantissa;
    vec4 result;
    float sgn;

    sgn = step(0.0, -value);
    value = abs(value);

    exponent = floor(log2(value));
    mantissa = value * pow(2.0, -exponent) - 1.0;
    exponent = exponent + 127.0;
    result = vec4(0);

    result.a = floor(exponent / 2.0);
    exponent = exponent - result.a * 2.0;
    result.a = result.a + 128.0 * sgn;

    result.b = floor(mantissa * 128.0);
    mantissa = mantissa - result.b / 128.0;
    result.b = result.b + exponent * 128.0;

    result.g = floor(mantissa * 32768.0);
    mantissa = mantissa - result.g / 32768.0;

    result.r = floor(mantissa * 8388608.0);

    return result / 255.0;
}

float unpack(vec4 texel)
{
    float exponent;
    float mantissa;
    float value;
    float sgn;

    sgn = -step(128.0, texel.a);
    texel.a += 128.0 * sgn;

    exponent = step(128.0, texel.b);
    texel.b -= exponent * 128.0;
    exponent += 2.0 * texel.a - 127.0;

    mantissa = texel.b * 65536.0 + texel.g * 256.0 + texel.r;
    value = pow(-1.0, sgn) * exp2(exponent) * (1.0 + mantissa * exp2(-23.0));

    return value;
}

void main(void)
{
    vec4 texel1 = texture2D(texture0, vTexCoord);
    float a1 = unpack(texel1 * 255.0);
    gl_FragColor = pack(a1 * scalar);
}

