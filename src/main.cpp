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
#include "shader_storage_buffer.hpp"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

struct Material
{
    glm::vec4 emission;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec3 specular;
    GLfloat shininess;
};

struct DirectionalLight
{
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;    
    glm::vec4 direction;
};

struct SpotLight
{
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    
    glm::vec4 position;
    glm::vec3 direction;
    GLfloat exponent;
    GLfloat openingAngle;
    
    GLfloat padding[3];
};

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

// Partie 1
Material bezierMat = 
{
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    1.0f
};

struct BezierCurve
{
    glm::vec3 p0;
    glm::vec3 c0;
    glm::vec3 c1;        
    glm::vec3 p1;
};

BezierCurve curves[5] = 
{
    {
        glm::vec3(-28.7912, 1.4484, -1.7349),
        glm::vec3(-28.0654, 1.4484, 6.1932),
        glm::vec3(-10.3562, 8.8346, 6.5997),
        glm::vec3(-7.6701, 8.8346, 8.9952)
    },
    {
        glm::vec3(-7.6701, 8.8346, 8.9952),
        glm::vec3(-3.9578, 8.8346, 12.3057),
        glm::vec3(-2.5652, 2.4770, 13.6914),
        glm::vec3(2.5079, 1.4484, 11.6581)
    },
    {
        glm::vec3(2.5079, 1.4484, 11.6581),
        glm::vec3(7.5810, 0.4199, 9.6248),
        glm::vec3(16.9333, 3.3014, 5.7702),
        glm::vec3(28.4665, 6.6072, 3.9096)
    },
    {
        glm::vec3(28.4665, 6.6072, 3.9096),
        glm::vec3(39.9998, 9.9131, 2.0491),
        glm::vec3(30.8239, 5.7052, -15.2108),
        glm::vec3(21.3852, 5.7052, -9.0729)
    },
    {
        glm::vec3(21.3852, 5.7052, -9.0729),
        glm::vec3(11.9464, 5.7052, -2.9349),
        glm::vec3(-1.0452, 1.4484, -12.4989),
        glm::vec3(-12.2770, 1.4484, -13.2807)
    }
};


// Partie 3
// Ne pas modifier
struct Particle
{
    glm::vec3 position;
    GLfloat zOrientation;
    glm::vec4 velocity; // vec3, but padded
    glm::vec4 color;
    glm::vec2 size; 
    GLfloat timeToLive;
    GLfloat maxTimeToLive;
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
    , isDay_(false)
    , totalTime(0.0)       
    , timerParticles_(0.0) 
    , nParticles_(0)
    {
    }
	
