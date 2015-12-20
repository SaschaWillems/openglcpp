/*
* OpenGL instanced mesh rendering
*
* Copyright (C) 2015 by Sascha Willems - www.saschawillems.de
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

#include "glRenderer.h"

using namespace std;

#define deg_to_rad(deg) deg * float(M_PI / 180)
#define INSTANCING_RANGE 3

struct UboInstanceData {
	// Model matrix for each instance
	glm::mat4 model;
	// Color for each instance
	// vec4 is used due to memory alignment 
	// GPU aligns at 16 bytes
	glm::vec4 color;
};

struct {
	// Global matrices
	struct {
		glm::mat4 projection;
		glm::mat4 view;
	} matrices;
	// Seperate data for each instance
} uboVS;

std::array<UboInstanceData, (INSTANCING_RANGE * 2 + 1)*(INSTANCING_RANGE * 2 + 1)*(INSTANCING_RANGE * 2 + 1)> uboInstance;

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
	glBindAttribLocation(program, 1, "inNormal");

	glLinkProgram(program);
	printProgramLog(program);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBOInst);

	glUseProgram(program);

	return program;
}

void glRenderer::generateShaders()
{
	shader = loadShader("../data/shader/mesh.vert", "../data/shader/mesh.frag");
}

void glRenderer::updateUBO()
{
	// Update ubo

	// Projection
	uboVS.matrices.projection = glm::perspective(deg_to_rad(60.0f), (float)1280 / (float)720, 0.001f, 256.0f);

	// View
	float zoom = -36.0f;
	glm::vec3 rotation = glm::vec3(20.0f, -45.0f, 0.0f);
	uboVS.matrices.view = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom));
	uboVS.matrices.view = glm::rotate(uboVS.matrices.view, deg_to_rad(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	uboVS.matrices.view = glm::rotate(uboVS.matrices.view, deg_to_rad(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	uboVS.matrices.view = glm::rotate(uboVS.matrices.view, deg_to_rad(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, &uboVS, sizeof(uboVS));
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	// Instanced part

	// Colors and model matrices are fixed
	float offset = 5.0f;
	uint32_t index = 0;
	for (int32_t x = -INSTANCING_RANGE; x <= INSTANCING_RANGE; x++)
	{
		for (int32_t y = -INSTANCING_RANGE; y <= INSTANCING_RANGE; y++)
		{
			for (int32_t z = -INSTANCING_RANGE; z <= INSTANCING_RANGE; z++)
			{
				// Instance model matrix
				uboInstance[index].model = glm::translate(glm::mat4(), glm::vec3(x * offset, y * offset, z * offset));
				uboInstance[index].model = glm::rotate(uboInstance[index].model, deg_to_rad(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				// Instance color (randomized)
				uboInstance[index].color = glm::vec4(
					(float)(rand() % 255) / 255.0f, 
					(float)(rand() % 255) / 255.0f, 
					(float)(rand() % 255) / 255.0f, 
					1.0);
				index++;
			}
		}
	}

	uint32_t uboSize = uboInstance.size() * sizeof(UboInstanceData);

	glBindBuffer(GL_UNIFORM_BUFFER, UBOInst);
	p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, uboInstance.data(), uboSize);
	glUnmapBuffer(GL_UNIFORM_BUFFER);

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

	demoMesh = new MeshLoader();
	demoMesh->LoadMesh("../data/angryteapot.X");

	float scale = 0.05f;
	std::vector<glm::vec3> vPos;
	std::vector<glm::vec3> vNorm;
	for (int m = 0; m < demoMesh->m_Entries.size(); m++)
	{
		for (int i = 0; i < demoMesh->m_Entries[m].Vertices.size(); i++)
		{
			vPos.push_back(demoMesh->m_Entries[m].Vertices[i].m_pos * scale);
			vNorm.push_back(demoMesh->m_Entries[m].Vertices[i].m_normal);
		}
	}
	uint32_t vBufferSize = vPos.size() * sizeof(glm::vec3);

	std::vector<UINT32> indexBuffer;
	for (int m = 0; m < demoMesh->m_Entries.size(); m++)
	{
		int indexBase = indexBuffer.size();
		for (int i = 0; i < demoMesh->m_Entries[m].Indices.size(); i++) {
			indexBuffer.push_back(demoMesh->m_Entries[m].Indices[i] + indexBase);
		}
	}
	int iBufferSize = indexBuffer.size() * sizeof(UINT32);
	indices = indexBuffer.size();

	glGenBuffers(2, VBO);
	glGenBuffers(1, &IBO);

	// Position data
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, vBufferSize, vPos.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	// Normal data
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, vBufferSize, vNorm.data(), GL_STATIC_DRAW);
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

	// Instancing ubo
	instanceCount = pow((INSTANCING_RANGE * 2) + 1, 3);

	glGenBuffers(1, &UBOInst);
	glBindBuffer(GL_UNIFORM_BUFFER, UBOInst);
	glBufferData(GL_UNIFORM_BUFFER, uboInstance.size() * sizeof(UboInstanceData), uboInstance.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	updateUBO();

	delete(demoMesh);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);

}

void glRenderer::renderScene()
{
	double frameTimeStart = glfwGetTime();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawElementsInstanced(
		GL_TRIANGLES,
		indices,
		GL_UNSIGNED_INT,
		(void*)0,
		instanceCount
		);

	glfwSwapBuffers(window);
}

void glRenderer::keyCallback(int key, int scancode, int action, int mods)
{
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
