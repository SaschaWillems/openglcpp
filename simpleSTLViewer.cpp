/*
This code is licensed under the Mozilla Public License Version 2.0 (http://opensource.org/licenses/MPL-2.0)
© 2015 by Sascha Willems - http://www.saschawillems.de

Simple stl (binary and ascii) loader and viewer using vertex array objects for rendering

Note : Requires OpenGL 4.4
*/

#include "stdafx.h"

#include <GL/glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

// Simple vertex class for learning 
// basic C++ operator overloading and stuff
struct Vertex3f {
	float x, y, z;
	Vertex3f() {
		x = 0;
		y = 0;
		z = 0;
	}
	Vertex3f(float vx, float vy, float vz) {
		x = vx;
		y = vy;
		z = vz;
	}
	inline Vertex3f operator*(float s) {
		return { x * s, y *s, z *s };
	}
	inline Vertex3f operator+(const Vertex3f& v) {
		return{ x + v.x, y + v.y, z + v.z };
	}
	inline Vertex3f operator/(float s) {
		return{ x / s, y / s, z / s };
	}
	inline Vertex3f& operator+=(Vertex3f& v) {
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		return *this;
	}
	inline Vertex3f& operator-=(Vertex3f& v) {
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
		return *this;
	}
	inline Vertex3f& operator/=(Vertex3f& v) {
		this->x /= v.x;
		this->y /= v.y;
		this->z /= v.z;
		return *this;
	}
};

struct ProgramHandles {
	GLuint aPosition;
	GLuint aNormal;
	GLuint mvMatrix;
	GLuint mvpMatrix;
	GLuint uLightPos;
	GLuint uColor;
};

Vertex3f rotation { 0.0f, 90.0f, 0.0f };

glm::mat4x4 mvMatrix;
glm::mat4x4 mvpMatrix;
glm::vec3 lightPos;

GLuint vertexBuffer;
GLuint normalBuffer;
GLuint vao;

double lastxpos = -1;
double lastypos = -1;

double frameTime = 1.0f;
float degTimer = .0f;

GLFWwindow* window;

ProgramHandles programHandles;

GLuint gProgram(0);

vector<Vertex3f> gVertices;
vector<Vertex3f> gNormals;

GLuint gVertexBuffer(0);
GLuint gNormalBuffer(0);

Vertex3f* gVertexBufferData(0);
Vertex3f* gNormalBufferData(0);

const GLchar* gVertexShaderSource[] = {
	"#version 440 core\n"
	"uniform mat4 uMVPMatrix;\n"   
	"uniform mat4 uMVMatrix;\n"   
	"uniform vec3 uLightPos;\n"   
	"uniform vec4 uColor;\n"

	"layout(location = 0) in vec3 aPosition;\n"     
	"layout(location = 1) in vec3 aNormal;\n"     

	"varying vec4 vColor;\n" 

	"void main()\n"
	"{\n"
	"   vec3 modelViewVertex = vec3(uMVMatrix * vec4(aPosition, 0.0));\n"
	"   vec3 modelViewNormal = vec3(uMVMatrix * vec4(aNormal, 0.0));\n"
	"   float distance = length(uLightPos - modelViewVertex);\n"
	"   vec3 lightVector = normalize(uLightPos - modelViewVertex);\n"
	"   float diffuse = max(dot(modelViewNormal, lightVector), 0.1);\n"
	"   diffuse = diffuse * (1.0 / (1.0 + (0.25 * distance * distance)));\n"
	"   vColor = uColor * diffuse;\n"
	"   gl_Position = uMVPMatrix * vec4(aPosition, 1.0);\n"
	"}" 
};

const GLchar* gFragmentShaderSource[] = {
	"#version 440 core\n"
	"varying vec4 vColor;\n"
	"void main() {\n"
	"  gl_FragColor = vColor;\n"
	"}"
};

inline float deg_to_rad(float rad)
{
	return rad * float(M_PI / 180);
}

