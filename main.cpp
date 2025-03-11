#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

////lightning
#include <chrono>
#include <thread>
#include <AudioToolbox/AudioToolbox.h>

void playThunderSound() {
	CFURLRef soundFileURLRef = CFURLCreateWithFileSystemPath(
		kCFAllocatorDefault,
		CFSTR("/Users/codrutajucan/Desktop/AN3/GP/proiectgp/loud-thunder-192165.caf"),
		kCFURLPOSIXPathStyle,
		false
	);

	SystemSoundID soundID;
	AudioServicesCreateSystemSoundID(soundFileURLRef, &soundID);
	AudioServicesPlaySystemSound(soundID);
	CFRelease(soundFileURLRef);
}

// Lightning Variables
float lightningTimer = 0.0f;
bool lightningActive = false;
float lightningIntensityMultiplier = 10.0f;
float lightningDuration = 0.1f;
float lightningCooldown = 5.0f;
float lastLightningTime = 0.0f;
void triggerLightning() {
	if (!lightningActive) {
		lightningActive = true;
		lightningTimer = 0.0f;
		playThunderSound();
		std::cout << "Lightning flash triggered!" << std::endl;
	}
}

///

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 3.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
///gps::Model3D teapot;
gps::Model3D scene;
gps::Model3D fairy;
GLfloat angle;

// shaders
gps::Shader myBasicShader;

//wireframe
bool wireframe = false;

//animation
float fairyAngle = 0.0f;

//fog
float fogDensity = 0.0f;

//autotour
std::vector<glm::vec3> tourPositions = {
	glm::vec3(0.0f, 3.0f, 10.0f),
	glm::vec3(5.0f, 5.0f, 5.0f),
	glm::vec3(-5.0f, 2.0f, -10.0f),
	glm::vec3(0.0f, 8.0f, -15.0f),
	glm::vec3(10.0f, 3.0f, 0.0f)
};

std::vector<glm::vec3> tourTargets = {
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(3.0f, 1.0f, -5.0f),
	glm::vec3(-3.0f, 0.0f, -5.0f),
	glm::vec3(2.0f, 2.0f, -10.0f),
	glm::vec3(0.0f, 0.0f, 0.0f)
};

int currentSegment = 0;
float t = 0.0f;
bool autoTour = false;


//pointlight
glm::vec3 pointLightPosition(1.9f, 0.607f,-4.0f);
glm::vec3 pointLightColor(1.0f, 1.0f, 1.0f);


//lights
bool directionalLightEnabled = true;
bool pointLightEnabled = true;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


//for resizing
bool cursorEnabled = false;

void toggleCursorMode(GLFWwindow* window) {
	cursorEnabled = !cursorEnabled;
	if (cursorEnabled) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	} else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

int width = 1024;
int height = 768;

void windowResizeCallback(GLFWwindow* window, int newWidth, int newHeight) {
	// Update global width and height
	width = newWidth;
	height = newHeight;

	// Set the new viewport size
	glViewport(0, 0, width, height);

	// Update the window dimensions in your custom Window class
	WindowDimensions dim;
	dim.width = width;
	dim.height = height;
	myWindow.setWindowDimensions(dim);

	// Update the projection matrix to reflect the new aspect ratio
	projection = glm::perspective(glm::radians(45.0f),
								  static_cast<float>(width) / static_cast<float>(height),
								  0.1f, 1000.0f);

	// Send the updated projection matrix to the shader
	myBasicShader.useShaderProgram();
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Log the new dimensions
	fprintf(stdout, "Window resized! New width: %d, and height: %d\n", width, height);
}





void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;

			if (pressedKeys[GLFW_KEY_R]) {
				wireframe = !wireframe;
				if (wireframe == false) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				} else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
			}

			if (pressedKeys[GLFW_KEY_UP]) {
				fogDensity += 0.01f;
				if (fogDensity > 1.0f) fogDensity = 1.0f;
				std::cout << "Fog Density Increased: " << fogDensity << std::endl;
			}
			if (pressedKeys[GLFW_KEY_DOWN]) {
				fogDensity -= 0.01f;
				if (fogDensity < 0.0f) fogDensity = 0.0f;
				std::cout << "Fog Density Decreased: " << fogDensity << std::endl;
			}
			if (key == GLFW_KEY_T && action == GLFW_PRESS) {
				autoTour = !autoTour;
				if (autoTour) {
					currentSegment = 0;
					t = 0.0f;
					std::cout << "Starting Automated Tour." << std::endl;
				}
			}
			if (key == GLFW_KEY_L && action == GLFW_PRESS) {
				directionalLightEnabled = !directionalLightEnabled;
				std::cout << "Directional Light " << (directionalLightEnabled ? "Enabled" : "Disabled") << std::endl;
			}

			if (key == GLFW_KEY_P && action == GLFW_PRESS) {
				pointLightEnabled = !pointLightEnabled;
				std::cout << "Point Light " << (pointLightEnabled ? "Enabled" : "Disabled") << std::endl;
			}
			if (key == GLFW_KEY_C && action == GLFW_PRESS) { // Press 'C' to toggle cursor mode
				toggleCursorMode(window);
			}
		} else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}
}


void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	static float lastX = xpos;
	static float lastY = ypos;
	static bool firstMouse = true;

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;


	static float pitch = 0.0f, yaw = -90.0f;
	pitch += yOffset;
	yaw += xOffset;
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	myCamera.rotate(pitch, yaw);

	view = myCamera.getViewMatrix();
	myBasicShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}



