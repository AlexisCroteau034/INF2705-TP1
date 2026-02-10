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

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

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
    : nSide_(5)
    , oldNSide_(0)
    , cameraPosition_(0.0f, 5.0f, -7.5f)
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
		
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
       
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
                
        loadShaderPrograms();
        
        // Partie 1
        initShapeData();
        
        // Partie 2
        loadModels();
	}
	
	    
    void checkShaderCompilingError(const char* name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            glDeleteShader(id);
            std::cout << "Shader \"" << name << "\" compile error: " << infoLog << std::endl;
        }
    }


    void checkProgramLinkingError(const char* name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            glDeleteProgram(id);
            std::cout << "Program \"" << name << "\" linking error: " << infoLog << std::endl;
        }
    }
	

	// Appelée à chaque trame. Le buffer swap est fait juste après.
	void drawFrame() override
	{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
        ImGui::End();
        
        switch (currentScene_)
        {
            case 0: sceneShape();  break;
            case 1: sceneModels(); break;
        }
	}

	// Appelée lorsque la fenêtre se ferme.
	void onClose() override
	{
        glDeleteVertexArrays(1, &vao_);
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
        glDeleteProgram(basicSP_);
        glDeleteProgram(transformSP_);
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
        grass_.load("../models/grass.ply");
        street_.load("../models/street.ply");
        streetcorner_.load("../models/streetcorner.ply");
    }
    
    GLuint loadShaderObject(GLenum type, const char* path)
    {
		GLuint shader = glCreateShader(type);

		std::string srcStd = readFile(path).c_str();
        const char* srcC = srcStd.c_str();

		glShaderSource(shader, 1, &srcC, nullptr);
		glCompileShader(shader);
		checkShaderCompilingError(path, shader);
        return shader;
    }
    
    void loadShaderPrograms()
    {
		basicSP_ = glCreateProgram();

        // Partie 1
        const char* BASIC_VERTEX_SRC_PATH = "./shaders/basic.vs.glsl";
        const char* BASIC_FRAGMENT_SRC_PATH = "./shaders/basic.fs.glsl";

		GLuint basicVertShader = loadShaderObject(GL_VERTEX_SHADER, BASIC_VERTEX_SRC_PATH);
		glAttachShader(basicSP_, basicVertShader);
		GLuint basicFragShader = loadShaderObject(GL_FRAGMENT_SHADER, BASIC_FRAGMENT_SRC_PATH);
		glAttachShader(basicSP_, basicFragShader);
        
        glLinkProgram(basicSP_);
        checkProgramLinkingError("basicSP_", basicSP_);

        glDetachShader(basicSP_, basicVertShader);
        glDeleteShader(basicVertShader);
        glDetachShader(basicSP_, basicFragShader);
        glDeleteShader(basicFragShader);
        
        // Partie 2
        const char* TRANSFORM_VERTEX_SRC_PATH = "./shaders/transform.vs.glsl";
        const char* TRANSFORM_FRAGMENT_SRC_PATH = "./shaders/transform.fs.glsl";

        transformSP_ = glCreateProgram();

		GLuint transformVertShader = loadShaderObject(GL_VERTEX_SHADER, TRANSFORM_VERTEX_SRC_PATH);
		glAttachShader(transformSP_, transformVertShader);
		GLuint transformFragShader = loadShaderObject(GL_FRAGMENT_SHADER, TRANSFORM_FRAGMENT_SRC_PATH);
		glAttachShader(transformSP_, transformFragShader);
        
        glLinkProgram(transformSP_);
        checkProgramLinkingError("transformSP_", basicSP_);

        glDetachShader(transformSP_, transformVertShader);
        glDeleteShader(transformVertShader);
        glDetachShader(transformSP_, transformFragShader);
        glDeleteShader(transformFragShader);

        colorModUniformLocation_ = glGetUniformLocation(transformSP_, "colorMod");
        car_.colorModUniformLocation = glGetUniformLocation(transformSP_, "colorMod");
        mvpUniformLocation_ = glGetUniformLocation(transformSP_, "mvp");
        car_.mvpUniformLocation = glGetUniformLocation(transformSP_, "mvp");
    }

    void generateNgon()
    {
        const float RADIUS = 0.7f;
        // change the color for each vertex
        static constexpr Color colorsLookup[3] = { 
            { 1.0f, 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f, 1.0f } 
        };
        for (int i = 0; i < nSide_; i++)
        {
            float angle = 2 * std::numbers::pi * i / nSide_;
            vertices_[i] = { 
                .pos = { RADIUS * std::sin(angle), RADIUS * std::cos(angle) },
                .color = colorsLookup[i % 3]
            };
        }
        vertices_[nSide_] = { .pos = { 0.0f, 0.0f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } };
        for (int i = 0, offset = 0; i < nSide_; i++)
        {
            // connection indexes (counter clockwise to avoid culling)
            elements_[offset++] = nSide_; // center
            elements_[offset++] = (i + 1) % nSide_; // next
            elements_[offset++] = i; // current
        }
    }

    void initShapeData()
    {
        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);

        glGenBuffers(1, &vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), vertices_, GL_STATIC_DRAW);

        glVertexAttribPointer(0, N_VALUES_PER_POS, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, N_VALUES_PER_COLOR, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) sizeof(Pos));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements_), elements_, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }
    
    void sceneShape()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderInt("Sides", &nSide_, MIN_N_SIDES, MAX_N_SIDES);
        ImGui::End();
        
        bool hasNumberOfSidesChanged = nSide_ != oldNSide_;
        if (hasNumberOfSidesChanged)
        {
            oldNSide_ = nSide_;
            generateNgon();

            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_), vertices_);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(elements_), elements_);
        }
        glUseProgram(basicSP_);
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, nSide_ * 3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    void drawStreetlights(glm::mat4& projView)
    {
        glUniform4f(colorModUniformLocation_, 1.0f, 1.0f, 1.0f, 1.0f);

        const float OFFSET = 17.0f;
        const float HEIGHT = -0.15f;
        const float SPACING = 10.0f;

        for (int side = 0; side < 4; ++side) {
            float angle = glm::radians(90.0f * side);

            for (int i = 0; i < 2; ++i) {
                float zPos = (i == 0) ? -SPACING : SPACING;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, glm::vec3(zPos, HEIGHT, OFFSET));
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                glm::mat4 mvp = projView * model;
                glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(mvp));
                streetlight_.draw();
            }
        }
    }
    
    void drawTree(glm::mat4& projView)
    {
        glDisable(GL_CULL_FACE);

        glUniform4f(colorModUniformLocation_, 1.0f, 1.0f, 1.0f, 1.0f);
        glm::mat4 treeModel =  glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.1f, 1.0f));
        treeModel = glm::scale(treeModel, glm::vec3(15.0f, 15.0f, 15.0f));

        glm::mat4 treeMVP = projView * treeModel;
        glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(treeMVP));
        tree_.draw();
        glEnable(GL_CULL_FACE);
    }
    
    void drawGround(glm::mat4& projView)
    {
        glUniform4f(colorModUniformLocation_, 1.0f, 1.0f, 1.0f, 1.0f);

        // offset ground by -0.1 to avoid overlap (depth buffer doesnt know which surface is in front)
        glm::mat4 grassModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f));
        grassModel = glm::scale(grassModel, glm::vec3(50.0f, 1.0f, 50.0f));

        glm::mat4 grassMVP = projView * grassModel;
        glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(grassMVP));
        grass_.draw();

        const float ROAD_OFFSET = 20.0f;
        const float ROAD_SPACING = 5.0f;
        const int N_ROAD_SEGMENT = 7;

        for (int side = 0; side < 4; ++side) {
            float angle = glm::radians(90.0f * side);

            for (int i = 0; i < N_ROAD_SEGMENT; ++i) {
                float segmentPos = (i - (N_ROAD_SEGMENT / 2)) * ROAD_SPACING;

                glm::mat4 roadModel = glm::mat4(1.0f);
                roadModel = glm::rotate(roadModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                roadModel = glm::translate(roadModel ,glm::vec3(segmentPos, 0.0f, ROAD_OFFSET));
                roadModel = glm::scale(roadModel, glm::vec3(5.0f, 1.0f, 5.0f));

                glm::mat4 roadMVP = projView * roadModel;
                glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(roadMVP));
                street_.draw();
            }
        }

        for (int side = 0; side < 4; ++side) {
            float angle = glm::radians(90.0f * side);

            glm::mat4 cornerModel = glm::mat4(1.0f);
            cornerModel = glm::rotate(cornerModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            cornerModel = glm::translate(cornerModel, glm::vec3(ROAD_OFFSET, 0.0f, ROAD_OFFSET));
            cornerModel = glm::scale(cornerModel, glm::vec3(5.0f, 1.0f, 5.0f));

            glm::mat4 cornerMVP = projView * cornerModel;
            glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(cornerMVP));
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
    
    glm::mat4 getPerspectiveProjectionMatrix()
    {
        float fov = glm::radians(70.0f);
        float aspect = getWindowAspect();
        float near = 0.1f;
        float far = 300.0f;
        
        return glm::perspective(fov, aspect, near, far);
    }
    
    void sceneModels()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderFloat("Car speed", &car_.speed, -10.0f, 10.0f, "%.2f m/s");
        ImGui::SliderFloat("Steering Angle", &car_.steeringAngle, -30.0f, 30.0f, "%.2f°");
        if (ImGui::Button("Reset steering"))
            car_.steeringAngle = 0.f;
        ImGui::Checkbox("Headlight", &car_.isHeadlightOn);
        ImGui::Checkbox("Left Blinker", &car_.isLeftBlinkerActivated);
        ImGui::Checkbox("Right Blinker", &car_.isRightBlinkerActivated);
        ImGui::Checkbox("Brake", &car_.isBraking);
        ImGui::End();
    
        updateCameraInput();
        car_.update(deltaTime_);
        
        glm::mat4 view = getViewMatrix();
        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 projView = proj * view;

        glUseProgram(transformSP_);
        drawGround(projView);
        drawTree(projView);
        drawStreetlights(projView);
        car_.draw(projView);
    }
    
