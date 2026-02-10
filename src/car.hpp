#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include "model.hpp"

class Car
{   
public:
    Car();
    
    void loadModels();
    
    void update(float deltaTime);
    
    void draw(glm::mat4& projView);
    
private:
    void drawFrame(glm::mat4& projView, glm::mat4& carModel);
    
    void drawWheel(const glm::mat4& projView, const glm::mat4& carModel, const bool isRight, const bool isFront);
    void drawWheels(const glm::mat4& projView, const glm::mat4& carModel);
    
    void drawBlinker(const glm::mat4& projView, const glm::mat4& headLightModel, bool isLeftHeadlight);
    void drawLight(const glm::mat4& projView, const glm::mat4& headLightModel, bool isFrontHeadlight);    
    void drawHeadlight(const glm::mat4& projView, const glm::mat4& carModel, bool isFrontHeadlight, bool isLeftHeadlight);
    void drawHeadlights(const glm::mat4& projView, const glm::mat4& carModel);
    
private:    
    Model frame_;
    Model wheel_;
    Model blinker_;
    Model light_;
    
public:
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
    
    GLuint colorModUniformLocation;
    GLuint mvpUniformLocation;
};
