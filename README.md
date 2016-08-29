# C++ OpenGL examples and demos

This repository contains sources for assorted OpenGL samples and demos written in C++.

This is a loose collection of OpenGL testing stuff, so no CMake, no cross platform and not much source documentation.

Use at your own risk ;)

## Requirements
- [GLEW](https://github.com/nigels-com/glew)
- [GFLW](https://github.com/glfw/glfw)
- [ASSIMP](https://github.com/assimp/assimp) (some examples)

## Samples

### [SPIR-V shaders](./SPIRVShader)
Shows how to load binary [SPIR-V](https://www.khronos.org/registry/spir-v/specs/1.0/SPIRV.pdf) Shaders with OpenGL. SPIR-V has initially been introduced as the binary shader format for Vulkan, but is also available on OpenGL via the [GL_ARB_gl_spirv](https://www.opengl.org/registry/specs/ARB/gl_spirv.txt) extension.

This example loads two basic binary SPIR-V shaders instead of GLSL shaders from text. SPIR-V shaders for OpenGL can be generated using the [glslang reference compiler](https://github.com/KhronosGroup/glslang). If you're using Visual Studio, you can use my [SPIRV-VSExtension](https://github.com/SaschaWillems/SPIRV-VSExtension) to generate SPIR-V from GLSL directly out of the IDE.

### [Compute shader particle system](./computeShader)
Attraction based compute shader particle system using SSBOs.

### [Instancing](./instancing)
Mesh instancing (OpenGL 3.3+) demo. Made to compare against the same Vulkan example.

### [Ray picking](./raypicking)
CPU based ray picking for object selection.

### [Geometry shader](./geometryShader)
Very simple geometry shader demo.

### [EGL](./eglExample)
Shows how to use EGL for setting up OpenGL ES 2.0 on a windows desktop. Only works for vendors that support EGL on desktop.

### Simple STL viewer
A very basic demo for loading and displaying a .stl file.
