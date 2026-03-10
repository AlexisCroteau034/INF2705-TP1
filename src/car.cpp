#include "car.hpp"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace gl;
using namespace glm;

#include <map>

#include "shaders.hpp"
#include "uniform_buffer.hpp"

struct Material
{
    glm::vec4 emission; // vec3, but padded
    glm::vec4 ambient;  // vec3, but padded
    glm::vec4 diffuse;  // vec3, but padded
    glm::vec3 specular;
    GLfloat shininess;
};

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
    const char* WINDOW_MODEL_PATHES[] = 
    {
        "../models/window.f.ply",
        "../models/window.r.ply",
        "../models/window.fl.ply",
        "../models/window.fr.ply",
        "../models/window.rl.ply",
        "../models/window.rr.ply"
    };
    for (unsigned int i = 0; i < 6; ++i)
    {
        windows[i].load(WINDOW_MODEL_PATHES[i]);
    }
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
    carModel = glm::mat4(1.0f);
    carModel = glm::translate(carModel, position);
    carModel = glm::rotate(carModel, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Car::draw(const glm::mat4& projView, const glm::mat4& view, bool use_outline)
{
    glm::mat4 carModel = glm::mat4(1.0f);
    carModel = glm::translate(carModel, position + glm::vec3(0.0f, 0.0f, -20.0f));
    carModel = glm::rotate(carModel, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f)); 
    carModel = glm::rotate(carModel, orientation.x, glm::vec3(1.0f, 0.0f, 0.0f)); 
    drawWheels(projView, view, carModel, use_outline);
    drawFrame(projView, view, carModel, use_outline);
    drawHeadlights(projView, view, carModel, use_outline);
}
    
void Car::drawFrame(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& carModel, bool use_outline)
{
    glm::mat4 model = glm::translate(carModel, glm::vec3(0.0f, 0.25f, 0.0f));
    glm::mat4 frameMVP = projView * model;
    if (use_outline) {
        glUniformMatrix4fv(edgeEffectShader->mvpULoc, 1, GL_FALSE, glm::value_ptr(frameMVP));
    } else {
        celShadingShader->setMatrices(frameMVP, view, model);
    }
    frame_.draw();
}

void Car::drawWheel(const glm::mat4& projView, const glm::mat4& view, glm::mat4 carModel, const bool isLeft, const bool isFront, bool use_outline)
{
    const float OFFSET = -0.10124f;

    if (isFront) {
        carModel = glm::rotate(carModel, glm::radians(-steeringAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    float roll = isLeft ? -wheelsRollAngle : wheelsRollAngle;

    carModel = glm::rotate(carModel, roll, glm::vec3(0.0f, 0.0f, 1.0f));
    carModel = glm::translate(carModel, glm::vec3(0.0f, 0.0f, OFFSET));

    glm::mat4 mvp = projView * carModel;
    if (use_outline) {
        glUniformMatrix4fv(edgeEffectShader->mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));
    } else {
        celShadingShader->setMatrices(mvp, view, carModel);
    }
    wheel_.draw();
}

void Car::drawWheels(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& carModel, bool use_outline)
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

        drawWheel(projView, view, model, isLeft, isFront, use_outline);
    }
}

void Car::drawBlinker(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& headlightModel, bool isLeftHeadlight, bool use_outline)
{
    glm::mat4 model = glm::translate(headlightModel, glm::vec3(0.0f, 0.0f, -0.06065f));
    glm::mat4 mvp = projView * model;

    if (use_outline) {
        glUniformMatrix4fv(edgeEffectShader->mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));
        blinker_.draw();
        return;
    }

    bool isBlinkerActivated = (isLeftHeadlight  && isLeftBlinkerActivated) ||
                              (!isLeftHeadlight && isRightBlinkerActivated);

    const glm::vec3 ON_COLOR (1.0f, 0.7f , 0.3f );
    const glm::vec3 OFF_COLOR (0.5f, 0.35f, 0.15f);
    Material blinkerMat = 
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {OFF_COLOR, 0.0f},
        {OFF_COLOR, 0.0f},
        {OFF_COLOR},
        10.0f
    };

    if (isBlinkerOn && isBlinkerActivated)
    {
        blinkerMat.emission = glm::vec4(ON_COLOR, 0.0f);
    }
    material->updateData(&blinkerMat, 0, sizeof(Material));

    celShadingShader->setMatrices(mvp, view, model);
    blinker_.draw();
}

