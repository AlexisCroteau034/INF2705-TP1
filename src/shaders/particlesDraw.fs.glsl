#version 330 core

in ATTRIB_GS_OUT
{
    vec4 color;
    vec2 uv;
} attribIn;

out vec4 FragColor;

uniform sampler2D textureSampler;

void main()
{
    vec4 texColor = texture(textureSampler, attribIn.uv);
    if (texColor.a < 0.02) discard;
    FragColor = texColor * attribIn.color;
}