GLuint CompileShaders(const GLchar** vertexShaderSource, const GLchar** fragmentShaderSource)
{
	//Compile vertex shader
	GLuint vertexShader(glCreateShader(GL_VERTEX_SHADER));
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	int logLength;
	glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		GLchar* strInfoLog = new GLchar[logLength + 1];
		glGetShaderInfoLog(vertexShader, logLength, NULL, strInfoLog);
		printf("vertex shader log: %s\n", strInfoLog);
	};

	//Compile fragment shader
	GLuint fragmentShader(glCreateShader(GL_FRAGMENT_SHADER));
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//Link vertex and fragment shader together
	GLuint program(glCreateProgram());
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	//Delete shaders objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

bool loadStl(const char* fileName) {
	ifstream in(fileName);
	if (!in.good()) return false;
	bool isBinary;
	// Quick check if ascii or binary (won't work on every file!)
	char head[6];
	in.read(head, 5);
	head[5] = '\0';
	isBinary = (strcmp(head, "solid") != 0);
	in.seekg(0, ifstream::beg);
	Vertex3f normal;
	Vertex3f vertices[3];
	if (isBinary) {
		in.close();
		FILE *f;
		fopen_s(&f, fileName, "rb");
		if (!f) return false;
		char title[80];
		int numFaces;
		fread(title, 80, 1, f);
		fread((void*)&numFaces, 4, 1, f);
		unsigned short uint16;
		for (size_t i = 0; i<numFaces; ++i) {
			fread((void*)&normal, sizeof(Vertex3f), 1, f);
			for (int i = 0; i < 3; i++) {
				fread((void*)&vertices[i], sizeof(Vertex3f), 1, f);
			}
			fread((void*)&uint16, sizeof(unsigned short), 1, f); // spacer

			for (int i = 0; i < 3; i++) {
				gVertices.push_back(vertices[i] * .025f);
				gNormals.push_back(normal);
			}
		}
		fclose(f);
	}
	else {
		char title[80];
		string s0, s1;
		in.read(title, 80);
		while (!in.eof()) {
			in >> s0;
			if (s0 == "facet") {
				in >> s1 >> normal.x >> normal.y >> normal.z >> s0 >> s1;
				for (int i = 0; i < 3; i++) {
					in >> s0 >> vertices[i].x >> vertices[i].z >> vertices[i].y; // flip z and y
				}
				in >> s0 >> s0;
				for (auto& vertex : vertices) {
					gVertices.push_back(vertex * .025f);
				}
				for (int i = 0; i < 3; i++) {
					gNormals.push_back(normal);
				}
			}
			else if (s0 == "endsolid") {
				break;
			}
		}
		in.close();
	}
	// Calculate extremes and center
	Vertex3f vmin = gVertices[0];
	Vertex3f vmax = gVertices[0];
	for (auto& v : gVertices) {
		vmin = { min(v.x, vmin.x), min(v.y, vmin.y), min(v.z, vmin.z) };
		vmax = { max(v.x, vmax.x), max(v.y, vmax.y), max(v.z, vmax.z) };
	}
	for (auto& v : gVertices) {
		v -= ((vmin + vmax) / 2.0f);
	}
	return true;
}

