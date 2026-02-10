#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 color;

out vec4 vertexColor;

uniform mat4 mvp;

void main()
{
    vertexColor = color;
	gl_Position = mvp * vec4(vertex, 1.0);
}
