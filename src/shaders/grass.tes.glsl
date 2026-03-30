#version 400 core

layout(triangles, equal_spacing, ccw, point_mode) in;

uniform mat4 mvp;

void main()
{
    vec4 p = gl_TessCoord.x * gl_in[0].gl_Position + 
             gl_TessCoord.y * gl_in[1].gl_Position + 
             gl_TessCoord.z * gl_in[2].gl_Position;
             
    gl_Position = p;
}
