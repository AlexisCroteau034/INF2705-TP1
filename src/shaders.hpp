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
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};

class ParticleComputeShader : public ShaderProgram
{
public:
    GLuint timeULoc;
    GLuint deltaTimeULoc;
    GLuint emitterPositionULoc;
    GLuint emitterDirectionULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};

class ParticleDrawShader : public ShaderProgram
{
public:
    GLuint projectionULoc;
    GLuint modelViewULoc;
    GLuint textureSamplerULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};