	void init() override
	{
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

        // TODO: Création des nouveaux shaders
        
        // Partie 1-2
        // TODO: Initialisation des meshes (béziers, patches)
        // Initialisation de la Spline Bézier
        glGenVertexArrays(1, &vaoBezier_);
        glGenBuffers(1, &vboBezier_);
        
        glBindVertexArray(vaoBezier_);
        glBindBuffer(GL_ARRAY_BUFFER, vboBezier_);

        // Allocation de l'espace (Max 16 points internes + 2 extrémités par courbe = 18. Pour 5 courbes = 90 points environ)
        const int maxPoints = 150; 
        glBufferData(GL_ARRAY_BUFFER, maxPoints * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        // Position attribut (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindVertexArray(0);

        glGenVertexArrays(1, &vaoGrass_);
        glGenBuffers(1, &vboGrass_);
        glBindVertexArray(vaoGrass_);
        glBindBuffer(GL_ARRAY_BUFFER, vboGrass_);

        std::vector<glm::vec3> patchVertices;
        const float size = 50.0f; // Taille correspondant au sol
        const int divisions = 6; // Plus il y a de divisions, plus le calcul de distance est précis
        const float step = size / divisions;

        for(int i = 0; i < divisions; i++) {
            for(int j = 0; j < divisions; j++) {
                float x = -size / 2.0f + i * step;
                float z = -size / 2.0f + j * step;

                // Premier triangle du carré
                patchVertices.push_back(glm::vec3(x, 0, z));
                patchVertices.push_back(glm::vec3(x + step, 0, z));
                patchVertices.push_back(glm::vec3(x, 0, z + step));
            
                // Deuxième triangle du carré
                patchVertices.push_back(glm::vec3(x + step, 0, z));
                patchVertices.push_back(glm::vec3(x + step, 0, z + step));
                patchVertices.push_back(glm::vec3(x, 0, z + step));
            }
        }
        numGrassVerts_ = patchVertices.size();

        glBufferData(GL_ARRAY_BUFFER, patchVertices.size() * sizeof(glm::vec3), patchVertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glBindVertexArray(0);
        
        // Partie 3
        // TODO: Allocation des SSBO.
        //       Allouer suffisament d'espace pour le nombre maximal de particules.
        //       Seulement le buffer en entrée à besoin d'être initialisé à 0.
        //       Réfléchisser au type d'usage.
        
        // TODO: Créer un vao pour le dessin des particules et activer les attributs nécessaires.


        // TODO: Création des nouveaux shaders.
        
        
        // TODO: Initialisation de la nouvelle texture pour les particules.
        // "../textures/smoke.png"

        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_PROGRAM_POINT_SIZE); // pour être en mesure de modifier gl_PointSize dans les shaders
        
        edgeEffectShader_.create();
        celShadingShader_.create();
        skyShader_.create();
        grassShader_.create();
        
        car_.edgeEffectShader = &edgeEffectShader_;
        car_.celShadingShader = &celShadingShader_;
        car_.material = &material_;
        
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

	    streetTexture_.use();
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
	    
        
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
        
        
        toggleSun();
        toggleStreetlight();
        updateCarLight();
        
        setLightingUniform();
        
        lights_.allocate(&lightsData_, sizeof(lightsData_));
        lights_.setBindingIndex(1);
        
        CHECK_GL_ERROR;

        loadModels();
	}


	void drawFrame() override
	{
        // TODO: Au besoin, ajouter la recharge de vos nouveaux shaders
        //       après if (ImGui::Button("Reload Shaders")).

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
        ImGui::End();
        
        sceneMain();
	}

	void onClose() override
	{
	}

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

    void initStaticModelMatrices()
    {
        const float OFFSET = 17.0f;
        const float HEIGHT = -0.15f;
        const float SPACING = 10.0f;
        int streetlightIndex = 0;

        for (int side = 0; side < 4; ++side) {
            float angle = glm::radians(90.0f * side);

            for (int i = 0; i < 2; ++i) {
                float zPos = (i == 0) ? -SPACING : SPACING;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, glm::vec3(zPos, HEIGHT, OFFSET));
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                streetlightModelMatrices_[streetlightIndex] = model;
                streetlightLightPositions[streetlightIndex] = glm::vec3(model * glm::vec4(-2.77, 5.2, 0.0, 1.0));
                streetlightIndex++;
            }
        }
    }
    
