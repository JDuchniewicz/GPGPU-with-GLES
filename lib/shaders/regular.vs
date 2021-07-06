attribute vec3 position;
attribute vec2 texCoord;

varying highp vec2 vTexCoord;

void main(void)
{
    gl_Position = vec4(position, 1.0);
    vTexCoord = texCoord;
}
