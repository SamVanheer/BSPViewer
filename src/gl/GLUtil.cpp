#include <GL/glew.h>

#include <iostream>

#include "GLUtil.h"

void _check_gl_error( const char *file, int line, bool fReport )
{
    GLenum err = glGetError();
 
    while( err != GL_NO_ERROR )
	{
        std::string error;
 
        switch( err )
		{
            case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
            case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }
 
		if( fReport )
			std::cerr << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;

        err = glGetError();
    }
}
