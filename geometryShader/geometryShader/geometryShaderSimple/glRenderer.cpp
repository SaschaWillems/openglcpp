/*
This code is licensed under the Mozilla Public License Version 2.0 (http://opensource.org/licenses/MPL-2.0)
© 2014 by Sascha Willems - http://www.saschawillems.de
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

#include "glRenderer.h"

using namespace std;

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


GLuint glRenderer::loadShader(const char* vertexShaderFile, const char* fragmentShaderFile, const char* geometryShader)
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

	// Compile geometry shader
	GLuint geomShader;
	if (geometryShader != "") {
		geomShader = glCreateShader(GL_GEOMETRY_SHADER);
		std::string geomShaderStr = readFile(geometryShader);
		const char *geomShaderSrc = geomShaderStr.c_str();
		std::cout << "Compiling geometry shader." << std::endl;
		glShaderSource(geomShader, 1, &geomShaderSrc, NULL);
		glCompileShader(geomShader);
		printShaderLog(geomShader);
	}

	std::cout << "Linking program" << std::endl;
	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	if (geometryShader != "") {
		glAttachShader(program, geomShader);
	}

	// Bind vertex attributes to VBO indices
	glBindAttribLocation(program, 0, "in_Position");
	glBindAttribLocation(program, 1, "in_Color");

	glLinkProgram(program);
	printProgramLog(program);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	if (geometryShader != "") {
		glDeleteShader(geomShader);
	}

	return program;
}


void glRenderer::generateShaders()
{
	shaderSimple = loadShader("data/shader/vertex.shader", "data/shader/fragment.shader", "data/shader/geometry_passthrough.shader");
	shaderGeometry = loadShader("data/shader/vertex.shader", "data/shader/fragment.shader", "data/shader/geometry_circle.shader");
}

void glRenderer::generateBuffers()
{
	// Default VAO needed for OpenGL 3.3+ core profiles
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Position data
	const GLfloat pointPos[] = {
		-0.5f, -0.5f, 0.0f,
	 	0.5f,  -0.5f, 0.0f,
 		0.0f,   0.5f, 0.0f,
	};

	// Color data
	const GLfloat pointCol[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};


	glGenBuffers(2, VBO);

	// VBO for position data
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pointPos), pointPos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	// VBO for color data
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pointCol), pointCol, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
}

void glRenderer::renderScene()
{
	double frameTimeStart = glfwGetTime();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, (wireframe ? GL_LINE : GL_FILL));

	glUseProgram(useGeometryShader ? shaderGeometry : shaderSimple);

	glUniform1f(glGetUniformLocation(useGeometryShader ? shaderGeometry : shaderSimple, "inRadius"), circleRadius);
	glUniform1i(glGetUniformLocation(useGeometryShader ? shaderGeometry : shaderSimple, "inNumDivisions"), circleDivisions);

	glPointSize(16.0f);
	glDrawArrays(GL_POINTS, 0, 3);

	glfwSwapBuffers(window);
}

void glRenderer::keyCallback(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		useGeometryShader = !useGeometryShader;
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		wireframe = !wireframe;
	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS && mods != GLFW_MOD_SHIFT && circleDivisions < 85)
		circleDivisions += 1;
	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS && mods != GLFW_MOD_SHIFT && circleDivisions > 3)
		circleDivisions -= 1;
	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS && mods == GLFW_MOD_SHIFT)
		circleRadius += 0.025f;
	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS && mods == GLFW_MOD_SHIFT && circleRadius >= 0.1f)
		circleRadius -= 0.025f;
}
