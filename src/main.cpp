#include <cstddef>
#include <cstdint>

#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <numbers>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "happly.h"
#include <imgui/imgui.h>

#include <inf2705/OpenGLApplication.hpp>

#include "model.hpp"
#include "car.hpp"

#include "model_data.hpp"
#include "shaders.hpp"
#include "textures.hpp"
#include "uniform_buffer.hpp"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

// Définition des structures pour la communication avec le shader. NE PAS MODIFIER.

struct Material
{
    glm::vec4 emission; // vec3, but padded
    glm::vec4 ambient;  // vec3, but padded
    glm::vec4 diffuse;  // vec3, but padded
    glm::vec3 specular;
    GLfloat shininess;
};

struct DirectionalLight
{
    glm::vec4 ambient;   // vec3, but padded
    glm::vec4 diffuse;   // vec3, but padded
    glm::vec4 specular;  // vec3, but padded    
    glm::vec4 direction; // vec3, but padded
};

struct SpotLight
{
    glm::vec4 ambient;   // vec3, but padded
    glm::vec4 diffuse;   // vec3, but padded
    glm::vec4 specular;  // vec3, but padded
    
    glm::vec4 position;  // vec3, but padded
    glm::vec3 direction;
    GLfloat exponent;
    GLfloat openingAngle;
    
    GLfloat padding[3];
};

// Matériels

Material defaultMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material grassMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.8f, 0.8f, 0.8f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.05f, 0.05f, 0.05f},
    100.0f
};

Material streetMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.7f, 0.7f, 0.7f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.025f, 0.025f, 0.025f},
    300.0f
};

Material streetlightMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.8f, 0.8f, 0.8f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material streetlightLightMat = 
{
    {0.8f, 0.7f, 0.5f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material windowMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f},
    2.0f
};

struct Pos
{
    GLfloat x;
    GLfloat y;
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

struct App : public OpenGLApplication
{
    App()
    : cameraPosition_(0.0f, 5.0f, -7.5f)
    , cameraOrientation_(0.0f, 0.0f)
    , currentScene_(0)
    , isMouseMotionEnabled_(false)
    {
    }
	
