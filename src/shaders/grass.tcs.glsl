#version 400 core

// TODO: À remplir
//layout() out;
layout(vertices = 3) out;

uniform mat4 modelView;

float getTessLevel(vec3 p1, vec3 p2) {
    const float MIN_TESS = 2.0;
    const float MAX_TESS = 32.0;
    const float MIN_DIST = 10.0;
    const float MAX_DIST = 40.0;

    // Point milieu de l'arête
    vec3 midPoint = (p1 + p2) / 2.0;
    
    // Position dans l'espace de la caméra
    vec4 viewPos = modelView * vec4(midPoint, 1.0);
    float dist = abs(viewPos.z);
    
    // Interpolation linéaire
    float factor = clamp((dist - MIN_DIST) / (MAX_DIST - MIN_DIST), 0.0, 1.0);
    return mix(MAX_TESS, MIN_TESS, factor);
}

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        vec3 v0 = gl_in[0].gl_Position.xyz;
        vec3 v1 = gl_in[1].gl_Position.xyz;
        vec3 v2 = gl_in[2].gl_Position.xyz;
        
        // Calcul du niveau pour chaque arête extérieure
        // L'arête 0 relie v1 et v2
        // L'arête 1 relie v2 et v0
        // L'arête 2 relie v0 et v1
        float tess0 = getTessLevel(v1, v2);
        float tess1 = getTessLevel(v2, v0);
        float tess2 = getTessLevel(v0, v1);

        gl_TessLevelOuter[0] = tess0;
        gl_TessLevelOuter[1] = tess1;
        gl_TessLevelOuter[2] = tess2;
        
        // Le niveau intérieur est le maximum des niveaux extérieurs
        gl_TessLevelInner[0] = max(max(tess0, tess1), tess2);
    }
}
