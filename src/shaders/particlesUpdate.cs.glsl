#version 430 core

// TODO: À remplir
// layout() in;
layout(local_size_x = 64) in;

struct Particle
{
    vec3 position;
    float zOrientation;
    vec3 velocity;
    vec4 color;
    vec2 size;
    float timeToLive;
    float maxTimeToLive;
};

layout(std140, binding = 0) readonly restrict buffer ParticlesInputBlock
{
    Particle particles[];
} dataIn;

layout(std140, binding = 1) writeonly restrict buffer ParticlesOutputBlock
{
    Particle particles[];
} dataOut;

uniform float time;
uniform float deltaTime;
uniform vec3 emitterPosition;
uniform vec3 emitterDirection;

// Random [0, 1]
float rand01()
{
    return fract(sin(dot(vec2(time*100, gl_GlobalInvocationID.x), vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // TODO: Mise à jour des particules. À partir de l'entrée, metter à jour la particule.
    //
    //       Si le temps de vie est à 0 ou en dessous, on procède à l'initialisation:
    //           - La particule commence à la position de l'émetteur.
    //           - Son orientation est aléatoire entre [0, 2pi].
    //           - La particule va dans la direction à une vitesse de 0.3 unité/s.
    //           - La particule monte naturellement vers le haut à une vitesse de 0.2 unité/s.
    //           - La couleur initiale est grise (0.5).
    //           - La taille initiale est de 0.2 unité pour chaque segment.
    //           - Le temps de vie de la particule est entre [1.5, 2.0].
    //
    //      Pour la mise à jour:
    //          - Réduiser le temps de vie.
    //          - Mise à jour de la position par la méthode d'Euler.
    //          - L'orientation a une vitesse angulaire constante de 0.5.
    //          - La couleur devient blanche de façon linéaire selon le temps de vie.
    //          - L'opacité est à 0.2 et a un effet de fade in/out de [0, 0.2] et [0.8, 1].
    //          - La taille augmente de façon linéaire jusqu'à 0.5 unité en fonction du temps de vie.

    uint index = gl_GlobalInvocationID.x;
    
    // Récupérer la particule courante depuis le buffer d'entrée
    Particle p = dataIn.particles[index];
    
    
    // Vérifier si la particule est "morte" et nécessite une réinitialisation
    if (p.timeToLive <= 0.0)
    {
        // Initialisation de la particule
        p.position = emitterPosition;
        
        // Orientation aléatoire entre 0 et 2π
        p.zOrientation = rand01() * 6.28318530718; 
        
        // Direction de l'émetteur (0.3 unité/s) + mouvement naturel vers le haut (0.2 unité/s)
        p.velocity = (emitterDirection * 0.3) + vec3(0.0, 0.2, 0.0);
        
        // Couleur initiale grise (0.5), l'opacité sera gérée dans la mise à jour
        p.color = vec4(0.5, 0.5, 0.5, 0.0);
        
        // Taille initiale de 0.2
        p.size = vec2(0.2, 0.2);
        
        // Temps de vie aléatoire entre 1.5 et 2.0
        p.maxTimeToLive = 1.5 + (rand01() * 0.5);
        p.timeToLive = p.maxTimeToLive;
    }
    else
    {
        // Mise à jour de la particule

        // Réduire le temps de vie
        p.timeToLive -= deltaTime;
        
        // Méthode d'Euler pour la position
        p.position += p.velocity * deltaTime;
        
        // Vitesse angulaire constante
        p.zOrientation += 0.5 * deltaTime;
        
        // Calcul du ratio de vie restant (de 1.0 à la naissance vers 0.0 à la mort)
        float lifeRatio = p.timeToLive / p.maxTimeToLive;
        
        // Interpolation linéaire de la couleur : gris (0.5) vers blanc (1.0)
        vec3 rgbColor = mix(vec3(1.0), vec3(0.5), lifeRatio);
        
        // Effet de fade in/out avec smoothstep (Opacité max à 0.2)
        // smoothstep(0.0, 0.2) gère la fin de vie (fade out)
        // (1.0 - smoothstep(0.8, 1.0)) gère le début de vie (fade in)
        float fade = smoothstep(0.0, 0.2, lifeRatio) * (1.0 - smoothstep(0.8, 1.0, lifeRatio));
        p.color = vec4(rgbColor, 0.2 * fade);
        
        // Augmentation de la taille de façon linéaire de 0.2 à 0.5
        float currentSize = mix(0.5, 0.2, lifeRatio);
        p.size = vec2(currentSize, currentSize);
    }
    
    // Écrire les données mises à jour dans le buffer de sortie
    dataOut.particles[index] = p;
}