	void init() override
	{
		// Le message expliquant les touches de clavier.
		setKeybindMessage(
			"ESC : quitter l'application." "\n"
			"T : changer de scène." "\n"
			"W : déplacer la caméra vers l'avant." "\n"
			"S : déplacer la caméra vers l'arrière." "\n"
			"A : déplacer la caméra vers la gauche." "\n"
			"D : déplacer la caméra vers la droite." "\n"
			"Q : déplacer la caméra vers le bas." "\n"
			"E : déplacer la caméra vers le haut." "\n"
			"Flèches : tourner la caméra." "\n"
			"Souris : tourner la caméra" "\n"
			"Espace : activer/désactiver la souris." "\n"
		);

		// Config de base.
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        
        // Partie 1
        
        // TODO:
        // Création des shaders program. Fait appel à la méthode "create()".
        edgeEffectShader_.create();
        celShadingShader_.create();
        skyShader_.create();
        
        // TODO: À ajouter.
        car_.edgeEffectShader = &edgeEffectShader_;
        car_.celShadingShader = &celShadingShader_;
        car_.material = &material_;
        
        
        // TODO: Chargement des textures, ainsi que la configuration de leurs paramètres.
        //
        //       Les textures ne se répètent pas, sauf le sol, la route (mais pas les coins), les arbres et les lampadaires.
        //
        //       Les textures ont un fini lisse, à l’exception des arbres, des lumières de lampadaire et
        //       des fenêtres de la voiture.
        //       
        //       Le mipmap __ne doit pas__ être activé pour toutes les textures, seulement le sol et la route.
        grassTexture_.load("../textures/grass.jpg");
        grassTexture_.setWrap(GL_REPEAT);
        grassTexture_.setFiltering(GL_LINEAR);
        grassTexture_.enableMipmap();

        streetTexture_.load("../textures/street.jpg");
        streetTexture_.setWrap(GL_REPEAT);
        streetTexture_.setFiltering(GL_LINEAR);
        streetTexture_.enableMipmap();

        streetcornerTexture_.load("../textures/streetcorner.jpg");
        streetcornerTexture_.setWrap(GL_CLAMP_TO_EDGE);
        streetcornerTexture_.setFiltering(GL_LINEAR);


        carTexture_.load("../textures/car.png");
        carTexture_.setWrap(GL_CLAMP_TO_EDGE);
        carTexture_.setFiltering(GL_LINEAR);

        carWindowTexture_.load("../textures/window.png");
        carWindowTexture_.setWrap(GL_CLAMP_TO_EDGE);
        carWindowTexture_.setFiltering(GL_NEAREST);

        treeTexture_.load("../textures/pine.jpg");
        treeTexture_.setWrap(GL_REPEAT);
        treeTexture_.setFiltering(GL_NEAREST);

        streetlightTexture_.load("../textures/streetlight.jpg");
        streetlightTexture_.setWrap(GL_REPEAT);
        streetlightTexture_.setFiltering(GL_LINEAR);

        // Simplement pour réduire l'effet "négatif" du mipmap qui rend la
        // texture flou trop près.
	    streetTexture_.use();
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
	    
        
        // TODO: Chargement des deux skyboxes.
        
        const char* pathes[] = {
            "../textures/skybox/Daylight Box_Right.bmp",
            "../textures/skybox/Daylight Box_Left.bmp",
            "../textures/skybox/Daylight Box_Top.bmp",
            "../textures/skybox/Daylight Box_Bottom.bmp",
            "../textures/skybox/Daylight Box_Front.bmp",
            "../textures/skybox/Daylight Box_Back.bmp",
        };
        
        const char* nightPathes[] = {
            "../textures/skyboxNight/right.png",
            "../textures/skyboxNight/left.png",
            "../textures/skyboxNight/top.png",
            "../textures/skyboxNight/bottom.png",
            "../textures/skyboxNight/front.png",
            "../textures/skyboxNight/back.png",
        };
        skyboxTexture_.load(pathes);
        skyboxNightTexture_.load(nightPathes);

        
        initStaticModelMatrices();
        
        // Partie 3
        
        // TODO: À ajouter. Aucune modification nécessaire.
        material_.allocate(&defaultMat, sizeof(Material));
        material_.setBindingIndex(0);
        
        lightsData_.dirLight =
        {
            {0.2f, 0.2f, 0.2f, 0.0f},
            {1.0f, 1.0f, 1.0f, 0.0f},
            {0.5f, 0.5f, 0.5f, 0.0f},
            {0.5f, -1.0f, 0.5f, 0.0f}
        };
        
        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {                
            lightsData_.spotLights[i].position = glm::vec4(streetlightLightPositions[i], 0.0f);
            lightsData_.spotLights[i].direction = glm::vec3(0, -1, 0);
            lightsData_.spotLights[i].exponent = 6.0f;
            lightsData_.spotLights[i].openingAngle = 60.f;        
        }
        
        // Initialisation des paramètres de lumière des phares
        
        lightsData_.spotLights[N_STREETLIGHTS].position = glm::vec4(-1.6, 0.64, -0.45, 0.0f);
        lightsData_.spotLights[N_STREETLIGHTS].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS].openingAngle = 30.f;
            
