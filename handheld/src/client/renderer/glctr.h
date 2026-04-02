#ifndef MINECRAFTCPP_GLCTR_H
#define MINECRAFTCPP_GLCTR_H

#if defined(__3DS__)

#include <3ds.h>
#include <citro3d.h>
#include <cstdint>

using GLenum = unsigned int;
using GLboolean = unsigned char;
using GLbitfield = unsigned int;
using GLvoid = void;
using GLbyte = signed char;
using GLshort = short;
using GLint = int;
using GLsizei = int;
using GLubyte = unsigned char;
using GLuint = unsigned int;
using GLfloat = float;
using GLclampf = float;
using GLdouble = double;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_NO_ERROR 0

#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_LINES 0x0001

#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100

#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE 0x1702

#define GL_VERTEX_ARRAY 0x8074
#define GL_NORMAL_ARRAY 0x8075
#define GL_COLOR_ARRAY 0x8076
#define GL_TEXTURE_COORD_ARRAY 0x8078

#define GL_TEXTURE_2D 0x0DE1
#define GL_BLEND 0x0BE2
#define GL_ALPHA_TEST 0x0BC0
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
#define GL_SCISSOR_TEST 0x0C11

#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4

#define GL_UNSIGNED_BYTE 0x1401
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033

#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_ALPHA 0x1906

#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_REPEAT 0x2901

#define GL_LEQUAL 0x0203
#define GL_EQUAL 0x0202

void ctrglPlatformInit();
void ctrglPlatformExit();
void ctrglBeginFrame();
void ctrglEndFrame();

void ctrglInit();
void ctrglGenBuffers(GLsizei n, GLuint* buffers);
void ctrglDeleteBuffers(GLsizei n, const GLuint* buffers);
void ctrglBindBuffer(GLenum target, GLuint buffer);
void ctrglBufferData(GLenum target, GLsizei size, const GLvoid* data, GLenum usage);
void ctrglVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void ctrglTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void ctrglColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void ctrglDrawArrays(GLenum mode, GLint first, GLsizei count);

extern "C" {
GLenum glGetError(void);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
void glClear(GLbitfield mask);
void glClearDepthf(GLclampf depth);
void glClearDepth(GLdouble depth);
void glEnable(GLenum cap);
void glDisable(GLenum cap);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glDepthFunc(GLenum func);
void glDepthMask(GLboolean flag);
void glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a);
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void glShadeModel(GLenum mode);
void glHint(GLenum target, GLenum mode);
void glLineWidth(GLfloat width);

void glMatrixMode(GLenum mode);
void glLoadIdentity(void);
void glPushMatrix(void);
void glPopMatrix(void);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glScalef(GLfloat x, GLfloat y, GLfloat z);
void glMultMatrixf(const GLfloat* m);
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);

void glEnableClientState(GLenum array);
void glDisableClientState(GLenum array);
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);

void glGenTextures(GLsizei n, GLuint* textures);
void glBindTexture(GLenum target, GLuint texture);
void glDeleteTextures(GLsizei n, const GLuint* textures);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);

void glGenBuffers(GLsizei n, GLuint* buffers);
void glBindBuffer(GLenum target, GLuint buffer);
void glBufferData(GLenum target, GLsizei size, const GLvoid* data, GLenum usage);
void glDeleteBuffers(GLsizei n, const GLuint* buffers);

void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void glColor3f(GLfloat r, GLfloat g, GLfloat b);
void glAlphaFunc(GLenum func, GLclampf ref);
void glPolygonOffset(GLfloat factor, GLfloat units);
void glFlush(void);

void glTranslate(GLfloat x, GLfloat y, GLfloat z);
void glDepthRangef(GLclampf n, GLclampf f);
void glDepthRange(GLdouble n, GLdouble f);
void glCullFace(GLenum mode);
void glPolygonMode(GLenum face, GLenum mode);
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
void glFogf(GLenum pname, GLfloat param);
void glFogfv(GLenum pname, const GLfloat* params);
void glFogi(GLenum pname, GLint param);
void glFogx(GLenum pname, GLint param);
void glColorMaterial(GLenum face, GLenum mode);
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
GLuint glGenLists(GLsizei range);
void glNewList(GLuint list, GLenum mode);
void glEndList(void);
void glCallList(GLuint list);
void glCallLists(GLsizei n, GLenum type, const GLvoid* lists);
void glDeleteLists(GLuint list, GLsizei range);
void glClearPolyID(GLuint polyId);
void glGetFloatv(GLenum pname, GLfloat* params);
void glGetFloat(GLenum pname, GLfloat* params);
}

#endif

#endif
