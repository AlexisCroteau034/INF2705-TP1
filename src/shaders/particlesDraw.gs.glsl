#version 330 core

// TODO: À remplir
// layout() in;
// layout() out;
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in ATTRIB_VS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
    vec4 color;
    vec2 size;
    float zOrientation;
} attribIn[];

out ATTRIB_GS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
    vec4 color;
    vec2 uv;
} attribOut;

uniform mat4 projection;


void main()
{

    // TODO: Convertiser les points en entrée en carrées.
    //       Un côté est de la taille particle::size.
    //       Les carrées sont toujours orientés vers la caméra et tourne
    //       sur eux même sur l'axe des z.
    //       Les coordonnées de texture peuvent être généré à partir de la position du point.
    //
    //       Il est important d'écrire chaque attribut de sortie du shader de géometrie,
    //       autrement c'est un comportement non défini.

    // La position du centre est déjà en View Space (calculée dans le VS)
    vec4 centerPos = gl_in[0].gl_Position;
    vec2 halfSize = attribIn[0].size * 0.5;
    float angle = attribIn[0].zOrientation;

    // Matrice de rotation 2D (autour de l'axe Z dans l'espace de la vue)
    float c = cos(angle);
    float s = sin(angle);
    mat2 rot = mat2(c, s, -s, c);

    // Définition des 4 coins du carré (offsets locaux)
    vec2 offsets[4] = vec2[](
        vec2(-halfSize.x, -halfSize.y), // Bas-gauche
        vec2( halfSize.x, -halfSize.y), // Bas-droite
        vec2(-halfSize.x,  halfSize.y), // Haut-gauche
        vec2( halfSize.x,  halfSize.y)  // Haut-droite
    );

    // Coordonnées de texture correspondantes
    vec2 uvs[4] = vec2[](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );

    // Génération des 4 sommets
    for (int i = 0; i < 4; ++i)
    {
        // On applique la rotation à l'offset
        vec2 rotatedOffset = rot * offsets[i];
        
        // On ajoute l'offset tourné à la position centrale.
        // Puisqu'on est en View Space, on ne modifie que X et Y pour rester face caméra.
        vec4 viewPos = centerPos + vec4(rotatedOffset, 0.0, 0.0);
        
        // Projection finale
        gl_Position = projection * viewPos;
        
        // Transfert des données au fragment shader
        attribOut.color = attribIn[0].color;
        attribOut.uv = uvs[i];
        
        EmitVertex();
    }
    
    EndPrimitive();
}