    void drawStreetlights(const glm::mat4& projView, const glm::mat4& view, bool forOutline = false)
    {
        if (!forOutline) {
            celShadingShader_.use();
        }

        for (unsigned int i = 0; i < N_STREETLIGHTS; i++) {
            const glm::mat4& model = streetlightModelMatrices_[i];
            glm::mat4 mvp = projView * model;

            if (forOutline) {
                glUniformMatrix4fv(edgeEffectShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));
                streetlight_.draw();
                streetlightLight_.draw();
            } else {
                if (!isDay_)
                    setMaterial(streetlightLightMat);
                else
                    setMaterial(streetlightMat);
                celShadingShader_.setMatrices(mvp, view, model);
                streetlightLight_.draw();
            
                setMaterial(streetlightMat);
                streetlightTexture_.use();
                celShadingShader_.setMatrices(mvp, view, model);
                streetlight_.draw();
            }
        }
    }
    
    void drawTree(const glm::mat4& projView, const glm::mat4& view, bool forOutline = false)
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
    
    void drawGround(const glm::mat4& projView, const glm::mat4& view)
    {
        setMaterial(grassMat);

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

        streetcornerTexture_.use();
        for (int side = 0; side < 4; ++side) {
            float angle = glm::radians(90.0f * side);

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
        
        lightsData_.spotLights[N_STREETLIGHTS].position = car_.carModel * glm::vec4(-1.6f, 0.64f, -0.45f, 1.0f);
        lightsData_.spotLights[N_STREETLIGHTS].direction = glm::mat3(car_.carModel) * glm::vec3(-10.0f, -1.0f, 0.0f);
        
        lightsData_.spotLights[N_STREETLIGHTS+1].position = car_.carModel * glm::vec4(-1.6f, 0.64f, 0.45f, 1.0f);
        lightsData_.spotLights[N_STREETLIGHTS+1].direction = glm::mat3(car_.carModel) * glm::vec3(-10.0f, -1.0f, 0.0f);
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
        
        lightsData_.spotLights[N_STREETLIGHTS+2].position = car_.carModel * glm::vec4(1.6f, 0.64f, -0.45f, 1.0f);        
        lightsData_.spotLights[N_STREETLIGHTS+2].direction = glm::mat3(car_.carModel) * glm::vec3(10.0f, -1.0f, 0.0f);
        
        lightsData_.spotLights[N_STREETLIGHTS+3].position = car_.carModel * glm::vec4(1.6f, 0.64f, 0.45f, 1.0f);
        lightsData_.spotLights[N_STREETLIGHTS+3].direction = glm::mat3(car_.carModel) * glm::vec3(10.0f, -1.0f, 0.0f);
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

    void setMaterial(Material& mat)
    {
        material_.updateData(&mat, 0, sizeof(Material));
    }
    
    glm::mat4 getPerspectiveProjectionMatrix()
    {
        float fov = glm::radians(70.0f);
        float aspect = getWindowAspect();
        float near = 0.1f;

        // TODO: Pertinent de modifier la distance ici.
        float far = 300.0f;
        
        return glm::perspective(fov, aspect, near, far);
    }

    glm::vec3 calculateBezier(BezierCurve& curve, float t) {
        float u = 1.0f - t;

        const float b0 = (float)std::pow(u, 3);
        const float b1 = 3.0f * t * u * u;
        const float b2 = 3.0f * t * t * u;
        const float b3 = (float)std::pow(t, 3);

        glm::vec3 p = b0 * curve.p0 + b1 * curve.c0 + b2 * curve.c1 + b3 * curve.p1;

        return p;
    }

    void sceneMain()
    {    
        ImGui::Begin("Scene Parameters");
        ImGui::SliderInt("Bezier Number Of Points", (int*)&bezierNPoints, 0, 16);
        if (ImGui::Button("Animate Camera"))
        {
            isAnimatingCamera = true;
            cameraMode = 1;
        }
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

        if (isAnimatingCamera)
        {
            if (cameraAnimation < 5)
            {
                // 1. Trouver sur quelle courbe on se trouve (0 à 4)
                int curveIndex = glm::clamp(static_cast<int>(floor(cameraAnimation)), 0, 4);
                float t = cameraAnimation - static_cast<float>(curveIndex);

                // 2. Déplacer la caméra sur la spline
                cameraPosition_ = calculateBezier(curves[curveIndex], t);

                // 3. ORIENTER la caméra vers la voiture À CHAQUE FRAME
                glm::vec3 distance = car_.position - cameraPosition_;               
                float horizontalDistance = sqrt(distance.x * distance.x + distance.z * distance.z);

                cameraOrientation_.y = atan2(-distance.x, -distance.z);
                cameraOrientation_.x = atan2(distance.y, horizontalDistance);
            
                // 4. Avancer l'animation
                cameraAnimation += deltaTime_ / 3.0;
            }
            else
            {
                // Fin de l'animation : on réinitialise les variables
                cameraAnimation = 0.f;
                isAnimatingCamera = false;
                cameraMode = 0;
            }
        }
        
        bool hasNumberOfSidesChanged = bezierNPoints != oldBezierNPoints;
        if (hasNumberOfSidesChanged)
        {
            oldBezierNPoints = bezierNPoints;
            
            std::vector<glm::vec3> bezierPoints;
            int nSegments = bezierNPoints + 1;
            
            for (int i = 0; i < 5; ++i) 
            {
                for (int j = 0; j <= nSegments; ++j) 
                {
                    // Éviter de dupliquer les points de jointure entre les 5 courbes
                    if (i > 0 && j == 0) continue; 
                    
                    float t = static_cast<float>(j) / static_cast<float>(nSegments);
                    bezierPoints.push_back(calculateBezier(curves[i], t));
                }
            }
            
            numBezierVerts_ = bezierPoints.size();
            
            glBindBuffer(GL_ARRAY_BUFFER, vboBezier_);
            glBufferSubData(GL_ARRAY_BUFFER, 0, numBezierVerts_ * sizeof(glm::vec3), bezierPoints.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        // TODO: Dessin de la courbe
        // glDraw...
        celShadingShader_.use();
        setMaterial(bezierMat);
        
        glm::mat4 bezierModel = glm::mat4(1.0f);
        glm::mat4 bezierMVP = projView * bezierModel;
        celShadingShader_.setMatrices(bezierMVP, view, bezierModel);

        glBindVertexArray(vaoBezier_);
        glDrawArrays(GL_LINE_STRIP, 0, numBezierVerts_);
        glBindVertexArray(0);
        
        
        // TODO: Dessin du gazon
        // glDraw...
        glm::mat4 groundModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.15f, 0.0f));

        grassShader_.use();
        glUniformMatrix4fv(grassShader_.modelViewULoc, 1, GL_FALSE, glm::value_ptr(view * groundModel));
        glUniformMatrix4fv(grassShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(projView * groundModel));

        glDisable(GL_CULL_FACE);

        glBindVertexArray(vaoGrass_);
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        glDrawArrays(GL_PATCHES, 0, numGrassVerts_);
        glBindVertexArray(0);

        glEnable(GL_CULL_FACE);
        
        
        // ...
        
        // Partie 3
        // TODO: Attention à l'endroit où vous faites votre dessin, la texture des particules est transparente.
        
        // Particles    
        totalTime += deltaTime_;
        timerParticles_ += deltaTime_;        
        const float particlesSpawnInterval = 0.2f;
        
        unsigned int particlesToAdd = timerParticles_ / particlesSpawnInterval;
        timerParticles_ -= particlesToAdd * particlesSpawnInterval;
        
        nParticles_ += particlesToAdd;
        if (nParticles_ > MAX_PARTICLES_)
            nParticles_ = MAX_PARTICLES_;

        // Particles update
        
        // TODO: Mise à jour des données à l'aide du compute shader
        //       Envoyer vos uniforms.
        
        // Utiliser car_.carModel pour calculer la nouvelle position et direction d'émission de particule.
        // glm::vec3 exhaustPos = vec3(2.0, 0.24, -0.43);
        // glm::vec3 exhaustDir = vec3(1, 0, 0);
        
        // TODO: Configurer les buffers d'entrée et de sortie.
        
        // TODO: Envois de la commande de calcul.
        //       Pas besoin d'optimiser le nombre de work group vs la taille local (dans le shader).
        
        
        // Particles draw
        
        // TODO: Dessin des particules. Utiliser le nombre de particules actuellement utilisées.
        //       Utiliser la texture et envoyer vos uniforms.
        //       Il sera nécessaire de spécifier les entrée en spécifiant le buffer d'entrée.
        //       Activer le blending et restaurer l'état du contexte modifié.
        
        // TODO: Interchanger les deux buffers, celui en entrée devient la sortie, et vice versa.

        // Sky box
        glDepthFunc(GL_LEQUAL);
        skyShader_.use();
        glm::mat4 skyView = glm::mat4(glm::mat3(view));
        glm::mat4 skyMVP = proj * skyView;
        glUniformMatrix4fv(skyShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(skyMVP));
        (isDay_ ? skyboxTexture_ : skyboxNightTexture_).use();
        skybox_.draw();
        glDepthFunc(GL_LESS);

        // Sol sans contour
        celShadingShader_.use();
        drawGround(projView, view);

        // Objets avec contour
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);
        glDepthMask(GL_TRUE);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        setMaterial(defaultMat);
        carTexture_.use();
        car_.draw(projView, view, false);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        carWindowTexture_.use();
        car_.drawWindows(projView, view);
        
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        glStencilFunc(GL_ALWAYS, 2, 0xFF);
        setMaterial(grassMat);
        treeTexture_.use();
        drawTree(projView, view);

        glStencilFunc(GL_ALWAYS, 3, 0xFF);
        setMaterial(streetlightMat);
        drawStreetlights(projView, view);

        // effet de contour
        glStencilMask(0x00);

        edgeEffectShader_.use();

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        car_.draw(projView, view, true);

        glStencilFunc(GL_NOTEQUAL, 2, 0xFF);
        drawTree(projView, view, true);

        glStencilFunc(GL_NOTEQUAL, 3, 0xFF);
        drawStreetlights(projView, view, true);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        // Objets transparent
        celShadingShader_.use();
        setMaterial(windowMat);
        carWindowTexture_.use();
        car_.drawWindows(projView, view);
    }
 
private:
    
    Car car_;

    unsigned int bezierNPoints = 3; // Nombre de points supplémentaires sur la courbe, 0 est une ligne droite
    unsigned int oldBezierNPoints = 0;
    
    int cameraMode = 0;
    float cameraAnimation = 0.f;
    bool isAnimatingCamera = false;

    // Partie 1-2
    // TODO: Ajouter les attributs de vbo, ebo, vao nécessaire
    GLuint vaoBezier_ = 0;
    GLuint vboBezier_ = 0;
    int numBezierVerts_ = 0;

    GLuint vaoGrass_ = 0;
    GLuint vboGrass_ = 0;
    int numGrassVerts_ = 0;
    
    
    // Partie 3
    // TODO: Attributs supplémentaires
    GLuint vaoParticles_;
    
    float totalTime;
    float timerParticles_;
    
    static const unsigned int MAX_PARTICLES_ = 64;
    unsigned int nParticles_;    
    
    // TODO: Ajouter vos shaders
    // TODO: Ajouter la texture de fumé
    
    // Ssbo
    ShaderStorageBuffer particles_[2];
    
    // Matrices statiques
    static constexpr unsigned int N_STREET_PATCHES = 7*4+4;
    glm::mat4 treeModelMatrice_;
    glm::mat4 groundModelMatrice_;
    glm::mat4 streetPatchesModelMatrices_[N_STREET_PATCHES];
    
    // Shaders
    EdgeEffect edgeEffectShader_;
    CelShading celShadingShader_;
    Sky skyShader_;
    GrassShader grassShader_;
    
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
