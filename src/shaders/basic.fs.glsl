#version 330 core

// TODO: La couleur des fragments est donnée à partir de la couleur
//       des vertices passée en entrée.
in vec3 vertexColor;

out vec3 fragColor;

void main()
{
    fragColor = vertexColor;
}
