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
    float n = inp.x * 256.0 + inp.y;
    return float(n / float(fraction_divider));
}

vec2 float_to_fixed(float inp)
{
    vec2 ret;
    inp = inp * float(fraction_divider);
    ret.x = inp / 256.0;
    ret.y = inp - inp / 256.0;
    return ret;
}

void main(void)
{
    // one texel is 2 16-bit values (a, b) (g, r)
    vec4 texel1 = texture2D(texture0, vTexCoord);
    vec4 texel2 = texture2D(texture1, vTexCoord);
    float a1 = fixed_to_float(vec2(texel1.a * 255.0, texel1.b * 255.0));
    float a2 = fixed_to_float(vec2(texel1.g * 255.0, texel1.r * 255.0));
    float b1 = fixed_to_float(vec2(texel2.a * 255.0, texel2.b * 255.0));
    float b2 = fixed_to_float(vec2(texel2.g * 255.0, texel2.r * 255.0));
    a1 += a2;
    b1 += b2;
    vec2 r1 = float_to_fixed(a1);
    vec2 r2 = float_to_fixed(b1);
    //float a1 = unpack(texel1 * 255.0); // need to rescale it before?
    //float a2 = unpack(texel2 * 255.0);
    gl_FragColor = vec4(texel1.a, texel1.b, 0.0, 0.0);
    //gl_FragColor = vec4(float_to_fixed(a1), float_to_fixed(a1));
    //gl_FragColor = vec4(r1, r2);
}

