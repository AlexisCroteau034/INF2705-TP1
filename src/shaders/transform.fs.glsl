#version 330 core

// TODO: La couleur des fragments est donnée à partir de la couleur
//       des vertices passée en entrée.
//       De plus, une variable uniform permet de multiplier la couleur
//       par une autre pour coloriser les fragments.

in vec4 vertexColor;
out vec4 color; 
uniform vec4 colorMod; 

void main()
{
    color = vertexColor * colorMod; 
}
