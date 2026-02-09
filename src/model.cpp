#include "model.hpp"

#include "happly.h"

using namespace gl;

// TODO: Il est fortement recommandé de définir quelques structs
//       pour représenter les attributs.
//       Faire de même pour représenter une vertex, qui est constitué d'attributs.
//       Cela facilitera l'utilisation et rendra votre code plus clair.
//       Un format entrelacé est recommandé (ordonné par vertex au lieu par attribut).
// struct ... { ... };

struct Pos
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct Color
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
};

struct Vertex 
{
    Pos pos;
    Color color;
};

constexpr uint8_t N_VALUES_PER_POS = sizeof(Pos) / sizeof(GLfloat);
constexpr uint8_t N_VALUES_PER_COLOR = sizeof(Color) / sizeof(GLfloat);

void Model::load(const char* path)
{
    // Chargement des données du fichier .ply.
    // Ne modifier pas cette partie.
    happly::PLYData plyIn(path);

    happly::Element& vertex = plyIn.getElement("vertex");
    std::vector<float> positionX = vertex.getProperty<float>("x");
    std::vector<float> positionY = vertex.getProperty<float>("y");
    std::vector<float> positionZ = vertex.getProperty<float>("z");
    
    std::vector<unsigned char> colorRed   = vertex.getProperty<unsigned char>("red");
    std::vector<unsigned char> colorGreen = vertex.getProperty<unsigned char>("green");
    std::vector<unsigned char> colorBlue  = vertex.getProperty<unsigned char>("blue");

    // Tableau de faces, une face est un tableau d'indices.
    // Les faces sont toutes des triangles dans nos modèles (donc 3 indices par face).
    std::vector<std::vector<unsigned int>> facesIndices = plyIn.getFaceIndices<unsigned int>();

    // TODO: Rassemblez les propriétés du fichier .ply pour correspondre au
    //       format de donnée souhaité (celui que vous avez défini dans la struct).
    size_t numVertices = positionX.size();
    std::vector<Vertex> vertices(numVertices);

    for (size_t i = 0; i < numVertices; ++i) {
        vertices[i].pos.x = positionX[i];
        vertices[i].pos.y = positionY[i];
        vertices[i].pos.z = positionZ[i];
        vertices[i].color.r = colorRed[i] / 255.0f;
        vertices[i].color.g = colorGreen[i] / 255.0f;
        vertices[i].color.b = colorBlue[i] / 255.0f;
        vertices[i].color.a = 1.0f;
    }
    
    // TODO: Rassemblez les indices dans un seul tableau contigu.
    std::vector<GLuint> indices;
    for (const auto& face: facesIndices) {
        for (GLuint i : face) {
            indices.push_back(i);
        }
    }
    
    // TODO: Allocation des ressources sur la carte graphique et envoyer les
    //       données traitées dans le vbo et ebo sur la carte graphique.

    // TODO: Créez un vao et spécifiez le format des données dans celui-ci.
    //       N'oubliez pas de lier le ebo avec le vao et de délier le vao
    //       du contexte pour empêcher des modifications sur celui-ci.

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, N_VALUES_PER_POS, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, N_VALUES_PER_COLOR, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) sizeof(Pos));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    
    // TODO: Initialisez count_, qui correspond au nombre d'indices à dessiner.
    count_ = static_cast<GLsizei>(indices.size());
}

Model::~Model()
{
    // TODO: Libérez les ressources allouées.
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
}

void Model::draw()
{
    // TODO: Dessin du modèle.
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

