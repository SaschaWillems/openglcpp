#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class glRenderer
{
private:
	GLuint baseshader;
	GLuint computeshader;
	GLuint SSBOPos;
	GLuint SSBOVel;
	GLuint particleTex;
	float frameDelta = 0.0f;
	float speedMultiplier = 0.15f;
	bool borderEnabled = true;
	bool colorFade = false;
	bool pause = false;
	float color[3];
	float colVec[3];
	float colorChangeTimer;
	float colorChangeLength;
	void resetPositionSSBO();
	void resetVelocitySSBO();
	GLuint loadShader(const char* vertexShaderFile, const char* fragmentShaderFile);
	GLuint loadComputeShader(const char* computeShaderFile);
	void printProgramLog(GLuint shader);
	void printShaderLog(GLuint program);
public:
	GLFWwindow* window;
	int particleCount = 1024 * 2;
	glRenderer();
	~glRenderer();
	void generateShaders();
	void generateBuffers();
	void resetBuffers();
	void generateTextures();
	void renderScene();
	void keyCallback(int key, int scancode, int action, int mods);
};

