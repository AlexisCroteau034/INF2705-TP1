#version 330 core

// TODO: Définir les entrées et sorties pour donner une position
//       et couleur à chaque vertex.

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

out vec3 vertexColor;

void main()
{
    gl_Position = vec4(position, 1.0);
    vertexColor = color;
}
