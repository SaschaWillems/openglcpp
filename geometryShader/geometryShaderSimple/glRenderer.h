/*
This code is licensed under the Mozilla Public License Version 2.0 (http://opensource.org/licenses/MPL-2.0)
© 2014 by Sascha Willems - http://www.saschawillems.de
*/

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class glRenderer
{
private:
	GLuint shaderSimple;
	GLuint shaderGeometry;
	GLuint VBO[2];
	bool useGeometryShader = true;
	bool wireframe = true;
	float circleRadius = 0.3f;
	float circleDivisions = 4;
	GLuint loadShader(const char* vertexShaderFile, const char* fragmentShaderFile, const char* geometryShader);
	void printProgramLog(GLuint shader);
	void printShaderLog(GLuint program);
public:
	GLFWwindow* window;
	glRenderer();
	~glRenderer();
	void generateShaders();
	void generateBuffers();
	void renderScene();
	void keyCallback(int key, int scancode, int action, int mods);
};

