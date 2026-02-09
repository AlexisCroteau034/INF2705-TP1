#version 330 core

// TODO: Définir les entrées et sorties pour donner une position
//       et couleur à chaque vertex.
//       Les vertices sont transformées à l'aide d'une matrice mvp
//       pour les placer à l'écran.

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 color;

out vec4 vertexColor;

uniform mat4 mvp;

void main()
{
    vertexColor = color;
	gl_Position = mvp * vec4(vertex, 1.0);
}
