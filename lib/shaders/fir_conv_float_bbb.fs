#ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
#else
    precision mediump float;
#endif

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float w; // inverse dimension of the texture - necessary to not overflow floats

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
    const int kSpan = 3;
    const int spread = kSpan / 2;
    vec4 samp[kSpan * kSpan];

    vec4 value;

    // load the part of the image which will be used for convolution from texture memory
    // unrolled for BBB
	// x is coords of the image sampled, u is the current texel
	// x o o
	// o u o
	// o o o
	if ((vTexCoord.x + float(-1) * w) > 1.0 ||
		(vTexCoord.x + float(-1) * w) < 0.0 ||
		(vTexCoord.y + float(-1) * w) > 1.0 ||
		(vTexCoord.y + float(-1) * w) < 0.0)
	{
		value = vec4(0.0);
	} else
	{
		value = texture2D(texture0, vTexCoord + vec2(float(-1) * w, float(-1) * w));
	}
	samp[0] = value;

	// o x o
	// o u o
	// o o o
	if ((vTexCoord.x + float(0) * w) > 1.0 ||
		(vTexCoord.x + float(0) * w) < 0.0 ||
		(vTexCoord.y + float(-1) * w) > 1.0 ||
		(vTexCoord.y + float(-1) * w) < 0.0)
	{
		value = vec4(0.0);
	} else
	{
    	value = texture2D(texture0, vTexCoord + vec2(float( 0) * w, float(-1) * w));
	}
	samp[1] = value;

	// o o x
	// o u o
	// o o o
	if ((vTexCoord.x + float(1) * w) > 1.0 ||
		(vTexCoord.x + float(1) * w) < 0.0 ||
		(vTexCoord.y + float(-1) * w) > 1.0 ||
		(vTexCoord.y + float(-1) * w) < 0.0)
	{
		value = vec4(0.0);
	} else
	{
		value = texture2D(texture0, vTexCoord + vec2(float( 1) * w, float(-1) * w));
	}
	samp[2] = value;

	// o o o
	// x u o
	// o o o
	if ((vTexCoord.x + float(-1) * w) > 1.0 ||
		(vTexCoord.x + float(-1) * w) < 0.0 ||
		(vTexCoord.y + float(0) * w) > 1.0 ||
		(vTexCoord.y + float(0) * w) < 0.0)
	{
		value = vec4(0.0);
	} else
	{
        value = texture2D(texture0, vTexCoord + vec2(float(-1) * w, float( 0) * w));
    }
    samp[3] = value;

	// o o o
	// o x o
	// o o o
	if ((vTexCoord.x + float(0) * w) > 1.0 ||
		(vTexCoord.x + float(0) * w) < 0.0 ||
		(vTexCoord.y + float(0) * w) > 1.0 ||
		(vTexCoord.y + float(0) * w) < 0.0)
	{
		value = vec4(0.0);
	} else
	{
        value = texture2D(texture0, vTexCoord + vec2(float( 0) * w, float( 0) * w));
    }
    samp[4] = value;

	// o o o
	// o u x
	// o o o
	if ((vTexCoord.x + float(1) * w) > 1.0 ||
		(vTexCoord.x + float(1) * w) < 0.0 ||
		(vTexCoord.y + float(0) * w) > 1.0 ||
		(vTexCoord.y + float(0) * w) < 0.0)
	{
		value = vec4(0.0);
	} else
	{
        value = texture2D(texture0, vTexCoord + vec2(float( 1) * w, float( 0) * w));
    }
    samp[5] = value;

	// o o o
	// o u o
	// x o o
	if ((vTexCoord.x + float(-1) * w) > 1.0 ||
		(vTexCoord.x + float(-1) * w) < 0.0 ||
		(vTexCoord.y + float(1) * w) > 1.0 ||
		(vTexCoord.y + float(1) * w) < 0.0)
	{
		value = vec4(0.0);
	} else
	{
        value = texture2D(texture0, vTexCoord + vec2(float(-1) * w, float( 1) * w));
    }
    samp[6] = value;

	// o o o
	// o u o
	// o x o
	if ((vTexCoord.x + float(0) * w) > 1.0 ||
		(vTexCoord.x + float(0) * w) < 0.0 ||
		(vTexCoord.y + float(1) * w) > 1.0 ||
		(vTexCoord.y + float(1) * w) < 0.0)
	{
		value = vec4(0.0);
	} else
	{
        value = texture2D(texture0, vTexCoord + vec2(float( 0) * w, float( 1) * w));
    }
    samp[7] = value;

	// o o o
	// o u o
	// o o x
	if ((vTexCoord.x + float(1) * w) > 1.0 ||
		(vTexCoord.x + float(1) * w) < 0.0 ||
		(vTexCoord.y + float(1) * w) > 1.0 ||
		(vTexCoord.y + float(1) * w) < 0.0)
	{
		value = vec4(0.0);
	} else
	{
        value = texture2D(texture0, vTexCoord + vec2(float( 1) * w, float( 1) * w));
    }
    samp[8] = value;

    float result = 0.0;

    float step = 1.0 / float (1 + kSpan); // how much step through the kernel texture

    // do the convolution (dot product)
    // unrolled for BBB
    result += unpack(samp[0] * 255.0) * unpack(texture2D(texture1, vec2(step * float(0 + 1), step * float(0 + 1))) * 255.0);
    result += unpack(samp[1] * 255.0) * unpack(texture2D(texture1, vec2(step * float(1 + 1), step * float(0 + 1))) * 255.0);
    result += unpack(samp[2] * 255.0) * unpack(texture2D(texture1, vec2(step * float(2 + 1), step * float(0 + 1))) * 255.0);
    result += unpack(samp[3] * 255.0) * unpack(texture2D(texture1, vec2(step * float(0 + 1), step * float(1 + 1))) * 255.0);
    result += unpack(samp[4] * 255.0) * unpack(texture2D(texture1, vec2(step * float(1 + 1), step * float(1 + 1))) * 255.0);
    result += unpack(samp[5] * 255.0) * unpack(texture2D(texture1, vec2(step * float(2 + 1), step * float(1 + 1))) * 255.0);
    result += unpack(samp[6] * 255.0) * unpack(texture2D(texture1, vec2(step * float(0 + 1), step * float(2 + 1))) * 255.0);
    result += unpack(samp[7] * 255.0) * unpack(texture2D(texture1, vec2(step * float(1 + 1), step * float(2 + 1))) * 255.0);
    result += unpack(samp[8] * 255.0) * unpack(texture2D(texture1, vec2(step * float(2 + 1), step * float(2 + 1))) * 255.0);

    // do the last transformation to float
    gl_FragColor = pack(result);
}


