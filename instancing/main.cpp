/*
* OpenGL instanced mesh rendering
*
* Copyright (C) 2015 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string>

#include "glRenderer.h"
#include "meshLoader.hpp"

glRenderer renderer;

const std::string appTitle = "OpenGL example - Instanced mesh rendering";

//Define an error callback
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	renderer.keyCallback(key, scancode, action, mods);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main(void)
{
	//Set the error callback
	glfwSetErrorCallback(error_callback);

	//Initialize GLFW
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Declare a window object
	GLFWwindow* window;

	//Create a window and create its OpenGL context
	window = glfwCreateWindow(1280, 720, appTitle.c_str(), NULL, NULL);

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

	// Output some info on the OpenGL implementation
	const GLubyte* glvendor = glGetString(GL_VENDOR); 
	const GLubyte* glrenderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	
	printf("Vendor: %s\n", glvendor);
	printf("Renderer: %s\n", glrenderer);
	printf("Version: %s\n", glversion);

	//Set a background color

	renderer = glRenderer();
	renderer.window = window;
	renderer.generateBuffers();
	renderer.generateShaders();

	glDisable(GL_CULL_FACE);

	printf("\nKeys:\n");
	printf("""g"" : Toggle geometry shader\n");
	printf("""w"" : Toggle wireframe\n");
	printf("""+"" : increase number of subdivisions\n");
	printf("""-"" : decrease number of subdivisions\n");
	printf("""shift +"" : increase radius\n");
	printf("""shift -"" : decrease radius\n");

	double lastFPStime = glfwGetTime();
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
			windowTitle += " fps) - 2015 by Sascha Willems (www.saschawillems.de)";
			const char* windowCaption = windowTitle.c_str();
			glfwSetWindowTitle(window, windowCaption);

			frameCounter = 0;
		}

		renderer.renderScene();

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