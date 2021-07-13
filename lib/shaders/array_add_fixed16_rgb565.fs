#ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
#else
    precision mediump float;
#endif

uniform sampler2D texture0;
uniform sampler2D texture1;

// since no bit shift in gles2.0 we precompute it on CPU
uniform int fraction_divider;

varying vec2 vTexCoord;

float fixed_to_float(vec2 inp)
{
    float n = inp.y * 256.0 + inp.x;
    return float(n / float(fraction_divider));
}

// TODO: remap rgb
vec2 float_to_fixed(float inp)
{
    vec2 ret = vec2(0);
    inp = inp * float(fraction_divider);
    ret.x = inp - inp / 256.0;
    ret.y = inp / 256.0;
    return ret;
}

void main(void)
{
    // one texel is 1 16-bit fixed-point value
    vec4 texel1 = texture2D(texture0, vTexCoord);
    vec4 texel2 = texture2D(texture1, vTexCoord);
    float a1 = fixed_to_float(vec2(texel1.r, texel1.g));
    float b1 = fixed_to_float(vec2(texel2.r, texel2.g));
    a1 += b1;
    gl_FragColor = float_to_fixed(a1);
}

