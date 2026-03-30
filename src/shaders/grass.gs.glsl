#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

out ATTRIBS_GS_OUT
{
    float heightRatio;
} attribsOut;


uniform mat4 mvp;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    const float baseWidth = 0.05;
    const float varWidth = 0.04;
    const float baseHeight = 0.4;
    const float varHeight = 0.4;
    
    vec4 center = gl_in[0].gl_Position;
    
    float r = rand(center.xz);
    float currentWidth = baseWidth + r * varWidth;
    float currentHeight = baseHeight + r * varHeight;
    
    float angleY = r * 6.28318;
    mat3 rotY = mat3(
        cos(angleY), 0.0, sin(angleY),
        0.0, 1.0, 0.0,
        -sin(angleY), 0.0, cos(angleY)
    );

    float angleX = r * 0.314159;
    mat3 rotX = mat3(
        1.0, 0.0, 0.0,
        0.0, cos(angleX), -sin(angleX),
        0.0, sin(angleX), cos(angleX)
    );

    vec3 right = rotY * vec3(currentWidth, 0.0, 0.0);
    vec3 left = rotY * vec3(-currentWidth, 0.0, 0.0);
    vec3 top = rotY * rotX * vec3(0.0, currentHeight, 0.0); 

    attribsOut.heightRatio = 0.0;
    gl_Position = mvp * vec4(center.xyz + left, 1.0);
    EmitVertex();
    
    gl_Position = mvp * vec4(center.xyz + right, 1.0);
    EmitVertex();
    
    attribsOut.heightRatio = 1.0;
    gl_Position = mvp * vec4(center.xyz + top, 1.0);
    EmitVertex();
    
    EndPrimitive();
}
