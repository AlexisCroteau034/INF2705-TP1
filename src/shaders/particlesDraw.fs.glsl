#version 330 core

in ATTRIB_GS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
    vec4 color;
    vec2 uv;
} attribIn;

out vec4 FragColor;

uniform sampler2D textureSampler;

void main()
{
    // TODO: Colorier les particules.
    //       La couleur finale est la couleur de la texture teinté
    //       par la couleur de la particule.
    //       Pour éviter de traiter des fragments invisibles, on discard
    //       les fragments dont le texel possède moins de 0.02 d'opacité.
    // Échantillonner la texture avec les coordonnées UV générées
    vec4 texColor = texture(textureSampler, attribIn.uv);
    
    // Discard des fragments dont le texel a moins de 0.02 d'opacité
    if (texColor.a < 0.02)
    {
        discard;
    }
    
    // La couleur finale est le texel teinté par la couleur de la particule
    FragColor = texColor * attribIn.color;
}
