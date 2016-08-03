/*
* OpenGL ray picking
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h> 
#include <iomanip>
#include <chrono>

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
#include <glm/gtx/transform.hpp>

#include "glRenderer.h"

// Defines
// If set, project vertices into screen space
// It not set, unproject mouse pos into world space (faster) 
// #ifdef INTERSECT_VERTEX_PROJECT
// Use GLM for unproject
//	#define INTERSECT_UNPROJECT_GLM
// Control triangle display (pyramid, overlapping)
//	#define TRIANGLE_PYRAMID

struct {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	int32_t selected = -1;
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
	shader = loadShader("../data/shader/raypicking.vert", "../data/shader/raypicking.frag");
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

#ifdef TRIANGLE_PYRAMID
	// Pyramid
	triangleData.push_back({  0.5f, 0.0f, 0.0f });
	triangleData.push_back({ -0.5f, 0.0f, 0.0f });
	triangleData.push_back({  0.0f, 1.0f, 0.0f });

	triangleData.push_back({ 0.0f, -1.0f, 0.0f });
	triangleData.push_back({ -1.0f, -1.0f, 0.0f });
	triangleData.push_back({ -0.5f, 0.0f, 0.0f });

	triangleData.push_back({ 1.0f, -1.0f, 0.0f });
	triangleData.push_back({ 0.0f, -1.0f, 0.0f });
	triangleData.push_back({ 0.5f, 0.0f, 0.0f });
#else
	// Overlapping
	for (int32_t i = -1; i < 2; i++)
	{
		triangleData.push_back({ 0.5f, -0.5f, float(i)*0.5f });
		triangleData.push_back({ -0.5f, -0.5f, float(i)*0.5f });
		triangleData.push_back({ 0.0f, 0.5f, float(i)*0.5f });
	}
#endif

	uint32_t vBufferSize = static_cast<uint32_t>(triangleData.size()) * sizeof(glm::vec3);

	// Setup indices
	std::vector<uint32_t> indexBuffer = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	indexCount = static_cast<uint32_t>(indexBuffer.size());
	uint32_t iBufferSize = indexCount * sizeof(uint32_t);

	glGenBuffers(2, VBO);
	glGenBuffers(1, &IBO);

	// Position
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, vBufferSize, triangleData.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

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

	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLES, 0, indexCount);

	glfwSwapBuffers(window);
	if (!paused)
	{
		rotation.y += deltaT * 50.0f;
	}
	updateUBO();
}

void glRenderer::keyCallback(int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_KP_ADD:
			zoom += 0.1f;
			break;
		case GLFW_KEY_KP_SUBTRACT:
			zoom -= 0.1f;
			break;
		case GLFW_KEY_P:
			paused = !paused;
			break;
		case GLFW_KEY_T:
			pickingType = !pickingType;
			std::cout << "pickingType = " << pickingType << std::endl;
			break;
		}
	}
}

// Möller–Trumbore intersection algorithm
static int rayIntersectsTriangle(glm::vec3 origin, glm::vec3 dir, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, float* intersection)
{
	// Triangle edges
	glm::vec3 e1(v1 - v0);
	glm::vec3 e2(v2 - v0);

	const float epsilon = 0.000001f;

	glm::vec3 P, Q;
	float i;
	float t;

	// Calculate determinant
	P = glm::cross(dir, e2);
	float det = glm::dot(e1, P);
	// If determinant is (close to) zero, the ray lies in the plane of the triangle or parallel it's plane
	if ((det > -epsilon) && (det < epsilon))
	{
		return 0;
	}	
	float invDet = 1.0f / det;

	// Distance from first vertex to ray origin
	glm::vec3 T = origin - v0;

	// Calculate u parameter
	float u = glm::dot(T, P) * invDet;
	// Intersection point lies outside of the triangle
	if ((u < 0.0f) || (u > 1.0f))
	{
		return 0;
	}

	//Prepare to test v parameter
	Q = glm::cross(T, e1);

	// Calculate v parameter
	float v = glm::dot(dir, Q) * invDet;
	// Intersection point lies outside of the triangle
	if (v < 0.f || u + v  > 1.f) return 0;

	// Calculate t
	t = glm::dot(e2, Q) * invDet;

	if (t > epsilon)
	{ 
		// Triangle interesected
		if (intersection)
		{
			*intersection = t;
		}
		return true;
	}

	// No intersection
	return false;
}

void glRenderer::mouseButtonCallback(int button, int action)
{
	int w, h;
	double mx, my;
	uint32_t numIntersections = 0;

	glfwGetCursorPos(window, &mx, &my);
	glfwGetWindowSize(window, &w, &h);

	struct
	{
		int32_t index = -1;
		float lastPos = std::numeric_limits<float>::max();
	} intersection;

	if ((action == GLFW_PRESS) && (button == GLFW_MOUSE_BUTTON_1))
	{
		auto tStart = std::chrono::high_resolution_clock::now();

		if (pickingType == PICKING_TYPE_PROJECT)
		{
			// (Slow) Variant: Project all vertices into screen space and shoot ray from mouse screen pos along z
			std::cout << "Using vertex project" << std::endl;

			mx /= static_cast<float>(w);
			my /= static_cast<float>(h);
			my = 1.0 - my;

			std::vector<glm::vec3> projected;
			for (auto vertex : triangleData)
			{
				// Project
				glm::vec4 proj = uboVS.projection * uboVS.view * uboVS.model * glm::vec4(vertex, 1.0f);
				proj /= proj.w;
				proj = proj * 0.5f + 0.5f;
				projected.push_back(glm::vec3(proj));
			}
			glm::vec3 origin = glm::vec3(static_cast<float>(mx), static_cast<float>(my), 0.0f);
			for (uint32_t i = 0; i < indexCount / 3; i++)
			{
				float currIntersectionPos;
				if (rayIntersectsTriangle(origin, glm::vec3(0.0f, 0.0f, 1.0f), projected[i * 3], projected[i * 3 + 1], projected[i * 3 + 2], &currIntersectionPos))
				{
					if (currIntersectionPos < intersection.lastPos)
					{
						intersection.lastPos = currIntersectionPos;
						intersection.index = i;
					}
					std::cout << "\tIntersection " << numIntersections << ": Triangle " << i << " intersected at ray pos " << currIntersectionPos << std::endl;
					numIntersections++;
				}
			}
		}
		else
		{
			// Fast Variant: Project mouse pos on near and far into world space
			std::cout << "Using mouse unproject" << std::endl;

			// Simple principle:
			//	- Project normalized screen space mouse position into world space
			//		- On near plane
			//		- On far plane
			//	- Use near plane world pas as origin
			//	- Calculate normalized ray dir using near and far pos

			glm::vec4 viewport = glm::vec4(0.0f, 0.0f, w, h);

#ifdef INTERSECT_UNPROJECT_GLM
			// Mouse world pos on near plane
			glm::vec3 worldNear = glm::unProject(glm::vec3(float(mx), float(h - my), 0.0f), uboVS.view * uboVS.model, uboVS.projection, viewport);
			// Mouse world pos on far plane
			glm::vec3 worldFar = glm::unProject(glm::vec3(float(mx), float(h - my), 1.0f), uboVS.view * uboVS.model, uboVS.projection, viewport);
#else
			glm::mat4 inverse = glm::inverse(uboVS.projection * uboVS.view * uboVS.model);

			// Mouse world pos on near plane
			glm::vec4 worldNear(float(mx), float(h - my), 0.0f, 1.0f);
			worldNear.x = ((worldNear.x - viewport[0]) / viewport[2]);
			worldNear.y = ((worldNear.y - viewport[1]) / viewport[3]);
			worldNear = worldNear * 2.0f - 1.0f;
			worldNear = inverse * worldNear;
			worldNear /= worldNear.w;

			// Mouse world pos on far plane
			glm::vec4 worldFar(float(mx), float(h - my), 1.0f, 1.0f);
			worldFar.x = ((worldFar.x - viewport[0]) / viewport[2]);
			worldFar.y = ((worldFar.y - viewport[1]) / viewport[3]);
			worldFar = worldFar * 2.0f - 1.0f;
			worldFar = inverse * worldFar;
			worldFar /= worldFar.w;
#endif
			// Get ray between pos on near and far plane
			glm::vec3 rayDir = glm::normalize(worldFar - worldNear);

			for (uint32_t i = 0; i < indexCount / 3; i++)
			{
				float currIntersectionPos;
				if (rayIntersectsTriangle(worldNear, rayDir, triangleData[i * 3], triangleData[i * 3 + 1], triangleData[i * 3 + 2], &currIntersectionPos))
				{
					if (currIntersectionPos < intersection.lastPos)
					{
						intersection.lastPos = currIntersectionPos;
						intersection.index = i;
					}
					std::cout << "\tIntersection " << numIntersections << ": Triangle " << i << " intersected at ray pos " << currIntersectionPos << std::endl;
					numIntersections++;
				}
			}

		}

		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		std::cout << "duration: " << std::setprecision(4) << tDiff << "ms" << std::endl;

		// Pass selection to shader for higlighting
		uboVS.selected = intersection.index;
		updateUBO();
	}

}
