#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h> 

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "glRenderer.h"

using namespace std;

struct vertex3f {
	GLfloat x, y, z, w;
};


string readFile(const char *fileName) 
{
	string fileContent;
	ifstream fileStream(fileName, ios::in);
	if (!fileStream.is_open()) {
		printf("File %s not found\n", fileName);
		return "";
	}
	string line = "";
	while (!fileStream.eof()) {
		getline(fileStream, line);
		fileContent.append(line + "\n");
	}
	fileStream.close();
	return fileContent;
}

GLuint loadBMPTexture(const char * fileName) 
{
	FILE * bmpFile = fopen(fileName, "rb");
	if (!bmpFile)
	{ 
		printf("Could not load bitmap from %s\n", fileName); 
		return 0;
	}

	unsigned char* bmpHeader = new unsigned char[54];
	if (fread(bmpHeader, 1, 54, bmpFile) != 54){
		printf("Headersize does not fit BMP!\n");
		return 0;
	}

	unsigned int width = *(int*)&(bmpHeader[0x12]);
	unsigned int height = *(int*)&(bmpHeader[0x16]);
	unsigned int dataPos = *(int*)&(bmpHeader[0x0A]) != 0 ? *(int*)&(bmpHeader[0x0A]) : 54;
	unsigned int imageSize = *(int*)&(bmpHeader[0x22]) != 0 ? *(int*)&(bmpHeader[0x22]) : width * height * 3;

	unsigned char* bmpData = new unsigned char[imageSize];
	fread(bmpData, 1, imageSize, bmpFile);
	fclose(bmpFile);

	// Generate OpenGL texture (and generate mipmaps
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, bmpData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	return textureID;
}

float random(float fMin, float fMax)
{
	float fRandNum = (float)rand() / RAND_MAX;
	return fMin + (fMax - fMin) * fRandNum;
}

glRenderer::glRenderer()
{
	srand(time(NULL));
	color[0] = random(64, 255);
	color[1] = random(64, 255);
	color[2] = random(64, 255);
	colorChangeTimer = 1000.0f;
}


glRenderer::~glRenderer()
{
}

void glRenderer::printProgramLog(GLuint program)
{
	GLint result = GL_FALSE;
	int logLength;

	glGetProgramiv(program, GL_COMPILE_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		GLchar* strInfoLog = new GLchar[logLength + 1];
		glGetProgramInfoLog(program, logLength, NULL, strInfoLog);
		printf("programlog: %s\n", strInfoLog);
	};
}

void glRenderer::printShaderLog(GLuint shader)
{
	GLint result = GL_FALSE;
	int logLength;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		GLchar* strInfoLog = new GLchar[logLength + 1];
		glGetShaderInfoLog(shader, logLength, NULL, strInfoLog);
		printf("shaderlog: %s\n", strInfoLog);
	};
}


GLuint glRenderer::loadShader(const char* vertexShaderFile, const char* fragmentShaderFile)
{
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shaders
	std::string vertShaderStr = readFile(vertexShaderFile);
	std::string fragShaderStr = readFile(fragmentShaderFile);
	const char *vertShaderSrc = vertShaderStr.c_str();
	const char *fragShaderSrc = fragShaderStr.c_str();

	GLint result = GL_FALSE;
	int logLength;

	// Compile vertex shader
	std::cout << "Compiling vertex shader." << std::endl;
	glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
	glCompileShader(vertShader);
	printShaderLog(vertShader);

	// Compile fragment shader
	std::cout << "Compiling fragment shader." << std::endl;
	glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
	glCompileShader(fragShader);
	printShaderLog(fragShader);

	std::cout << "Linking program" << std::endl;
	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	printProgramLog(program);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}

GLuint glRenderer::loadComputeShader(const char* computeShaderFile) 
{
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

	// Read shaders
	std::string computeShaderStr = readFile(computeShaderFile);
	const char *computeShaderSrc = computeShaderStr.c_str();

	GLint result = GL_FALSE;
	int logLength;

	std::cout << "Compiling compute shader." << std::endl;
	glShaderSource(computeShader, 1, &computeShaderSrc, NULL);
	glCompileShader(computeShader);

	printShaderLog(computeShader);

	std::cout << "Linking program" << std::endl;
	GLuint program = glCreateProgram();
	glAttachShader(program, computeShader);
	glLinkProgram(program);
	printProgramLog(program);
	
	glDeleteShader(computeShader);

	return program;

}

void glRenderer::generateShaders()
{
	baseshader = loadShader("data/shader/vertex.shader", "data/shader/fragment.shader");
	computeshader = loadComputeShader("data/shader/particlesystem.shader");
}

