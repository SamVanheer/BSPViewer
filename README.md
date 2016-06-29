This project is a prototype map renderer for Half-Life BSP files. It uses OpenGL 3.1 Shaders (also known as Retained Mode) to render the map, as opposed to the engine's method, which uses Immediate Mode.

Currently, partial support for the following render modes is available:
Normal
Texture
Additive
Solid

Support for these shaders is incomplete and requires more work, but it is currently possible to load and render an entire map.
Water textures use a separate shader that enables the sinusoidal wave effects used in Quake and Half-Life.

The project uses the following third party libraries:  
SDL: Version 2.0.4.0  
GLEW: Version 1.13.0.0  
GLM: Version 0.9.6.3  

All third party libraries are referenced as though they were placed in the external/ directory.
Each library is stored in the same manner:  
LIBNAME/  
+-include  
+-lib  
