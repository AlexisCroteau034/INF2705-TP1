#version 330 core

#define MAX_SPOT_LIGHTS 12

in ATTRIBS_VS_OUT
{
    vec2 texCoords;
    vec3 normal;
    vec3 color;
} attribsIn;

in LIGHTS_VS_OUT
{
    vec3 obsPos;
    vec3 dirLightDir;
    
    vec3 spotLightsDir[MAX_SPOT_LIGHTS];
    vec3 spotLightsSpotDir[MAX_SPOT_LIGHTS];
} lightsIn;


struct Material
{
    vec3 emission;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirectionalLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec3 direction;
};

struct SpotLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec3 position;
    vec3 direction;
    float exponent;
    float openingAngle;
};

uniform int nSpotLights;

uniform vec3 globalAmbient;

layout (std140) uniform MaterialBlock
{
    Material mat;
};

layout (std140) uniform LightingBlock
{
    DirectionalLight dirLight;
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

uniform sampler2D diffuseSampler;

out vec4 FragColor;


float computeSpot(in float openingAngle, in float exponent, in vec3 spotDir, in vec3 lightDir, in vec3 normal)
{
    float spotFactor = 0.0;
    
    // TODO: Calcul de spotlight, l'algorithme classique d'OpenGL vu en classe (voir annexe).
    // Calcul de spotlight, l'algorithme classique d'OpenGL vu en classe.
    float cosAngle = dot(normalize(lightDir), -normalize(spotDir));
    float cosCutoff = cos(radians(openingAngle / 2.0));

    if (cosAngle > cosCutoff) {
        spotFactor = pow(cosAngle, exponent);
    }
    
    return spotFactor;
}

void main()
{
    // TODO: Calcul d'illumination
    // Calcul d'illumination
    vec3 N = normalize(attribsIn.normal);
    vec3 V = normalize(-lightsIn.obsPos);
    vec4 texColor = texture(diffuseSampler, attribsIn.texCoords);

    // Propriétés du matériau basées sur la texture
    vec3 mat_diffuse = mat.diffuse * texColor.rgb;
    vec3 mat_ambient = mat.ambient * texColor.rgb;

    // Terme ambiant global et émissif
    vec3 final_color = mat.emission + globalAmbient * mat_ambient;

    // Directional light
    vec3 L_dir = normalize(lightsIn.dirLightDir);
    float diff_dir = max(dot(N, L_dir), 0.0);
    
    // TODO: Seulement la lumière directionnel à l'effet de cel-shading, sur la composante diffuse et spéculaire
    const float LEVELS = 4;
    // Seulement la lumière directionnelle a l'effet de cel-shading, sur la composante diffuse et spéculaire
    const float LEVELS = 4.0;
    float cel_diff = floor(diff_dir * LEVELS) / (LEVELS - 1.0);
    
    vec3 H_dir = normalize(L_dir + V);
    float spec_dir = pow(max(dot(N, H_dir), 0.0), mat.shininess);
    float cel_spec = step(0.8, spec_dir); // Un spéculaire plus net pour le cel-shading

    final_color += dirLight.ambient * mat_ambient;
    final_color += dirLight.diffuse * mat_diffuse * cel_diff;
    final_color += dirLight.specular * mat.specular * cel_spec;
        
    // Spot light
    
    for(int i = 0; i < nSpotLights; i++)
    {
        // TODO: Calcul des spotlights
    
        // Utiliser un facteur d'atténuation. On peut utiliser smoothstep avec la distance
        // entre la surface illuminé et la source de lumière. Il devrait y avoir un effet de blending
        // entre 7 et 10 unitées.
        // Le facteur impacte la composante diffuse et spéculaire.
        // Calcul des spotlights
        vec3 L_spot = normalize(lightsIn.spotLightsDir[i]);
        vec3 D_spot = normalize(lightsIn.spotLightsSpotDir[i]);
        float dist = length(lightsIn.spotLightsDir[i]);

        float spotFactor = computeSpot(spotLights[i].openingAngle, spotLights[i].exponent, D_spot, -L_spot, N);
        
        if (spotFactor > 0.0) {
            float attenuation = smoothstep(10.0, 7.0, dist);

            float diff_spot = max(dot(N, L_spot), 0.0);
            vec3 H_spot = normalize(L_spot + V);
            float spec_spot = pow(max(dot(N, H_spot), 0.0), mat.shininess);

            vec3 spotIllumination = spotLights[i].ambient * mat_ambient;
            spotIllumination += (spotLights[i].diffuse * mat_diffuse * diff_spot +
                               spotLights[i].specular * mat.specular * spec_spot) * spotFactor * attenuation;
            final_color += spotIllumination;
        }
    }

    vec4 texColor = texture(diffuseSampler, attribsIn.texCoords);
    // Pour la Partie 1, on veut afficher seulement la texture.
    // La multiplication par attribsIn.color fait en sorte que les modèles avec des couleurs de sommets noires (comme les roues) apparaissent noirs.
    vec3 color = mat.emission + texColor.rgb * mat.diffuse;
    
    //color += normal/2.0 + vec3(0.5); // DEBUG: Show normals
    FragColor = vec4(color, texColor.a);
    FragColor = vec4(final_color, texColor.a);
}