void Init(void)
{
	if (!loadStl("purple_tentacle.stl")) {
		exit(EXIT_FAILURE);
	}

	gProgram = CompileShaders(gVertexShaderSource, gFragmentShaderSource);
	glUseProgram(gProgram);

	programHandles.aPosition = glGetAttribLocation(gProgram, "aPosition");
	programHandles.aNormal = glGetAttribLocation(gProgram, "aNormal");

	programHandles.mvpMatrix = glGetUniformLocation(gProgram, "uMVPMatrix");
	programHandles.mvMatrix = glGetUniformLocation(gProgram, "uMVMatrix");
	programHandles.uLightPos = glGetUniformLocation(gProgram, "uLightPos");
	programHandles.uColor = glGetUniformLocation(gProgram, "uColor");

	// vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// vertices
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, gVertices.size() * sizeof(Vertex3f), &gVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(programHandles.aPosition);
	glVertexAttribPointer(programHandles.aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// normals
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, gNormals.size() * sizeof(Vertex3f), &gNormals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(programHandles.aNormal);
	glVertexAttribPointer(programHandles.aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void Display(double timeFactor)
{
	glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 mmMatrix = glm::mat4();
	mmMatrix = glm::translate(mmMatrix, glm::vec3(0.0f, 0.0f, -5.0f));

	// modelview matrix
	mvMatrix = glm::mat4();
	mvMatrix = glm::translate(mvMatrix, glm::vec3(0.0f, 0.0f, -2.0f));
	mvMatrix = glm::rotate(mvMatrix, deg_to_rad(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	mvMatrix = glm::rotate(mvMatrix, deg_to_rad(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	mvMatrix = glm::rotate(mvMatrix, deg_to_rad(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	// modelview projection matrix
	int winWidth, winHeight;
	glfwGetWindowSize(window, &winWidth, &winHeight);
	mvpMatrix = glm::perspective(45.0f, (float)winWidth / (float)winHeight, 0.1f, 100.0f) * mvMatrix;

	lightPos.x = sin(deg_to_rad(360.0 * degTimer));
	lightPos.z = cos(deg_to_rad(360.0 * degTimer));

	glUniformMatrix4fv(programHandles.mvpMatrix, 1, false, &mvpMatrix[0][0]);
	glUniformMatrix4fv(programHandles.mvMatrix, 1, false, &mvMatrix[0][0]);
	glUniform3f(programHandles.uLightPos, lightPos.x, lightPos.y, lightPos.z);
	glUniform4f(programHandles.uColor, 1.0, 1.0, 1.0, 0.0);

	// Render vertex array object
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, gVertices.size());
}

// glfw error callback
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

// glfw mouse cursor callback
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if ((lastxpos == -1) || (lastypos == -1)) {
		lastxpos = xpos;
		lastypos = ypos;
		return;
	}

	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS) {
		rotation.y += (xpos - lastxpos) * 0.25f;
		rotation.x += (ypos - lastypos) * 0.25f;
		rotation.y = (rotation.y > 360.0f) ? 360.0f : rotation.y - 360.0f;
		rotation.x == (rotation.x > 360.0f) ? 360.0f : rotation.x - 360.0f;
	}

	lastxpos = xpos;
	lastypos = ypos;
}

// glfw keyboard callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main(void)
{
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	// requst OpenGL 4.4 core context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create a window and create its OpenGL context
	//	window = glfwCreateWindow(1920, 1200, "OpenGL Compute Shader Particle System", glfwGetPrimaryMonitor(), NULL);
	window = glfwCreateWindow(1280, 720, "OpenGL persistent mapped buffers", NULL, NULL);

	//If the window couldn't be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(window);

	//Set callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}

	Init();

	// Output some info on the OpenGL implementation
	const GLubyte* glvendor = glGetString(GL_VENDOR);
	const GLubyte* glrenderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);

	printf("Vendor: %s\n", glvendor);
	printf("Renderer: %s\n", glrenderer);
	printf("Version: %s\n", glversion);

	glEnable(GL_DEPTH_TEST);

	double lastFPStime = glfwGetTime();
	int frameCounter = 0;

	do
	{

		double frameTimeStart = glfwGetTime();

		double thisFPStime = glfwGetTime();
		frameCounter++;

		if (thisFPStime - lastFPStime >= 1.0)
		{
			lastFPStime = thisFPStime;

			std::string windowTitle = "OpenGL persitent mapped buffers (";
			windowTitle += std::to_string(frameCounter);
			windowTitle += " fps) - (c) 2015 by Sascha Willems (www.saschawillems.de)";
			const char* windowCaption = windowTitle.c_str();
			glfwSetWindowTitle(window, windowCaption);

			frameCounter = 0;
		}


		Display(frameTime);
		glfwSwapBuffers(window);

		glfwPollEvents();

		frameTime = (float)(glfwGetTime() - frameTimeStart);

		degTimer += (float)frameTime * 0.1;
		if (degTimer > 360.0) {
			degTimer -= 360.0;
		}
  
	}
	while (!glfwWindowShouldClose(window));

	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}