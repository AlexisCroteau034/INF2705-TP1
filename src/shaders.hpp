#include "shader_program.hpp"

#include <glm/glm.hpp>

class EdgeEffect : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint colorULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


class Sky : public ShaderProgram
{
public:
    GLuint mvpULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


class CelShading : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint viewULoc;
    GLuint modelViewULoc;
    GLuint normalULoc;
    
    GLuint nSpotLightsULoc;
    
    GLuint globalAmbientULoc;

public:
    void setMatrices(const glm::mat4& mvp, const glm::mat4& view, const glm::mat4& model);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
    virtual void assignAllUniformBlockIndexes() override;
};

class GrassShader : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint modelViewULoc;

protected:
    void load() override
    {
        name_ = "GrassShader";
        loadShaderSource(GL_VERTEX_SHADER, "./shaders/grass.vs.glsl");
        loadShaderSource(GL_TESS_CONTROL_SHADER, "./shaders/grass.tcs.glsl");
        loadShaderSource(GL_TESS_EVALUATION_SHADER, "./shaders/grass.tes.glsl");
        
        // Nous décommenterons ces deux lignes pour les étapes 4 et 5 !
        loadShaderSource(GL_GEOMETRY_SHADER, "./shaders/grass.gs.glsl");
        loadShaderSource(GL_FRAGMENT_SHADER, "./shaders/grass.fs.glsl");
        
        link();
    }

    void getAllUniformLocations() override
    {
        mvpULoc = glGetUniformLocation(id_, "mvp");
        modelViewULoc = glGetUniformLocation(id_, "modelView");
    }
};