void Car::drawLight(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& headLightModel, bool isFrontHeadlight, bool use_outline)
{
    glm::mat4 model = glm::translate(headLightModel, glm::vec3(0.0f, 0.0f, 0.029));
    glm::mat4 mvp = projView * model;

    if (use_outline) {
        glUniformMatrix4fv(edgeEffectShader->mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));
        light_.draw();
        return;
    }

    const glm::vec3 FRONT_ON_COLOR (1.0f, 1.0f, 1.0f);
    const glm::vec3 FRONT_OFF_COLOR(0.5f, 0.5f, 0.5f);
    const glm::vec3 REAR_ON_COLOR  (1.0f, 0.1f, 0.1f);
    const glm::vec3 REAR_OFF_COLOR (0.5f, 0.1f, 0.1f);

    const float Z_OFFSET = 0.029;

    Material lightFrontMat = 
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {FRONT_OFF_COLOR, 0.0f},
        {FRONT_OFF_COLOR, 0.0f},
        {FRONT_OFF_COLOR},
        10.0f
    };
    
    Material lightRearMat = 
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {REAR_OFF_COLOR, 0.0f},
        {REAR_OFF_COLOR, 0.0f},
        {REAR_OFF_COLOR},
        10.0f
    };

    if (isFrontHeadlight) {
        if (isHeadlightOn) {
            lightFrontMat.emission = glm::vec4(FRONT_ON_COLOR, 0.0f);
        }
        material->updateData(&lightFrontMat, 0, sizeof(Material));
    } else {
        if (isBraking) {
            lightRearMat.emission = glm::vec4(REAR_ON_COLOR, 0.0f);
        }
        material->updateData(&lightRearMat, 0, sizeof(Material));
    }

    celShadingShader->setMatrices(mvp, view, model);
    light_.draw();
}

void Car::drawHeadlight(const glm::mat4& projView, const glm::mat4& view, glm::mat4 headLightModel, bool isFrontHeadlight, bool isLeftHeadlight, bool use_outline) 
{
    if (isFrontHeadlight && isLeftHeadlight) {
        headLightModel = glm::rotate(headLightModel, glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else if (isFrontHeadlight)
    {
        headLightModel = glm::rotate(headLightModel, glm::radians(-5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    drawLight(projView, view, headLightModel, isFrontHeadlight, use_outline);
    drawBlinker(projView, view, headLightModel, isLeftHeadlight, use_outline);
}

void Car::drawHeadlights(const glm::mat4& projView, const glm::mat4& view, const glm::mat4& carModel, bool use_outline)
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

        drawHeadlight(projView, view, model, isFrontHeadlight, isLeftHeadlight, use_outline);
    }
}

void Car::drawWindows(const glm::mat4& projView, const glm::mat4& view)
{
    const glm::vec3 WINDOW_POSITION[] =
    {
        glm::vec3(-0.813, 0.755, 0.0),
        glm::vec3(1.092, 0.761, 0.0),
        glm::vec3(-0.3412, 0.757, 0.51),
        glm::vec3(-0.3412, 0.757, -0.51),
        glm::vec3(0.643, 0.756, 0.508),
        glm::vec3(0.643, 0.756, -0.508)
    };
    
    // TODO: À ajouter et compléter.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE); // Pour que les vitres soient visibles des deux côtés

    // On recrée la matrice de modèle de la voiture comme dans Car::draw pour être cohérent
    glm::mat4 carDrawModel = glm::mat4(1.0f);
    carDrawModel = glm::translate(carDrawModel, position + glm::vec3(0.0f, 0.0f, -20.0f));
    carDrawModel = glm::rotate(carDrawModel, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f)); 
    carDrawModel = glm::rotate(carDrawModel, orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));

    std::map<float, unsigned int> sorted;
    for (unsigned int i = 0; i < 6; i++)
    {
        // Calculer la position de la fenêtre dans le monde
        glm::vec3 windowWorldPos = glm::vec3(carDrawModel * glm::vec4(WINDOW_POSITION[i], 1.0f));
        // Transformer en espace de vue
        glm::vec4 windowViewPos = view * glm::vec4(windowWorldPos, 1.0f);
        // Calculer la distance à la caméra (qui est à l'origine en espace de vue)
        float distance = glm::length(glm::vec3(windowViewPos));
        sorted[distance] = i;
    }

    // Itérer sur la map triée en ordre inverse (du plus loin au plus proche)
    for (std::map<float, unsigned int>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
    {
        unsigned int i = it->second;

        // On applique le même décalage que pour le châssis (frame)
        glm::mat4 model = glm::translate(carDrawModel, glm::vec3(0.0f, 0.25f, 0.0f));
        
        // Calculer la matrice MVP
        glm::mat4 mvp = projView * model;

        // Envoyer les matrices au shader et dessiner
        celShadingShader->setMatrices(mvp, view, model);
        windows[i].draw();
    }

    // Restaurer l'état d'OpenGL
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}