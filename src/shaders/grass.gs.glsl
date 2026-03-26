#version 330 core

// TODO: À remplir
// layout() in;
// layout() out;
layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

// in ATTRIBS_TES_OUT
// {
//     // TODO: Ajouter les attributs si nécessaire
// } attribsIn[];

out ATTRIBS_GS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
    float heightRatio;
} attribsOut;


uniform mat4 mvp;

// Fonction pseudo aléatoire, utiliser le paramètre co pour avoir une valeur différente en sortie
float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // TODO: Générer un brin d'herbe à partir du point en entrée
    
    // En premier lieu, on pourra générer des triangles avec le point comme étant le centre
    // de la base du triangle.
    // Par la suite, on ajoutera un facteur aléatoire de rotation en y [0, 2pi], puis une
    // rotation en x [0, 0.1pi] pour "plier" les brins. Utiliser le facteur aléatoire.
    
    // On pourra aussi faire varié la taille avec la taille de base et la variation [0, 0.04].
    const float baseWidth = 0.05;
    const float varWidth = 0.04;
    
    // Idem avec la hauteur.
    const float baseHeight = 0.4;
    const float varHeight = 0.4;
    
    // Pour finir, émission des différents points. N'oublier pas d'écrire chaque attribut de sortie.
    vec4 center = gl_in[0].gl_Position;
    
    // Facteurs aléatoires basés sur la position
    float r = rand(center.xz);
    float currentWidth = baseWidth + r * varWidth;
    float currentHeight = baseHeight + r * varHeight;
    
    // Rotation aléatoire en Y
    float angle = r * 6.28318; // [0, 2pi]
    mat3 rotY = mat3(
        cos(angle), 0.0, sin(angle),
        0.0, 1.0, 0.0,
        -sin(angle), 0.0, cos(angle)
    );

    // Points de base
    vec3 right = rotY * vec3(currentWidth, 0.0, 0.0);
    vec3 left = rotY * vec3(-currentWidth, 0.0, 0.0);
    
    // Point du haut (légèrement décalé pour simuler la pliure en X demandée)
    vec3 top = rotY * vec3(0.0, currentHeight, currentHeight * 0.2); 

    // Émission du brin d'herbe (Triangle)[cite: 3]
    attribsOut.heightRatio = 0.0; // Base foncée
    gl_Position = mvp * vec4(center.xyz + left, 1.0);
    EmitVertex();
    
    attribsOut.heightRatio = 0.0; // Base foncée
    gl_Position = mvp * vec4(center.xyz + right, 1.0);
    EmitVertex();
    
    attribsOut.heightRatio = 1.0; // Haut clair
    gl_Position = mvp * vec4(center.xyz + top, 1.0);
    EmitVertex();
    
    EndPrimitive();
}
