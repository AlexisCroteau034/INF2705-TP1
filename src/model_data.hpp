#pragma once

// TODO: Compléter les coordonnées de texture.
//       On veut que la texture du sol se répète 5 fois sur chaque côté.
//       Pour la route, on aura une répétition de 2.
//       Pour les coins, aucune répétition n'est nécessaire.

// Le sol se répète 5 fois sur chaque côté (5.0f)
float ground[] =
{
    // Position           // Texture coordinates (U, V)
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   5.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   5.0f, 5.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 5.0f
};

// La route se répète 2 fois sur sa longueur, mais 1 seule fois sur sa largeur
// (Cela évite l'effet "double route")
float street[] =
{
    // Position           // Texture coordinates
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   2.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   2.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f
};

// Le coin ne se répète pas (1.0f)
float streetcorner[] =
{
    // Position           // Texture coordinates
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f, // Coordonnées de texture de 0.0 à 1.0
     0.5f, 0.0f, -0.5f,   1.0f, 0.0f, // pour ne pas répéter la texture.
     0.5f, 0.0f,  0.5f,   1.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f
};

unsigned int planeElements[] =
{
    0, 2, 1,
    0, 3, 2
};
