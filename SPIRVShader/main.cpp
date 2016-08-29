/*
* OpenGL Example - Using binary SPIR-V shaders (GL_ARB_gl_spirv)
*
* Extension: https://www.opengl.org/registry/specs/ARB/gl_spirv.txt
* Supported GPUs: http://opengl.gpuinfo.org/gl_listreports.php?listreportsbyextension=GL_ARB_gl_spirv
* 
* Copyright (C) 2015 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const std::string appTitle = "OpenGL example - GL_ARB_gl_spirv";

struct 
{
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
} uboVS;

class OpenGLExample
{
public:
	GLuint shader;
	GLuint VBO[2];
	GLuint IBO;
	GLuint UBO;
	uint32_t indices;
	float zoom = -2.0f;
	glm::vec3 rotation = glm::vec3(0.0f);
	GLFWwindow* window;

	OpenGLExample(GLFWwindow *window)
	{
		this->window = window;
	}

	void printShaderLog(GLuint shader)
	{
		GLint result = GL_FALSE;
		int logLength;

		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) 
		{
			GLchar* strInfoLog = new GLchar[logLength + 1];
			glGetShaderInfoLog(shader, logLength, NULL, strInfoLog);
			std::cout << "Shaderlog: " << strInfoLog << std::endl;
		};
	}

	void printProgramLog(GLuint program)
	{
		GLint result = GL_FALSE;
		int logLength;

		glGetProgramiv(program, GL_LINK_STATUS, &result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) 
		{
			GLchar* strInfoLog = new GLchar[logLength + 1];
			glGetProgramInfoLog(program, logLength, NULL, strInfoLog);
			std::cout << "Programlog: " << strInfoLog << std::endl;
		};
	}

	bool loadBinaryShader(const char *fileName, GLuint stage, GLuint binaryFormat, GLuint &shader)
	{
		std::ifstream shaderFile;
		shaderFile.open(fileName, std::ios::binary | std::ios::ate);
		if (shaderFile.is_open())
		{
			size_t size = shaderFile.tellg();
			shaderFile.seekg(0, std::ios::beg);
			char* bin = new char[size];
			shaderFile.read(bin, size);

			GLint status;
			shader = glCreateShader(stage);									// Create a new shader
			glShaderBinary(1, &shader, binaryFormat, bin, size);			// Load the binary shader file
			glSpecializeShaderARB(shader, "main", 0, nullptr, nullptr);		// Set main entry point (required, no specialization used in this example)
			glGetShaderiv(shader, GL_COMPILE_STATUS, &status);				// Check compilation status
			return status;
		}
		else
		{
			std::cerr << "Could not open \"" << fileName << "\"" << std::endl;
			return false;
		}
	}

	GLuint loadShader(const char* vsFileName, const char* fsFileName)
	{
		GLuint vertShader, fragShader;

		GLint result = GL_TRUE;

		result &= loadBinaryShader(vsFileName, GL_VERTEX_SHADER, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, vertShader);
		result &= loadBinaryShader(fsFileName, GL_FRAGMENT_SHADER, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, fragShader);

		if (!result)
		{
			std::cerr << "Could not load all binary shaders required for this program" << std::endl;
			return GL_FALSE;
		}

		std::cout << "Linking shader program" << std::endl;
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

	void loadAssets()
	{
		shader = loadShader("../data/shader/triangle.vert.spv", "../data/shader/triangle.frag.spv");
	}

	void updateUBO()
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

	void generateBuffers()
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

	void draw(double deltaT)
	{
		double frameTimeStart = glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 9);

		glfwSwapBuffers(window);

		rotation.y += deltaT * 50.0f;
		updateUBO();
	}
};

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

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

	GLFWwindow* window = glfwCreateWindow(1280, 720, appTitle.c_str(), NULL, NULL);
	
	if (!window)
	{
		std::cerr << "Failed to open GLFW window" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(window);

	//Set callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK)
	{
		std::cerr << "GLEW Error: " <<  glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	std::cout << "Vendor:" << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer:" << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version:" << glGetString(GL_VERSION) << std::endl;

	std::cout << std::boolalpha << "SPIRV supported: " << (GLEW_ARB_gl_spirv == true) << std::endl;

	if (!GLEW_ARB_gl_spirv)
	{
		std::cerr << "This examples requires support for SPIR-V (GL_ARB_gl_spirv)!" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	glfwSwapInterval(0);

	OpenGLExample example(window);

	example.generateBuffers();
	example.loadAssets();

	glDisable(GL_CULL_FACE);

	double lastFPStime = glfwGetTime();
	double lastFrameTime = glfwGetTime();
	int frameCounter = 0;

	//Main Loop
	do
	{
		double thisFPStime = glfwGetTime();
		frameCounter++;

		if (thisFPStime - lastFPStime >= 1.0)
		{
			lastFPStime = thisFPStime;

			std::string windowTitle = appTitle +" (";
			windowTitle += std::to_string(frameCounter);
			windowTitle += " fps) - 2016 by Sascha Willems (www.saschawillems.de)";
			const char* windowCaption = windowTitle.c_str();
			glfwSetWindowTitle(window, windowCaption);

			frameCounter = 0;
		}

		double currTime = glfwGetTime();
		example.draw(currTime - lastFrameTime);
		lastFrameTime = currTime;

		//Get and organize events, like keyboard and mouse input, window resizing, etc...
		glfwPollEvents();


	} //Check if the ESC key had been pressed or if the window had been closed
	while (!glfwWindowShouldClose(window));

	//Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	//Finalize and clean up GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);
}