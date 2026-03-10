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
    
    vec3 L = normalize(lightDir);
    vec3 D = normalize(spotDir);
    
    float cosGamma = dot(L, -D);
    float cosDelta = cos(radians(openingAngle));
    
    if (cosGamma > cosDelta) {
        spotFactor = pow(cosGamma, exponent);
    }
    
    return spotFactor;
}

void main()
{
    vec3 N = normalize(attribsIn.normal);
    vec3 V = normalize(-lightsIn.obsPos);
    
    vec3 L_dir = normalize(-lightsIn.dirLightDir);
    vec3 R_dir = reflect(-L_dir, N);
    
    float diff_dir = max(dot(N, L_dir), 0.0);
    float spec_dir = pow(max(dot(V, R_dir), 0.0), mat.shininess);
    if (diff_dir == 0.0) spec_dir = 0.0;
    
    const float LEVELS = 4.0;
    float cel_diff = floor(diff_dir * LEVELS) / LEVELS;
    float cel_spec = floor(spec_dir * LEVELS) / LEVELS;
    
    vec3 totalAmbient  = globalAmbient * mat.ambient + dirLight.ambient * mat.ambient;
    vec3 totalDiffuse  = dirLight.diffuse * mat.diffuse * cel_diff;
    vec3 totalSpecular = dirLight.specular * mat.specular * cel_spec;
    
    for(int i = 0; i < nSpotLights; i++)
    {
        vec3 L_spot = lightsIn.spotLightsDir[i];
        float distanceToLight = length(L_spot);
        L_spot = normalize(L_spot);
        
        vec3 spotDir = lightsIn.spotLightsSpotDir[i];
        
        float spotFactor = computeSpot(spotLights[i].openingAngle, spotLights[i].exponent, spotDir, L_spot, N);
        
        if (spotFactor > 0.0)
        {
            float diff_spot = max(dot(N, L_spot), 0.0);
            vec3 R_spot = reflect(-L_spot, N);
            float spec_spot = pow(max(dot(V, R_spot), 0.0), mat.shininess);
            if (diff_spot == 0.0) spec_spot = 0.0;
            
            float attenuation = 1.0 - smoothstep(7.0, 10.0, distanceToLight);
            
            vec3 spotAmbient  = spotLights[i].ambient * mat.ambient;
            vec3 spotDiffuse  = spotLights[i].diffuse * mat.diffuse * diff_spot * spotFactor;
            vec3 spotSpecular = spotLights[i].specular * mat.specular * spec_spot * spotFactor;
            
            totalAmbient  += spotAmbient * attenuation;
            totalDiffuse  += spotDiffuse * attenuation;
            totalSpecular += spotSpecular * attenuation;
        }
    }

    vec4 texColor = texture(diffuseSampler, attribsIn.texCoords);
    vec3 baseColor = texColor.rgb * attribsIn.color; 

    vec3 color = mat.emission + baseColor * (totalAmbient + totalDiffuse) + totalSpecular;
    
    FragColor = vec4(color, texColor.a);
}
