#include "car.hpp"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace gl;
using namespace glm;

    
Car::Car()
: position(0.0f, 0.0f, 0.0f), orientation(0.0f, 0.0f), speed(0.f)
, wheelsRollAngle(0.f), steeringAngle(0.f)
, isHeadlightOn(false), isBraking(false)
, isLeftBlinkerActivated(false), isRightBlinkerActivated(false)
, isBlinkerOn(false), blinkerTimer(0.f)
{}

void Car::loadModels()
{
    frame_.load("../models/frame.ply");
    wheel_.load("../models/wheel.ply");
    blinker_.load("../models/blinker.ply");
    light_.load("../models/light.ply");
}

void Car::update(float deltaTime)
{
    if (isBraking)
    {
        const float LOW_SPEED_THRESHOLD = 0.1f;
        const float BRAKE_APPLIED_SPEED_THRESHOLD = 0.01f;
        const float BRAKING_FORCE = 4.f;
    
        if (fabs(speed) < LOW_SPEED_THRESHOLD)
            speed = 0.f;
            
        if (speed > BRAKE_APPLIED_SPEED_THRESHOLD)
            speed -= BRAKING_FORCE * deltaTime;
        else if (speed < -BRAKE_APPLIED_SPEED_THRESHOLD)
            speed += BRAKING_FORCE * deltaTime;
    }
    
    const float WHEELBASE = 2.7f;
    float angularSpeed = speed * sin(-glm::radians(steeringAngle)) / WHEELBASE;
    orientation.y += angularSpeed * deltaTime;
    
    glm::vec3 positionMod = glm::rotate(glm::mat4(1.0f), orientation.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(-speed, 0.f, 0.f, 1.f);
    position += positionMod * deltaTime;
    
    const float WHEEL_RADIUS = 0.2f;
    wheelsRollAngle += speed / (2.f * M_PI * WHEEL_RADIUS) * deltaTime;
    
    if (wheelsRollAngle > M_PI)
        wheelsRollAngle -= 2.f * M_PI;
    else if (wheelsRollAngle < -M_PI)
        wheelsRollAngle += 2.f * M_PI;
        
    if (isRightBlinkerActivated || isLeftBlinkerActivated)
    {
        const float BLINKER_PERIOD_SEC = 0.5f;
        blinkerTimer += deltaTime;
        if (blinkerTimer > BLINKER_PERIOD_SEC)
        {
            blinkerTimer = 0.f;
            isBlinkerOn = !isBlinkerOn;
        }
    }
    else
    {
        isBlinkerOn = true;
        blinkerTimer = 0.f;
    }  
}

void Car::draw(glm::mat4& projView)
{
    glm::mat4 carModel = glm::mat4(1.0f);
    carModel = glm::translate(carModel, position + glm::vec3(0.0f, 0.0f, -20.0f));
    carModel = glm::rotate(carModel, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f)); 
    carModel = glm::rotate(carModel, orientation.x, glm::vec3(1.0f, 0.0f, 0.0f)); 
    glUniform4f(colorModUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);
    drawWheels(projView, carModel);
    drawFrame(projView, carModel);
    drawHeadlights(projView, carModel);
}
    
void Car::drawFrame(glm::mat4& projView, glm::mat4& carModel)
{
    carModel = glm::translate(carModel, glm::vec3(0.0f, 0.25f, 0.0f));
    glm::mat4 frameMVP = projView * carModel;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(frameMVP));
    frame_.draw();
}