void glRenderer::resetPositionSSBO()
{

	// Reset to mouse cursor pos
	double cursorX, cursorY;
	int windowWidth, windowHeight;
	glfwPollEvents();
	glfwGetCursorPos(window, &cursorX, &cursorY);
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	float destPosX = (float)(cursorX / (windowWidth)-0.5f) * 2.0f;
	float destPosY = (float)((windowHeight - cursorY) / windowHeight - 0.5f) * 2.0f;

	struct vertex3f* verticesPos = (struct vertex3f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particleCount * sizeof(vertex3f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < particleCount; i++) {
		float rnd = (float)rand() / (float)(RAND_MAX);
		float rndVal = (float)rand() / (float)(RAND_MAX / (360.0f * 3.14f * 2.0f));
		float rndRad = (float)rand() / (float)(RAND_MAX)* 0.2f; // TODO : Change multiplier to get cool effects (e.g. wider range)
		verticesPos[i].x = destPosX + cos(rndVal) * rndRad;
		verticesPos[i].y = destPosY + sin(rndVal) * rndRad;
		verticesPos[i].z = 0.0f;
		verticesPos[i].w = 1.0f;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void glRenderer::resetVelocitySSBO()
{
	struct vertex3f* verticesVel = (struct vertex3f*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particleCount * sizeof(vertex3f), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < particleCount; i++) {
		verticesVel[i].x = 0.0f;
		verticesVel[i].y = 0.0f;
		verticesVel[i].z = 0.0f;
		verticesVel[i].w = 1.0f;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void glRenderer::resetBuffers()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOPos);
	resetPositionSSBO();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOVel);
	resetVelocitySSBO();
}

void glRenderer::generateBuffers()
{
	// Position SSBO
	if (glIsBuffer(SSBOPos)) {
		glDeleteBuffers(1, &SSBOPos);
	};
	glGenBuffers(1, &SSBOPos);
	// Bind to SSBO
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOPos);
	// Generate empty storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleCount * sizeof(vertex3f), NULL, GL_STATIC_DRAW);
	// Fill
	resetPositionSSBO();
	// Bind buffer to target index 0
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBOPos);

	// Velocity SSBO
	if (glIsBuffer(SSBOVel)) {
		glDeleteBuffers(1, &SSBOVel);
	};
	glGenBuffers(1, &SSBOVel);
	// Bind to SSBO
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOVel);
	// Generate empty storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleCount * sizeof(vertex3f), NULL, GL_STATIC_DRAW);
	// Fill
	resetVelocitySSBO();
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	// Bind buffer to target index 1
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBOVel);

}

void glRenderer::generateTextures()
{
	particleTex = loadBMPTexture("data/particle.bmp");
}

void glRenderer::renderScene()
{

	if (colorFade)
	{

		colorChangeTimer -= frameDelta * 25.0f;

		if ((colorChangeTimer <= 0.0f) & (colorChangeLength <= 0.0f)) {
			if (random(0.0f, 100.0f) < 50.0f)
			{
				colVec[0] = (int)random(0.0f, 8.0f) * 32.0f - color[0];
				colVec[1] = (int)random(0.0f, 8.0f) * 32.0f - color[1];
				colVec[2] = (int)random(0.0f, 8.0f) * 32.0f - color[2];

				float vlength = sqrt(colVec[0] * colVec[0] + colVec[1] * colVec[1] + colVec[2] * colVec[2]);
				colorChangeLength = vlength * 2.0f;

				colVec[0] = colVec[0] / vlength;
				colVec[1] = colVec[2] / vlength;
				colVec[2] = colVec[2] / vlength;

			};
			colorChangeTimer = 1000.0f;
		}

		if (colorChangeLength > 0.0f)
		{
			color[0] += colVec[0] * frameDelta;
			color[1] += colVec[1] * frameDelta;
			color[2] += colVec[2] * frameDelta;

			colorChangeLength -= frameDelta;
		}

	}
	else 
	{
		color[0] = 255.0f;
		color[1] = 64.0f;
		color[2] = 0.0f;
	}


	double frameTimeStart = glfwGetTime();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// Run compute shader

	double cursorX, cursorY;
	int windowWidth, windowHeight;
	glfwGetCursorPos(window, &cursorX, &cursorY);
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	float destPosX = (float)(cursorX / (windowWidth) - 0.5f) * 2.0f;
	float destPosY = (float)((windowHeight - cursorY) / windowHeight - 0.5f) * 2.0f;

	glUseProgram(computeshader);
	glUniform1f(glGetUniformLocation(computeshader, "deltaT"), frameDelta * speedMultiplier * (pause ? 0.0f : 1.0f));
	glUniform3f(glGetUniformLocation(computeshader, "destPos"), destPosX, destPosY, 0);
	glUniform2f(glGetUniformLocation(computeshader, "vpDim"), 1, 1);
	glUniform1i(glGetUniformLocation(computeshader, "borderClamp"), (int)borderEnabled);

	int workingGroups = particleCount / 16;

	glDispatchCompute(workingGroups, 1, 1);

	glUseProgram(0);

	// Set memory barrier on per vertex base to make sure we get what was written by the compute shaders
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

	// Render scene

	glUseProgram(baseshader);

	glUniform4f(glGetUniformLocation(baseshader, "inColor"), color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f, 1.0f);

	glGetError();

	glBindTexture(GL_TEXTURE_2D, particleTex);

	GLuint posAttrib = glGetAttribLocation(baseshader, "pos");

	glBindBuffer(GL_ARRAY_BUFFER, SSBOPos);
	glVertexAttribPointer(posAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);
	glPointSize(16);
	glDrawArrays(GL_POINTS, 0, particleCount);

	glfwSwapBuffers(window);

	frameDelta = (float)(glfwGetTime() - frameTimeStart) * 100.0f;

}

void glRenderer::keyCallback(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		resetBuffers();
	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)
		speedMultiplier += 0.025f;
	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)
		speedMultiplier -= 0.025f;
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
		borderEnabled = !borderEnabled;
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		colorFade = !colorFade;
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		pause = !pause;
	if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS)
	{
		particleCount += 1024;
		printf("particle count : %d\nRegenerating SSBOs...\n", particleCount);
		generateBuffers();
	}
	if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS && particleCount > 1024)
	{
		particleCount -= 1024;
		printf("particle count : %d\nRegenerating SSBOs...\n", particleCount);
		generateBuffers();
	}
}