        lightsData_.spotLights[N_STREETLIGHTS+1].position = glm::vec4(-1.6, 0.64, 0.45, 0.0f);
        lightsData_.spotLights[N_STREETLIGHTS+1].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS+1].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS+1].openingAngle = 30.f;
            
        lightsData_.spotLights[N_STREETLIGHTS+2].position = glm::vec4(1.6, 0.64, -0.45, 0.0f);
        lightsData_.spotLights[N_STREETLIGHTS+2].direction = glm::vec3(10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS+2].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS+2].openingAngle = 60.f;
                   
        lightsData_.spotLights[N_STREETLIGHTS+3].position = glm::vec4(1.6, 0.64, 0.45, 0.0f);
        lightsData_.spotLights[N_STREETLIGHTS+3].direction = glm::vec3(10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS+3].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS+3].openingAngle = 60.f;
        
        
        toggleStreetlight();
        updateCarLight();
        
        setLightingUniform();
        
        lights_.allocate(&lightsData_, sizeof(lightsData_));
        lights_.setBindingIndex(1);
        
        CHECK_GL_ERROR;

        loadModels();
	}


	// Appelée à chaque trame. Le buffer swap est fait juste après.
	void drawFrame() override
	{
        // On efface les tampons de couleur, de profondeur et de stencil
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
        ImGui::End();
        
        sceneMain();
	}

	// Appelée lorsque la fenêtre se ferme.
	void onClose() override
	{
	}

	// Appelée lors d'une touche de clavier.
	void onKeyPress(const sf::Event::KeyPressed& key) override
	{
		using enum sf::Keyboard::Key;
		switch (key.code)
		{
		    case Escape:
		        window_.close();
	        break;
		    case Space:
		        isMouseMotionEnabled_ = !isMouseMotionEnabled_;
		        if (isMouseMotionEnabled_)
		        {
		            window_.setMouseCursorGrabbed(true);
		            window_.setMouseCursorVisible(false);
	            }
	            else
	            {
	                window_.setMouseCursorGrabbed(false);
	                window_.setMouseCursorVisible(true);
                }
	        break;
	        case T:
                currentScene_ = ++currentScene_ < N_SCENE_NAMES ? currentScene_ : 0;
            break;
		    default: break;
		}
	}

	void onResize(const sf::Event::Resized& event) override
	{	
	}
	
	void onMouseMove(const sf::Event::MouseMoved& mouseDelta) override
	{	    
	    if (!isMouseMotionEnabled_)
	        return;
        
        const float MOUSE_SENSITIVITY = 0.1;
        float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
        float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
	    cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;
	}
	
	void updateCameraInput() 
    {
        if (!window_.hasFocus())
            return;
            
        if (isMouseMotionEnabled_)
        {
            sf::Vector2u windowSize = window_.getSize();
            sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
            sf::Mouse::setPosition(windowHalfSize, window_);
        }
        
        float cameraMouvementX = 0;
        float cameraMouvementY = 0;
        
        const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;
        
        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;

        // Keyboard input
        glm::vec3 positionOffset = glm::vec3(0.0);
        const float SPEED = 10.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            positionOffset.z -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            positionOffset.z += SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            positionOffset.x -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            positionOffset.x += SPEED;
            
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            positionOffset.y -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            positionOffset.y += SPEED;

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }
    
    void loadModels()
    {
        car_.loadModels();
        tree_.load("../models/pine.ply");
        streetlight_.load("../models/streetlight.ply");
        streetlightLight_.load("../models/streetlight_light.ply");
        skybox_.load("../models/skybox.ply");
        grass_.load(ground, sizeof(ground), planeElements, sizeof(planeElements));
        street_.load(street, sizeof(street), planeElements, sizeof(planeElements));
        streetcorner_.load(streetcorner, sizeof(streetcorner), planeElements, sizeof(planeElements));
    }

    // Méthode pour le calcul des matrices initiales des arbres et des lampadaires.
    void initStaticModelMatrices()
    {
        // ...
        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            // ...
            
            // TODO: À ajouter. C'est pour avoir la position de la lumière du lampadaire pour la partie 3.
            streetlightLightPositions[i] = glm::vec3(streetlightModelMatrices_[i] * glm::vec4(-2.77, 5.2, 0.0, 1.0));
        }
    }
    
    void drawStreetlights(glm::mat4& projView, glm::mat4& view, bool forOutline = false)
    {
        const float OFFSET = 17.0f;
        const float HEIGHT = -0.15f;
        const float SPACING = 10.0f;

        if (!forOutline) {
            celShadingShader_.use();
        }

        for (int side = 0; side < 4; ++side) {
            float angle = glm::radians(90.0f * side);

            for (int i = 0; i < 2; ++i) {
                float zPos = (i == 0) ? -SPACING : SPACING;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, glm::vec3(zPos, HEIGHT, OFFSET));
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 mvp = projView * model;

                if (forOutline) {
                    glUniformMatrix4fv(edgeEffectShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));
                    streetlight_.draw();
                    streetlightLight_.draw();
                } else {
                    // Dessin de la lumière du lampadaire
                    if (!isDay_)
                        setMaterial(streetlightLightMat);
                    else
                        setMaterial(streetlightMat);
                    celShadingShader_.setMatrices(mvp, view, model);
                    streetlightLight_.draw();
                
                    // Dessin du poteau du lampadaire
                    setMaterial(streetlightMat);
                    streetlightTexture_.use();
                    celShadingShader_.setMatrices(mvp, view, model);
                    streetlight_.draw();
                }
            }
        }
    }
    
    void drawTree(glm::mat4& projView, glm::mat4& view, bool forOutline = false)
    {
        glDisable(GL_CULL_FACE);

        glm::mat4 treeModel =  glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.1f, 1.0f));
        treeModel = glm::scale(treeModel, glm::vec3(15.0f, 15.0f, 15.0f));
        glm::mat4 treeMVP = projView * treeModel;

        if (forOutline)
            glUniformMatrix4fv(edgeEffectShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(treeMVP));
        else
            celShadingShader_.setMatrices(treeMVP, view, treeModel);

        tree_.draw();
        glEnable(GL_CULL_FACE);
    }
    
    // TODO: À modifier, ajouter les textures
    void drawGround(glm::mat4& projView, glm::mat4& view)
    {
        setMaterial(grassMat);
        // offset ground by -0.1 to avoid overlap (depth buffer doesnt know which surface is in front)
        glm::mat4 grassModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f));
        grassModel = glm::scale(grassModel, glm::vec3(50.0f, 1.0f, 50.0f));

        glm::mat4 grassMVP = projView * grassModel;
        celShadingShader_.setMatrices(grassMVP, view, grassModel);
        grassTexture_.use();
        grass_.draw();

        const float ROAD_OFFSET = 20.0f;
        const float ROAD_SPACING = 5.0f;
        const int N_ROAD_SEGMENT = 7;

        setMaterial(streetMat);
        streetTexture_.use();
        for (int side = 0; side < 4; ++side) {
            float angle = glm::radians(90.0f * side);

            for (int i = 0; i < N_ROAD_SEGMENT; ++i) {
                float segmentPos = (i - (N_ROAD_SEGMENT / 2)) * ROAD_SPACING;

                streetTexture_.use();

                glm::mat4 roadModel = glm::mat4(1.0f);
                roadModel = glm::rotate(roadModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                roadModel = glm::translate(roadModel ,glm::vec3(segmentPos, 0.0f, ROAD_OFFSET));
                roadModel = glm::scale(roadModel, glm::vec3(5.0f, 1.0f, 5.0f));

                glm::mat4 roadMVP = projView * roadModel;
                celShadingShader_.setMatrices(roadMVP, view, roadModel);
                
                street_.draw();
            }
        }

        // Street corners use the same material as the street.
        streetcornerTexture_.use();
        for (int side = 0; side < 4; ++side) {
            float angle = glm::radians(90.0f * side);

            streetcornerTexture_.use();

            glm::mat4 cornerModel = glm::mat4(1.0f);
            cornerModel = glm::rotate(cornerModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            cornerModel = glm::translate(cornerModel, glm::vec3(ROAD_OFFSET, 0.0f, ROAD_OFFSET));
            cornerModel = glm::scale(cornerModel, glm::vec3(5.0f, 1.0f, 5.0f));

            glm::mat4 cornerMVP = projView * cornerModel;
            celShadingShader_.setMatrices(cornerMVP, view, cornerModel);
            streetcorner_.draw();
        }
    }
    
    glm::mat4 getViewMatrix()
    {
        glm::mat4 view = glm::mat4(1.0f);

        view = glm::rotate(view, -cameraOrientation_.x, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, -cameraOrientation_.y, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, -cameraPosition_);
        
        return view;
    }

    void setLightingUniform()
    {
        celShadingShader_.use();
        glUniform1i(celShadingShader_.nSpotLightsULoc, N_STREETLIGHTS+4);
        
        float ambientIntensity = 0.05;
        glUniform3f(celShadingShader_.globalAmbientULoc, ambientIntensity, ambientIntensity, ambientIntensity);
    }

    // TODO: À ajouter. Pas de modification.
    void toggleSun()
    {
        if (isDay_)
        {
            lightsData_.dirLight.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f); 
            lightsData_.dirLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
        }
        else
        {
            lightsData_.dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); 
            lightsData_.dirLight.diffuse = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
    
    // TODO: À ajouter. Pas de modification.
    void toggleStreetlight()
    {
        if (isDay_)
        {
            for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.0f), 0.0f);
            }
        }
        else
        {
            for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.02f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.8f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.4f), 0.0f);
            }
        }
    }

    // TODO: À ajouter.
    void updateCarLight()
    {
        if (car_.isHeadlightOn)
        {
            lightsData_.spotLights[N_STREETLIGHTS].ambient = glm::vec4(glm::vec3(0.01), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].diffuse = glm::vec4(glm::vec3(1.0), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].specular = glm::vec4(glm::vec3(0.4), 0.0f);
            
            lightsData_.spotLights[N_STREETLIGHTS+1].ambient = glm::vec4(glm::vec3(0.01), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+1].diffuse = glm::vec4(glm::vec3(1.0), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+1].specular = glm::vec4(glm::vec3(0.4), 0.0f);
            
            // TODO: Partie 3.
            //       Utiliser car_.carModel pour calculer la nouvelle position et orientation de la lumière.
            //       La lumière devrait suivre le véhicule qui se déplace.
            
            lightsData_.spotLights[N_STREETLIGHTS].position = glm::vec4(-1.6, 0.64, -0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS].direction = glm::vec3(-10, -1, 0);
            
            lightsData_.spotLights[N_STREETLIGHTS+1].position = glm::vec4(-1.6, 0.64, 0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS+1].direction = glm::vec3(-10, -1, 0);
        }
        else
        {
            lightsData_.spotLights[N_STREETLIGHTS].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].specular = glm::vec4(0.0f);
            
            lightsData_.spotLights[N_STREETLIGHTS+1].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+1].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+1].specular = glm::vec4(0.0f);
        }
        
        if (car_.isBraking)
        {
            lightsData_.spotLights[N_STREETLIGHTS+2].ambient = glm::vec4(0.01, 0.0, 0.0, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+2].diffuse = glm::vec4(0.9, 0.1, 0.1, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+2].specular = glm::vec4(0.35, 0.05, 0.05, 0.0f);
            
            lightsData_.spotLights[N_STREETLIGHTS+3].ambient = glm::vec4(0.01, 0.0, 0.0, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+3].diffuse = glm::vec4(0.9, 0.1, 0.1, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+3].specular = glm::vec4(0.35, 0.05, 0.05, 0.0f);
            
            // TODO: Partie 3.
            //       Utiliser car_.carModel pour calculer la nouvelle position et orientation de la lumière.
            //       La lumière devrait suivre le véhicule qui se déplace.
            
            lightsData_.spotLights[N_STREETLIGHTS+2].position = glm::vec4(1.6, 0.64, -0.45, 1.0f);        
            lightsData_.spotLights[N_STREETLIGHTS+2].direction = glm::vec3(10, -1, 0);
            
            lightsData_.spotLights[N_STREETLIGHTS+3].position = glm::vec4(1.6, 0.64, 0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS+3].direction = glm::vec3(10, -1, 0);
        }
        else
        {
            lightsData_.spotLights[N_STREETLIGHTS+2].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+2].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+2].specular = glm::vec4(0.0f);
            
            lightsData_.spotLights[N_STREETLIGHTS+3].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+3].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS+3].specular = glm::vec4(0.0f);
        }
    }

    // TODO: À ajouter. Pas de modification.
    void setMaterial(Material& mat)
    {
        // Ça vous donne une idée de comment utiliser les ubo dans car.cpp.
        material_.updateData(&mat, 0, sizeof(Material));
    }
    
    glm::mat4 getPerspectiveProjectionMatrix()
    {
        float fov = glm::radians(70.0f);
        float aspect = getWindowAspect();
        float near = 0.1f;
        float far = 300.0f;
        
        return glm::perspective(fov, aspect, near, far);
    }

    // TODO: À ajouter et modifier.
    //       Ajouter les textures, les skyboxes, les fenêtres de la voiture,
    //       les effets de contour, etc.
    void sceneMain()
    {    
        ImGui::Begin("Scene Parameters");
        if (ImGui::Button("Toggle Day/Night"))
        {
            isDay_ = !isDay_;
            toggleSun();
            toggleStreetlight();
            lights_.updateData(&lightsData_, 0, sizeof(DirectionalLight) + N_STREETLIGHTS * sizeof(SpotLight));
        }
        ImGui::SliderFloat("Car Speed", &car_.speed, -10.0f, 10.0f, "%.2f m/s");
        ImGui::SliderFloat("Steering Angle", &car_.steeringAngle, -30.0f, 30.0f, "%.2f°");
        if (ImGui::Button("Reset Steering"))
            car_.steeringAngle = 0.f;
        ImGui::Checkbox("Headlight", &car_.isHeadlightOn);
        ImGui::Checkbox("Left Blinker", &car_.isLeftBlinkerActivated);
        ImGui::Checkbox("Right Blinker", &car_.isRightBlinkerActivated);
        ImGui::Checkbox("Brake", &car_.isBraking);
        ImGui::End();
    
        updateCameraInput();
        car_.update(deltaTime_);
        
        updateCarLight();
        lights_.updateData(&lightsData_.spotLights[N_STREETLIGHTS], sizeof(DirectionalLight) + N_STREETLIGHTS * sizeof(SpotLight), 4 * sizeof(SpotLight));
                
        glm::mat4 view = getViewMatrix();
        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 projView = proj * view;

        // --- Dessin du skybox ---
        // On le dessine en premier pour qu'il serve de fond à toute la scène.
        // Ainsi, le contour des objets se dessinera par-dessus le ciel et non l'inverse.
        glDepthFunc(GL_LEQUAL);
        skyShader_.use();
        glm::mat4 skyView = glm::mat4(glm::mat3(view));
        glm::mat4 skyMVP = proj * skyView;
        glUniformMatrix4fv(skyShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(skyMVP));
        (isDay_ ? skyboxTexture_ : skyboxNightTexture_).use();
        skybox_.draw();
        glDepthFunc(GL_LESS); // On remet la fonction de profondeur par défaut pour le reste.

        // --- Dessin des objets sans contour ---
        celShadingShader_.use();
        setMaterial(grassMat);
        drawGround(projView, view);

        // --- Dessin des objets avec contour ---

        // 1. Première passe : dessiner les objets normalement et remplir le stencil buffer
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);
        glDepthMask(GL_TRUE);

        celShadingShader_.use();

        // Voiture - ID 1
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // <-- Valeur 1
        setMaterial(defaultMat);
        carTexture_.use();
        car_.draw(projView, view, false);

        // Arbre - ID 2
        glStencilFunc(GL_ALWAYS, 2, 0xFF); // <-- Valeur 2
        setMaterial(grassMat);
        treeTexture_.use();
        drawTree(projView, view);

        // Lampadaires - ID 3
        glStencilFunc(GL_ALWAYS, 3, 0xFF); // <-- Valeur 3
        setMaterial(streetlightMat);
        drawStreetlights(projView, view);

        // 2. Deuxième passe : dessiner les objets agrandis pour le contour
        glStencilMask(0x00); // Désactiver l'écriture
        // TRÈS IMPORTANT : On laisse le test de profondeur (GL_DEPTH_TEST) activé !

        edgeEffectShader_.use();

        // Contour Voiture (s'annule SEULEMENT si stencil == 1)
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        car_.draw(projView, view, true);

        // Contour Arbre (s'annule SEULEMENT si stencil == 2)
        glStencilFunc(GL_NOTEQUAL, 2, 0xFF);
        drawTree(projView, view, true);

        // Contour Lampadaires (s'annule SEULEMENT si stencil == 3)
        glStencilFunc(GL_NOTEQUAL, 3, 0xFF);
        drawStreetlights(projView, view, true);

        // 3. Restaurer les états
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        // --- Dessin des objets transparents (fenêtres) ---
        // Les fenêtres sont dessinées en dernier, après les objets opaques et le skybox.
        // Elles ne participent pas à l'effet de contour.
        setMaterial(windowMat);
        carWindowTexture_.use();
        car_.drawWindows(projView, view);
    }
 
