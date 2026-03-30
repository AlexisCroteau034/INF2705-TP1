#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in float aZOrientation;
layout(location = 3) in vec4 aColor;
layout(location = 4) in vec2 aSize;

out ATTRIB_VS_OUT
{
    vec4 color;
    vec2 size;
    float zOrientation;
} attribOut;

uniform mat4 modelView;

void main()
{
    gl_Position = modelView * vec4(aPos, 1.0);
    
    attribOut.color = aColor;
    attribOut.size = aSize;
    attribOut.zOrientation = aZOrientation;
}
