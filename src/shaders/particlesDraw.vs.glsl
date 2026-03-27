#version 330 core

// TODO: Définisser les inputs nécessaires pour le dessin.
//       Les propriétés des particules ne sont pas tous utiles ici.
layout(location = 0) in vec3 aPos;
layout(location = 1) in float aZOrientation;
layout(location = 3) in vec4 aColor;
layout(location = 4) in vec2 aSize;

out ATTRIB_VS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
    vec4 color;
    vec2 size;
    float zOrientation;
} attribOut;

uniform mat4 modelView;

void main()
{
    // TODO: À remplir
    // Calcul de la position de la particule dans le référentiel de la vue
    gl_Position = modelView * vec4(aPos, 1.0);
    
    // Transfert des attributs nécessaires au shader de géométrie
    attribOut.color = aColor;
    attribOut.size = aSize;
    attribOut.zOrientation = aZOrientation;
}