void processMovement() {

	bool keyPressed = false;

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		keyPressed = true;
	}
	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		keyPressed = true;
	}
	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		keyPressed = true;
	}
	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		keyPressed = true;
	}
	if (pressedKeys[GLFW_KEY_Q]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
		keyPressed = true;
	}
	if (pressedKeys[GLFW_KEY_E]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
		keyPressed = true;
	}

	if (autoTour) {
		if (currentSegment < tourPositions.size() - 1) {
			glm::vec3 startPos = tourPositions[currentSegment];
			glm::vec3 endPos = tourPositions[currentSegment + 1];
			glm::vec3 startTarget = tourTargets[currentSegment];
			glm::vec3 endTarget = tourTargets[currentSegment + 1];

			glm::vec3 newPosition = glm::mix(startPos, endPos, t);
			glm::vec3 newTarget = glm::mix(startTarget, endTarget, t);

			myCamera.setPosition(newPosition);
			myCamera.setTarget(newTarget);

			t += 0.01f;
			if (t >= 1.0f) {
				t = 0.0f;
				currentSegment++;
			}
		} else {
			autoTour = false; // End tour after the last segment
			std::cout << "Automated Tour Completed." << std::endl;
		}
	}


	view = myCamera.getViewMatrix();
	myBasicShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}





void initOpenGLWindow() {
    myWindow.Create(width, height, "OpenGL Project Core");
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	static float fov = 45.0f; // Default FOV

	fov -= static_cast<float>(yOffset);
	if (fov < 1.0f) fov = 1.0f;     // Clamp to avoid extreme zoom-in
	if (fov > 90.0f) fov = 90.0f;   // Clamp to avoid extreme zoom-out

	// Update projection matrix
	projection = glm::perspective(glm::radians(fov),
								  (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
								  0.1f, 100.0f);
	myBasicShader.useShaderProgram();
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}


void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
	glfwSetScrollCallback(myWindow.getWindow(), scrollCallback); // Add this line
	glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture cursor
}


void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    //teapot.LoadModel("../models/teapot/teapot20segUT.obj");
	scene.LoadModel("../models/scene/scene.obj");
	fairy.LoadModel("../models/scene/fairy.obj");
}

void initShaders() {
	myBasicShader.loadShader(
        "../shaders/basic.vert",
        "../shaders/basic.frag");
}

void initUniforms() {
	myBasicShader.useShaderProgram();

	// Model matrix
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// View matrix
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// Normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// Projection matrix

	projection = glm::perspective(glm::radians(45.0f),
								  static_cast<float>(myWindow.getWindowDimensions().width) /
								  static_cast<float>(myWindow.getWindowDimensions().height),
								  0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// projection = glm::perspective(glm::radians(45.0f),
	// 							  (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
	// 							  0.1f, 100.0f);
	// projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Light direction and color
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // White light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	// Fog parameters
	glm::vec3 fogColor(0.7f, 0.7f, 0.7f); // Light gray fog
	float fogDensity = 0.05f; // Adjust as needed

	GLint fogColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogColor");
	glUniform3fv(fogColorLoc, 1, glm::value_ptr(fogColor));

	GLint fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
	glUniform1f(fogDensityLoc, fogDensity);



	GLint directionalLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "directionalLightEnabled");
	GLint pointLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightEnabled");

	glUniform1i(directionalLightEnabledLoc, directionalLightEnabled);
	glUniform1i(pointLightEnabledLoc, pointLightEnabled);

}

void updateLightning(float deltaTime) {
	lastLightningTime += deltaTime;

	if (lightningActive) {
		lightningTimer += deltaTime;

		if (lightningTimer >= lightningDuration) {
			lightningActive = false;
			lastLightningTime = 0.0f;
		}
	} else if (lastLightningTime >= lightningCooldown) {
		triggerLightning();
	}

	float currentIntensity = lightningActive ? lightningIntensityMultiplier : 1.0f;
	GLint lightningIntensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightningIntensity");
	glUniform1f(lightningIntensityLoc, currentIntensity);
}

void initLightning() {
	myBasicShader.useShaderProgram();
	GLint lightningIntensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightningIntensity");
	glUniform1f(lightningIntensityLoc, 1.0f);
}

void renderTeapot(gps::Shader shader) {
	shader.useShaderProgram();

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	glm::mat4 sceneModel = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sceneModel));
	scene.Draw(shader);


	fairyAngle += 0.2f;
	if (fairyAngle > 360.0f)
		fairyAngle = 0.0f;

	glm::mat4 fairyModel = glm::mat4(1.0f);
	fairyModel = glm::rotate(fairyModel, glm::radians(-fairyAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	fairyModel = glm::translate(fairyModel, glm::vec3(3.0f, 2.0f, 0.0f));
	fairyModel = glm::rotate(fairyModel, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fairyModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * fairyModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	fairy.Draw(shader);
}


void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myBasicShader.useShaderProgram();

	GLint fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
	glUniform1f(fogDensityLoc, fogDensity);

	GLint directionalLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "directionalLightEnabled");
	glUniform1i(directionalLightEnabledLoc, directionalLightEnabled);

	GLint pointLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightEnabled");
	glUniform1i(pointLightEnabledLoc, pointLightEnabled);

	GLint pointLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos");
	glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPosition));

	GLint pointLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor");
	glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

	renderTeapot(myBasicShader);
}



void cleanup() {
    myWindow.Delete();
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    	width = myWindow.getWindowDimensions().width;
    	height = myWindow.getWindowDimensions().height;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();


	glCheckError();
	// application loop
	auto lastTime = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		processMovement();

		updateLightning(deltaTime);
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
