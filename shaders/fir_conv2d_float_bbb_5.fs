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
    const int kSpan = 5;
    const int spread = kSpan / 2;
    vec4 samp[kSpan * kSpan];

    vec4 value;

    // load the part of the image which will be used for convolution from texture memory
    // unrolled for BBB
	// x is coords of the image sampled, u is the current texel
	// x o o o o
	// o o o o o
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-2) * w, float(-2) * w));
	if ((vTexCoord.x + float(-2) * w) > 1.0 ||
		(vTexCoord.x + float(-2) * w) < 0.0 ||
		(vTexCoord.y + float(-2) * w) > 1.0 ||
		(vTexCoord.y + float(-2) * w) < 0.0)
	{
		samp[0] = vec4(0.0);
	} else
	{
		samp[0] = value;
	}

	// o x o o o
	// o o o o o
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-1) * w, float(-2) * w));
	if ((vTexCoord.x + float(-1) * w) > 1.0 ||
		(vTexCoord.x + float(-1) * w) < 0.0 ||
		(vTexCoord.y + float(-2) * w) > 1.0 ||
		(vTexCoord.y + float(-2) * w) < 0.0)
	{
		samp[1] = vec4(0.0);
	} else
	{
    	samp[1] = value;
	}

	// o o x o o
	// o o o o o
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 0) * w, float(-2) * w));
	if ((vTexCoord.x + float(0) * w) > 1.0 ||
		(vTexCoord.x + float(0) * w) < 0.0 ||
		(vTexCoord.y + float(-2) * w) > 1.0 ||
		(vTexCoord.y + float(-2) * w) < 0.0)
	{
		samp[2] = vec4(0.0);
	} else
	{
    	samp[2] = value;
	}

	// o o o x o
	// o o o o o
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 1) * w, float(-2) * w));
	if ((vTexCoord.x + float(1) * w) > 1.0 ||
		(vTexCoord.x + float(1) * w) < 0.0 ||
		(vTexCoord.y + float(-2) * w) > 1.0 ||
		(vTexCoord.y + float(-2) * w) < 0.0)
	{
		samp[3] = vec4(0.0);
	} else
	{
        samp[3] = value;
    }

	// o o o o x
	// o o o o o
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 2) * w, float(-2) * w));
	if ((vTexCoord.x + float(2) * w) > 1.0 ||
		(vTexCoord.x + float(2) * w) < 0.0 ||
		(vTexCoord.y + float(-2) * w) > 1.0 ||
		(vTexCoord.y + float(-2) * w) < 0.0)
	{
		samp[4] = vec4(0.0);
	} else
	{
        samp[4] = value;
    }

	// o o o o o
	// x o o o o
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-2) * w, float(-1) * w));
	if ((vTexCoord.x + float(-2) * w) > 1.0 ||
		(vTexCoord.x + float(-2) * w) < 0.0 ||
		(vTexCoord.y + float(-1) * w) > 1.0 ||
		(vTexCoord.y + float(-1) * w) < 0.0)
	{
		samp[5] = vec4(0.0);
	} else
	{
        samp[5] = value;
    }

	// o o o o o
	// o x o o o
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-1) * w, float(-1) * w));
	if ((vTexCoord.x + float(-1) * w) > 1.0 ||
		(vTexCoord.x + float(-1) * w) < 0.0 ||
		(vTexCoord.y + float(-1) * w) > 1.0 ||
		(vTexCoord.y + float(-1) * w) < 0.0)
	{
		samp[6] = vec4(0.0);
	} else
	{
        samp[6] = value;
    }

	// o o o o o
	// o o x o o
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 0) * w, float(-1) * w));
	if ((vTexCoord.x + float(0) * w) > 1.0 ||
		(vTexCoord.x + float(0) * w) < 0.0 ||
		(vTexCoord.y + float(-1) * w) > 1.0 ||
		(vTexCoord.y + float(-1) * w) < 0.0)
	{
		samp[7] = vec4(0.0);
	} else
	{
        samp[7] = value;
    }

	// o o o o o
	// o o o x o
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 1) * w, float(-1) * w));
	if ((vTexCoord.x + float(1) * w) > 1.0 ||
		(vTexCoord.x + float(1) * w) < 0.0 ||
		(vTexCoord.y + float(-1) * w) > 1.0 ||
		(vTexCoord.y + float(-1) * w) < 0.0)
	{
		samp[8] = vec4(0.0);
	} else
	{
        samp[8] = value;
    }

	// o o o o o
	// o o o o x
	// o o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 2) * w, float(-1) * w));
	if ((vTexCoord.x + float(2) * w) > 1.0 ||
		(vTexCoord.x + float(2) * w) < 0.0 ||
		(vTexCoord.y + float(-1) * w) > 1.0 ||
		(vTexCoord.y + float(-1) * w) < 0.0)
	{
		samp[9] = vec4(0.0);
	} else
	{
        samp[9] = value;
    }

	// o o o o o
	// o o o o o
	// x o u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-2) * w, float( 0) * w));
	if ((vTexCoord.x + float(-2) * w) > 1.0 ||
		(vTexCoord.x + float(-2) * w) < 0.0 ||
		(vTexCoord.y + float(0) * w) > 1.0 ||
		(vTexCoord.y + float(0) * w) < 0.0)
	{
		samp[10] = vec4(0.0);
	} else
	{
        samp[10] = value;
    }

	// o o o o o
	// o o o o o
	// o x u o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-1) * w, float( 0) * w));
	if ((vTexCoord.x + float(-1) * w) > 1.0 ||
		(vTexCoord.x + float(-1) * w) < 0.0 ||
		(vTexCoord.y + float(0) * w) > 1.0 ||
		(vTexCoord.y + float(0) * w) < 0.0)
	{
		samp[11] = vec4(0.0);
	} else
	{
        samp[11] = value;
    }

	// o o o o o
	// o o o o o
	// o o x o o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 0) * w, float( 0) * w));
	if ((vTexCoord.x + float(0) * w) > 1.0 ||
		(vTexCoord.x + float(0) * w) < 0.0 ||
		(vTexCoord.y + float(0) * w) > 1.0 ||
		(vTexCoord.y + float(0) * w) < 0.0)
	{
		samp[12] = vec4(0.0);
	} else
	{
        samp[12] = value;
    }

	// o o o o o
	// o o o o o
	// o o u x o
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 1) * w, float( 0) * w));
	if ((vTexCoord.x + float(1) * w) > 1.0 ||
		(vTexCoord.x + float(1) * w) < 0.0 ||
		(vTexCoord.y + float(0) * w) > 1.0 ||
		(vTexCoord.y + float(0) * w) < 0.0)
	{
		samp[13] = vec4(0.0);
	} else
	{
        samp[13] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o x
	// o o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 2) * w, float( 0) * w));
	if ((vTexCoord.x + float(2) * w) > 1.0 ||
		(vTexCoord.x + float(2) * w) < 0.0 ||
		(vTexCoord.y + float(0) * w) > 1.0 ||
		(vTexCoord.y + float(0) * w) < 0.0)
	{
		samp[14] = vec4(0.0);
	} else
	{
        samp[14] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// x o o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-2) * w, float( 1) * w));
	if ((vTexCoord.x + float(-2) * w) > 1.0 ||
		(vTexCoord.x + float(-2) * w) < 0.0 ||
		(vTexCoord.y + float(1) * w) > 1.0 ||
		(vTexCoord.y + float(1) * w) < 0.0)
	{
		samp[15] = vec4(0.0);
	} else
	{
        samp[15] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// o x o o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-1) * w, float( 1) * w));
	if ((vTexCoord.x + float(-1) * w) > 1.0 ||
		(vTexCoord.x + float(-1) * w) < 0.0 ||
		(vTexCoord.y + float(1) * w) > 1.0 ||
		(vTexCoord.y + float(1) * w) < 0.0)
	{
		samp[16] = vec4(0.0);
	} else
	{
        samp[16] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// o o x o o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 0) * w, float( 1) * w));
	if ((vTexCoord.x + float(0) * w) > 1.0 ||
		(vTexCoord.x + float(0) * w) < 0.0 ||
		(vTexCoord.y + float(1) * w) > 1.0 ||
		(vTexCoord.y + float(1) * w) < 0.0)
	{
		samp[17] = vec4(0.0);
	} else
	{
        samp[17] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// o o o x o
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 1) * w, float( 1) * w));
	if ((vTexCoord.x + float(1) * w) > 1.0 ||
		(vTexCoord.x + float(1) * w) < 0.0 ||
		(vTexCoord.y + float(1) * w) > 1.0 ||
		(vTexCoord.y + float(1) * w) < 0.0)
	{
		samp[18] = vec4(0.0);
	} else
	{
        samp[18] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// o o o o x
	// o o o o o
    value = texture2D(texture0, vTexCoord + vec2(float( 2) * w, float( 1) * w));
	if ((vTexCoord.x + float(2) * w) > 1.0 ||
		(vTexCoord.x + float(2) * w) < 0.0 ||
		(vTexCoord.y + float(1) * w) > 1.0 ||
		(vTexCoord.y + float(1) * w) < 0.0)
	{
		samp[19] = vec4(0.0);
	} else
	{
        samp[19] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// o o o o o
	// x o o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-2) * w, float( 2) * w));
	if ((vTexCoord.x + float(-2) * w) > 1.0 ||
		(vTexCoord.x + float(-2) * w) < 0.0 ||
		(vTexCoord.y + float(2) * w) > 1.0 ||
		(vTexCoord.y + float(2) * w) < 0.0)
	{
		samp[20] = vec4(0.0);
	} else
	{
        samp[20] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// o o o o o
	// o x o o o
    value = texture2D(texture0, vTexCoord + vec2(float(-1) * w, float( 2) * w));
	if ((vTexCoord.x + float(-1) * w) > 1.0 ||
		(vTexCoord.x + float(-1) * w) < 0.0 ||
		(vTexCoord.y + float(2) * w) > 1.0 ||
		(vTexCoord.y + float(2) * w) < 0.0)
	{
		samp[21] = vec4(0.0);
	} else
	{
        samp[21] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// o o o o o
	// o o x o o
    value = texture2D(texture0, vTexCoord + vec2(float( 0) * w, float( 2) * w));
	if ((vTexCoord.x + float(0) * w) > 1.0 ||
		(vTexCoord.x + float(0) * w) < 0.0 ||
		(vTexCoord.y + float(2) * w) > 1.0 ||
		(vTexCoord.y + float(2) * w) < 0.0)
	{
		samp[22] = vec4(0.0);
	} else
	{
        samp[22] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// o o o o o
	// o o o x o
    value = texture2D(texture0, vTexCoord + vec2(float( 1) * w, float( 2) * w));
	if ((vTexCoord.x + float(1) * w) > 1.0 ||
		(vTexCoord.x + float(1) * w) < 0.0 ||
		(vTexCoord.y + float(2) * w) > 1.0 ||
		(vTexCoord.y + float(2) * w) < 0.0)
	{
		samp[23] = vec4(0.0);
	} else
	{
        samp[23] = value;
    }

	// o o o o o
	// o o o o o
	// o o u o o
	// o o o o o
	// o o o o x
    value = texture2D(texture0, vTexCoord + vec2(float( 2) * w, float( 2) * w));
	if ((vTexCoord.x + float(2) * w) > 1.0 ||
		(vTexCoord.x + float(2) * w) < 0.0 ||
		(vTexCoord.y + float(2) * w) > 1.0 ||
		(vTexCoord.y + float(2) * w) < 0.0)
	{
		samp[24] = vec4(0.0);
	} else
	{
        samp[24] = value;
    }
    float result = 0.0;

    float step = 1.0 / float (1 + kSpan); // how much step through the kernel texture

    // do the convolution (dot product)
    // unrolled for BBB

    // they have to be specified explicitly for SGX (otherwise they are equal to 0.0)
	vec4 a0 = texture2D(texture1, vec2(step * float(0 + 1), step * float(0 + 1)));
	vec4 a1 = texture2D(texture1, vec2(step * float(1 + 1), step * float(0 + 1)));
	vec4 a2 = texture2D(texture1, vec2(step * float(2 + 1), step * float(0 + 1)));
	vec4 a3 = texture2D(texture1, vec2(step * float(3 + 1), step * float(0 + 1)));
	vec4 a4 = texture2D(texture1, vec2(step * float(4 + 1), step * float(0 + 1)));
	vec4 a5 = texture2D(texture1, vec2(step * float(0 + 1), step * float(1 + 1)));
	vec4 a6 = texture2D(texture1, vec2(step * float(1 + 1), step * float(1 + 1)));
	vec4 a7 = texture2D(texture1, vec2(step * float(2 + 1), step * float(1 + 1)));
	vec4 a8 = texture2D(texture1, vec2(step * float(3 + 1), step * float(1 + 1)));
	vec4 a9 = texture2D(texture1, vec2(step * float(4 + 1), step * float(1 + 1)));
	vec4 a10 = texture2D(texture1, vec2(step * float(0 + 1), step * float(2 + 1)));
	vec4 a11 = texture2D(texture1, vec2(step * float(1 + 1), step * float(2 + 1)));
	vec4 a12 = texture2D(texture1, vec2(step * float(2 + 1), step * float(2 + 1)));
	vec4 a13 = texture2D(texture1, vec2(step * float(3 + 1), step * float(2 + 1)));
	vec4 a14 = texture2D(texture1, vec2(step * float(4 + 1), step * float(2 + 1)));
	vec4 a15 = texture2D(texture1, vec2(step * float(0 + 1), step * float(3 + 1)));
	vec4 a16 = texture2D(texture1, vec2(step * float(1 + 1), step * float(3 + 1)));
	vec4 a17 = texture2D(texture1, vec2(step * float(2 + 1), step * float(3 + 1)));
	vec4 a18 = texture2D(texture1, vec2(step * float(3 + 1), step * float(3 + 1)));
	vec4 a19 = texture2D(texture1, vec2(step * float(4 + 1), step * float(3 + 1)));
	vec4 a20 = texture2D(texture1, vec2(step * float(0 + 1), step * float(3 + 1)));
	vec4 a21 = texture2D(texture1, vec2(step * float(1 + 1), step * float(3 + 1)));
	vec4 a22 = texture2D(texture1, vec2(step * float(2 + 1), step * float(3 + 1)));
	vec4 a23 = texture2D(texture1, vec2(step * float(3 + 1), step * float(3 + 1)));
	vec4 a24 = texture2D(texture1, vec2(step * float(4 + 1), step * float(3 + 1)));

    result += unpack(samp[0] * 255.0) * unpack(a0 * 255.0);
    result += unpack(samp[1] * 255.0) * unpack(a1 * 255.0);
    result += unpack(samp[2] * 255.0) * unpack(a2 * 255.0);
    result += unpack(samp[3] * 255.0) * unpack(a3 * 255.0);
    result += unpack(samp[4] * 255.0) * unpack(a4 * 255.0);
    result += unpack(samp[5] * 255.0) * unpack(a5 * 255.0);
    result += unpack(samp[6] * 255.0) * unpack(a6 * 255.0);
    result += unpack(samp[7] * 255.0) * unpack(a7 * 255.0);
    result += unpack(samp[8] * 255.0) * unpack(a8 * 255.0);
    result += unpack(samp[9] * 255.0) * unpack(a9 * 255.0);
    result += unpack(samp[10] * 255.0) * unpack(a10 * 255.0);
    result += unpack(samp[11] * 255.0) * unpack(a11 * 255.0);
    result += unpack(samp[12] * 255.0) * unpack(a12 * 255.0);
    result += unpack(samp[13] * 255.0) * unpack(a13 * 255.0);
    result += unpack(samp[14] * 255.0) * unpack(a14 * 255.0);
    result += unpack(samp[15] * 255.0) * unpack(a15 * 255.0);
    result += unpack(samp[16] * 255.0) * unpack(a16 * 255.0);
    result += unpack(samp[17] * 255.0) * unpack(a17 * 255.0);
    result += unpack(samp[18] * 255.0) * unpack(a18 * 255.0);
    result += unpack(samp[19] * 255.0) * unpack(a19 * 255.0);
    result += unpack(samp[20] * 255.0) * unpack(a20 * 255.0);
    result += unpack(samp[21] * 255.0) * unpack(a21 * 255.0);
    result += unpack(samp[22] * 255.0) * unpack(a22 * 255.0);
    result += unpack(samp[23] * 255.0) * unpack(a23 * 255.0);
    result += unpack(samp[24] * 255.0) * unpack(a24 * 255.0);

    // do the last transformation to float
    gl_FragColor = pack(result);
}
