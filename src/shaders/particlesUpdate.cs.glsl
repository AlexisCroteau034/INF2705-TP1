#version 430 core

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

float rand01()
{
    return fract(sin(dot(vec2(time*100, gl_GlobalInvocationID.x), vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    uint index = gl_GlobalInvocationID.x;
    
    Particle p = dataIn.particles[index];
    
    if (p.timeToLive <= 0.0)
    {
        p.position = emitterPosition;
        p.zOrientation = rand01() * 6.28318530718; 
        p.velocity = (emitterDirection * 0.3) + vec3(0.0, 0.2, 0.0);
        p.color = vec4(0.5, 0.5, 0.5, 0.0);
        p.size = vec2(0.2, 0.2);
        p.maxTimeToLive = 1.5 + (rand01() * 0.5);
        p.timeToLive = p.maxTimeToLive;
    }
    else
    {
        p.timeToLive -= deltaTime;
        p.position += p.velocity * deltaTime;
        p.zOrientation += 0.5 * deltaTime;
        
        float lifeRatio = p.timeToLive / p.maxTimeToLive;
        vec3 rgbColor = mix(vec3(1.0), vec3(0.5), lifeRatio);
        
        float fade = smoothstep(0.0, 0.2, lifeRatio) * (1.0 - smoothstep(0.8, 1.0, lifeRatio));
        p.color = vec4(rgbColor, 0.2 * fade);
        
        float currentSize = mix(0.5, 0.2, lifeRatio);
        p.size = vec2(currentSize, currentSize);
    }
    
    dataOut.particles[index] = p;
}

