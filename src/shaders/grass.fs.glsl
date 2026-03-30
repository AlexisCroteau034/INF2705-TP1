#version 330 core

in ATTRIBS_GS_OUT
{
    float heightRatio;
} attribsIn;

out vec4 FragColor;

void main()
{
    const vec3 GRASS_COLOR = vec3(0.6, 0.86, 0.21); 
    const vec3 BASE_COLOR = GRASS_COLOR * 0.3;
    
    vec3 finalColor = mix(BASE_COLOR, GRASS_COLOR, attribsIn.heightRatio);
    FragColor = vec4(finalColor, 1.0);
}