private:
    // Shaders
    GLuint basicSP_;
    GLuint transformSP_;
    GLuint colorModUniformLocation_;
    GLuint mvpUniformLocation_;
    
    // Partie 1
    GLuint vbo_, ebo_, vao_;
    
    static constexpr unsigned int MIN_N_SIDES = 5;
    static constexpr unsigned int MAX_N_SIDES = 12;
    
    Vertex vertices_[MAX_N_SIDES + 1];
    GLuint elements_[MAX_N_SIDES * 3];
    
    int nSide_, oldNSide_;
    
    // Partie 2
    Model tree_;
    Model streetlight_;
    Model grass_;
    Model street_;
    Model streetcorner_;
    
    Car car_;
    
    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;
    
    // Matrices statiques
    static constexpr unsigned int N_STREETLIGHTS = 2*4;
    static constexpr unsigned int N_STREET_PATCHES = 7*4+4;
    glm::mat4 treeModelMatrice_;
    glm::mat4 groundModelMatrice_;
    glm::mat4 streetlightModelMatrices_[N_STREETLIGHTS];
    glm::mat4 streetPatchesModelMatrices_[N_STREET_PATCHES];
    
    // Imgui var
    const char* const SCENE_NAMES[2] = {
        "Introduction",
        "3D Model & transformation",
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
	app.run(argc, argv, "Tp1", settings);
}