void Car::drawWheel(const glm::mat4& projView, const glm::mat4& carModel, const bool isLeft, const bool isFront)
{
    const float OFFSET = -0.10124f;

    glm::mat4 model = carModel;

    if (isFront) {
        model = glm::rotate(model, glm::radians(-steeringAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    float roll = isLeft ? -wheelsRollAngle : wheelsRollAngle;

    model = glm::rotate(model, roll, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, OFFSET));

    glm::mat4 mvp = projView * model;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    wheel_.draw();
}

void Car::drawWheels(const glm::mat4& projView, const glm::mat4& carModel)
{
    const glm::vec3 WHEEL_POSITIONS[] =
    {
        glm::vec3(-1.29f, 0.245f, -0.57f),
        glm::vec3(-1.29f, 0.245f,  0.57f),
        glm::vec3( 1.4f , 0.245f, -0.57f),
        glm::vec3( 1.4f , 0.245f,  0.57f)
    };

    for (int i = 0; i < 4; ++i) {
        glm::mat4 model = carModel;
        model = glm::translate(model, WHEEL_POSITIONS[i]);

        bool isFront = (i <= 1);
        bool isLeft = (i % 2 != 0);

        if (isLeft) {
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        drawWheel(projView, model, isLeft, isFront);
    }
}

void Car::drawBlinker(const glm::mat4& projView, const glm::mat4& headlightModel, bool isLeftHeadlight)
{
    bool isBlinkerActivated = (isLeftHeadlight  && isLeftBlinkerActivated) ||
                              (!isLeftHeadlight && isRightBlinkerActivated);

    const glm::vec3 ON_COLOR (1.0f, 0.7f , 0.3f );
    const glm::vec3 OFF_COLOR(0.5f, 0.35f, 0.15f);
    
    if (isBlinkerOn && isBlinkerActivated)
    {
        glUniform4f(colorModUniformLocation, ON_COLOR.x, ON_COLOR.y, ON_COLOR.z, 1.0f);
    }
    else
    {
        glUniform4f(colorModUniformLocation, OFF_COLOR.x, OFF_COLOR.y, OFF_COLOR.z, 1.0f);
    }
    glm::mat4 model = glm::translate(headlightModel, glm::vec3(0.0f, 0.0f, -0.06065f));
    glm::mat4 mvp = projView * model;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    blinker_.draw();
}

void Car::drawLight(const glm::mat4& projView, const glm::mat4& headLightModel, bool isFrontHeadlight)
{
    const glm::vec3 FRONT_ON_COLOR (1.0f, 1.0f, 1.0f);
    const glm::vec3 FRONT_OFF_COLOR(0.5f, 0.5f, 0.5f);
    const glm::vec3 REAR_ON_COLOR  (1.0f, 0.1f, 0.1f);
    const glm::vec3 REAR_OFF_COLOR (0.5f, 0.1f, 0.1f);

    const float Z_OFFSET = 0.029;

    glm::vec3 color;

    if (isFrontHeadlight) {
        color = isHeadlightOn ? FRONT_ON_COLOR : FRONT_OFF_COLOR;
    } else {
        color = isBraking ? REAR_ON_COLOR : REAR_OFF_COLOR;
    }

    glm::mat4 model = glm::translate(headLightModel, glm::vec3(0.0f, 0.0f, Z_OFFSET));
    glm::mat4 mvp = projView * model;

    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform4f(colorModUniformLocation, color.r, color.g, color.b, 1.0f);
    light_.draw();
}

void Car::drawHeadlight(const glm::mat4& projView, const glm::mat4& headLightModel, bool isFrontHeadlight, bool isLeftHeadlight) 
{
    glm::mat4 model = headLightModel;

    if (isFrontHeadlight && isLeftHeadlight) {
        model = glm::rotate(model, glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else if (isFrontHeadlight)
    {
        model = glm::rotate(model, glm::radians(-5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    drawLight(projView, model, isFrontHeadlight);
    drawBlinker(projView, model, isLeftHeadlight);
}

void Car::drawHeadlights(const glm::mat4& projView, const glm::mat4& carModel)
{
    const glm::vec3 HEADLIGHT_POSITIONS[] =
    {
        glm::vec3(-1.9650f, 0.38f, -0.45f),
        glm::vec3(-1.9650f, 0.38f,  0.45f),
        glm::vec3( 2.0019f, 0.38f, -0.45f),
        glm::vec3( 2.0019f, 0.38f,  0.45f)
    };

    for (int i = 0; i < 4; ++i) {
        glm::mat4 model = carModel;
        model = glm::translate(model, HEADLIGHT_POSITIONS[i]);

        bool isFrontHeadlight = HEADLIGHT_POSITIONS[i].x < 0;
        bool isLeftHeadlight = HEADLIGHT_POSITIONS[i].z > 0;

        if (isLeftHeadlight) {
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
        }

        drawHeadlight(projView, model, isFrontHeadlight, isLeftHeadlight);
    }
}
