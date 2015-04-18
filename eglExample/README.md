# EGL example

<a href="./screenshot.png"><img src="./screenshot.png" width="300px"></a>

## About
This is a basic demo showing how to use Khronos' EGL API for rendering OpenGL ES 2.0 on a windows desktop.

EGL is used to create a context with OpenGL ES as the client API, the rendering is then done using OpenGL ES calls.

## Notes
Using OpenGL ES on desktops differs from IHV to IHV. While NVIDIA and Intel expose OpenGL ES on desktop via WGL/GLX extensions (e.g. http://opengl.delphigl.de/gl_listreports.php?listreportsbyextension=WGL_EXT_create_context_es_profile), AMD exposes it through EGL.

## Requirements
To make this demo work, you need an ICD that supports EGL on windows, which should be AMD only.

The required libs and dlls are available via :
- AMD OpenGL ES SDK : http://developer.amd.com/tools-and-sdks/graphics-development/amd-opengl-es-sdk/
- ANGLE : https://code.google.com/p/angleproject/
- PowerVR SDK : http://community.imgtec.com/developers/powervr/graphics-sdk/

Headers and dlls included in this demo are from the AMD OpenGL ES SDK, so you may have to use different ones depending on your GPU.