/*
* OpenGL 3.3 basic triangle rendering
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

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
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glRenderer.h"

struct {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
} uboVS;

std::string readFile(const char *fileName) 
{
	std::string fileContent;
	std::ifstream fileStream(fileName, std::ios::in);
	if (!fileStream.is_open()) {
		printf("File %s not found\n", fileName);
		return "";
	}
	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		fileContent.append(line + "\n");
	}
	fileStream.close();
	return fileContent;
}

glRenderer::glRenderer()
{
}

glRenderer::~glRenderer()
{
}

void glRenderer::printProgramLog(GLuint program)
{
	GLint result = GL_FALSE;
	int logLength;

	glGetProgramiv(program, GL_LINK_STATUS, &result);
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

	// Bind vertex attributes to VBO indices
	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");

	glLinkProgram(program);
	printProgramLog(program);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);

	glUseProgram(program);

	return program;
}

void glRenderer::generateShaders()
{
	shader = loadShader("../data/shader/triangle.vert", "../data/shader/triangle.frag");
}

void glRenderer::updateUBO()
{
	// Update ubo
	// Update matrices
	uboVS.projection = glm::perspective(glm::radians(60.0f), (float)1280 / (float)720, 0.1f, 256.0f);

	uboVS.view = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom));

	uboVS.model = glm::mat4();
	uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));


	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, &uboVS, sizeof(uboVS));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void glRenderer::generateBuffers()
{
	// Default VAO needed for OpenGL 3.3+ core profiles
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Setup vertex data
	const GLfloat vPos[] = {
		 1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f
	};
	const GLfloat vCol[] = { 
		1.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 1.0f 
	};
	uint32_t vBufferSize = 9 * sizeof(float);

	// Setup indices
	std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
	indices = static_cast<uint32_t>(indexBuffer.size());
	uint32_t iBufferSize = indices * sizeof(uint32_t);

	glGenBuffers(2, VBO);
	glGenBuffers(1, &IBO);

	// Position
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, vBufferSize, &vPos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	// Color
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, vBufferSize, &vCol, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	// Indices 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iBufferSize, indexBuffer.data(), GL_STATIC_DRAW);

	// Uniform buffer object
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uboVS), &uboVS, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	updateUBO();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
}

void glRenderer::renderScene(double deltaT)
{
	double frameTimeStart = glfwGetTime();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLES, 0, 9);

	glfwSwapBuffers(window);

	rotation.y += deltaT * 50.0f;
	updateUBO();
}

void glRenderer::keyCallback(int key, int scancode, int action, int mods)
{
	// nothing here yet
}
