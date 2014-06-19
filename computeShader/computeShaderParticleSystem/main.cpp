//Include GLEW
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

//Include the standard C++ headers
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string>

#include "glRenderer.h"

glRenderer renderer;

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

	//Request an OpenGL 4.3 core context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Declare a window object
	GLFWwindow* window;

	//Create a window and create its OpenGL context
//	window = glfwCreateWindow(1920, 1200, "OpenGL Compute Shader Particle System", glfwGetPrimaryMonitor(), NULL);
	window = glfwCreateWindow(1280, 720, "OpenGL Compute Shader Particle System", NULL, NULL);

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
	renderer.generateTextures();
	renderer.generateBuffers();
	renderer.generateShaders();

	printf("\nKeys:\n");
	printf("""r"" : reset particles at current cursor pos\n");
	printf("""b"" : toggle viewport border for particle movement\n");
	printf("""c"" : toggle random color fade\n");
	printf("""+"" : increase speed\n");
	printf("""-"" : decrease speed\n");
	printf("""page up"" : increase particle count by 1024\n");
	printf("""page down"" : decrease particle count by 1024\n");


	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

			std::string windowTitle = "OpenGL Compute Shader Particle System (";
			windowTitle += std::to_string(renderer.particleCount);
			windowTitle += " particles @ ";
			windowTitle += std::to_string(frameCounter);
			windowTitle += " fps)";
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