private:
    
    Car car_;
    
    // Matrices statiques
    static constexpr unsigned int N_STREET_PATCHES = 7*4+4;
    glm::mat4 treeModelMatrice_;
    glm::mat4 groundModelMatrice_;
    glm::mat4 streetPatchesModelMatrices_[N_STREET_PATCHES];
    
    // Shaders
    EdgeEffect edgeEffectShader_;
    CelShading celShadingShader_;
    Sky skyShader_;
    
    // Textures
    Texture2D grassTexture_;
    Texture2D streetTexture_;
    Texture2D streetcornerTexture_;
    Texture2D carTexture_;
    Texture2D carWindowTexture_;
    Texture2D treeTexture_;
    Texture2D streetlightTexture_;
    Texture2D streetlightLightTexture_;
    TextureCubeMap skyboxTexture_;
    TextureCubeMap skyboxNightTexture_;
    
    // Uniform buffers
    UniformBuffer material_;
    UniformBuffer lights_;

        struct {
        DirectionalLight dirLight;
        SpotLight spotLights[16];
    } lightsData_;
    
    bool isDay_;
    
    Model tree_;
    Model streetlight_;
    Model streetlightLight_;
    Model grass_;
    Model street_;
    Model streetcorner_;
    Model skybox_;
    
    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;
    
    static constexpr unsigned int N_TREES = 1;
    glm::mat4 treeModelMatrices_[N_TREES];
    static constexpr unsigned int N_STREETLIGHTS = 8;
    glm::mat4 streetlightModelMatrices_[N_STREETLIGHTS];
    glm::vec3 streetlightLightPositions[N_STREETLIGHTS];
    
    // Imgui var
    const char* const SCENE_NAMES[1] = {
        "Main scene"
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;
    
    bool isMouseMotionEnabled_;
};


int main(int argc, char* argv[])
{
	WindowSettings settings = {};
	settings.fps = 60;
	settings.context.depthBits = 24;
	settings.context.stencilBits = 8;
	settings.context.antiAliasingLevel = 4;
	settings.context.majorVersion = 3;
	settings.context.minorVersion = 3;
	settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

	App app;
	app.run(argc, argv, "Tp2", settings);
}