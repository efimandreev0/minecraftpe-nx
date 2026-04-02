#ifndef MINECRAFTCPP_GLCTR_H
#define MINECRAFTCPP_GLCTR_H

#if defined(__3DS__)

#include <GLES/gl.h>

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif


void ctrglInit();
void ctrglGenBuffers(GLsizei n, GLuint* buffers);
void ctrglDeleteBuffers(GLsizei n, const GLuint* buffers);
void ctrglBindBuffer(GLenum target, GLuint buffer);
void ctrglBufferData(GLenum target, GLsizei size, const GLvoid* data, GLenum usage);
void ctrglVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void ctrglTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void ctrglColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void ctrglDrawArrays(GLenum mode, GLint first, GLsizei count);

#endif

#endif //MINECRAFTCPP_GLCTR_H
