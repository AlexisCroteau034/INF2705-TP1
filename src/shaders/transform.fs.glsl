#version 330 core

in vec4 vertexColor;
out vec4 color; 
uniform vec4 colorMod; 

void main()
{
    color = vertexColor * colorMod; 
}
