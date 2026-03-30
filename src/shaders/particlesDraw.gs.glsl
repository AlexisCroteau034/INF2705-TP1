#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in ATTRIB_VS_OUT
{
    vec4 color;
    vec2 size;
    float zOrientation;
} attribIn[];

out ATTRIB_GS_OUT
{
    vec4 color;
    vec2 uv;
} attribOut;

uniform mat4 projection;


void main()
{
    vec4 centerPos = gl_in[0].gl_Position;
    vec2 halfSize = attribIn[0].size * 0.5;
    float angle = attribIn[0].zOrientation;
    float c = cos(angle);
    float s = sin(angle);
    mat2 rot = mat2(c, s, -s, c);

    vec2 offsets[4] = vec2[](
        vec2(-halfSize.x, -halfSize.y),
        vec2( halfSize.x, -halfSize.y),
        vec2(-halfSize.x,  halfSize.y),
        vec2( halfSize.x,  halfSize.y)
    );

    vec2 uvs[4] = vec2[](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );

    for (int i = 0; i < 4; ++i)
    {
        vec2 rotatedOffset = rot * offsets[i];
        vec4 viewPos = centerPos + vec4(rotatedOffset, 0.0, 0.0);
        
        gl_Position = projection * viewPos;
        
        attribOut.color = attribIn[0].color;
        attribOut.uv = uvs[i];
        
        EmitVertex();
    }
    
    EndPrimitive();
}
