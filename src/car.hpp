#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include "model.hpp"
#include "uniform_buffer.hpp"

class EdgeEffect;
class CelShading;

class Car
{   
public:
    Car();
    
    void loadModels();
    
    void update(float deltaTime);
    
    void draw(const glm::mat4& projView, const glm::mat4& view, bool use_outline);

    void drawWindows(const glm::mat4& projView, const glm::mat4& view);
    
private:
    // TODO: Adapter les paramètres des méthodes privée ici au besoin, surtout pour la partie 3.
    void drawFrame(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& carModel, bool use_outline);
    void drawWheels(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& carModel, bool use_outline);
    void drawWheel(const glm::mat4& projView, const glm::mat4& view, glm::mat4 carModel, const bool isLeft, const bool isFront, bool use_outline);
    void drawHeadlights(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& carModel, bool use_outline);
    void drawHeadlight(const glm::mat4& projView, const glm::mat4& view, glm::mat4 headLightModel, bool isFrontHeadlight, bool isLeftHeadlight, bool use_outline);
    void drawLight(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& headLightModel, bool isFrontHeadlight, bool use_outline);
    void drawBlinker(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& headlightModel, bool isLeftHeadlight, bool use_outline);
    
private:    
    Model windows[6];
    Model frame_;
    Model wheel_;
    Model blinker_;
    Model light_;
    
public:
    glm::mat4 carModel;

    EdgeEffect* edgeEffectShader;
    CelShading* celShadingShader;
    UniformBuffer* material;

    glm::vec3 position;
    glm::vec2 orientation;    
    
    float speed;
    float wheelsRollAngle;
    float steeringAngle;
    bool isHeadlightOn;
    bool isBraking;
    bool isLeftBlinkerActivated;
    bool isRightBlinkerActivated;
    
    bool isBlinkerOn;
    float blinkerTimer;
};
