#ifndef GLUTIL_H
#define GLUTIL_H

void _check_gl_error( const char *file, int line, bool fReport = true );

#define check_gl_error() _check_gl_error(__FILE__,__LINE__)

#define ignore_gl_errors() _check_gl_error(__FILE__,__LINE__, false)

#endif //GLUTIL_H
