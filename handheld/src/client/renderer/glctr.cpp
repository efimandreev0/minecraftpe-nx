#include "glctr.h"

#if defined(__3DS__)

#include <unordered_map>
#include <vector>
#include <cstring>

#ifndef DISPLAY_TRANSFER_FLAGS
#define DISPLAY_TRANSFER_FLAGS (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
#endif


namespace {
struct ArrayPointerState {
    GLint size = 0;
    GLenum type = 0;
    GLsizei stride = 0;
    const GLvoid* pointer = nullptr;
};

struct CtrBuffer {
    std::vector<unsigned char> data;
};

std::unordered_map<GLuint, CtrBuffer> g_buffers;
GLuint g_nextBufferId = 1;
GLuint g_boundArrayBuffer = 0;
ArrayPointerState g_vertexPointer;
ArrayPointerState g_texCoordPointer;
ArrayPointerState g_colorPointer;

C3D_RenderTarget* g_target = nullptr;
float g_clearR = 0.0f;
float g_clearG = 0.0f;
float g_clearB = 0.0f;
float g_clearA = 1.0f;
float g_clearDepth = 1.0f;

const GLvoid* resolvePointer(const ArrayPointerState& state) {
    if (g_boundArrayBuffer == 0 || state.pointer == nullptr) {
        return state.pointer;
    }

    auto it = g_buffers.find(g_boundArrayBuffer);
    if (it == g_buffers.end() || it->second.data.empty()) {
        return nullptr;
    }

    const size_t offset = reinterpret_cast<size_t>(state.pointer);
    if (offset >= it->second.data.size()) {
        return nullptr;
    }

    return static_cast<const GLvoid*>(it->second.data.data() + offset);
}

u32 packColor(float r, float g, float b, float a) {
    const int ri = (int)(r * 255.0f);
    const int gi = (int)(g * 255.0f);
    const int bi = (int)(b * 255.0f);
    const int ai = (int)(a * 255.0f);
    return C3D_Color32((u8)ri, (u8)gi, (u8)bi, (u8)ai);
}
}

void ctrglPlatformInit() {
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    g_target = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (g_target) {
        C3D_RenderTargetSetOutput(
            g_target,
            GFX_TOP,
            GFX_LEFT,
            DISPLAY_TRANSFER_FLAGS
        );
    }
}

void ctrglPlatformExit() {
    if (g_target) {
        C3D_RenderTargetDelete(g_target);
        g_target = nullptr;
    }
    C3D_Fini();
}

void ctrglBeginFrame() {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    if (g_target) {
        C3D_FrameDrawOn(g_target);
    }
}

void ctrglEndFrame() {
    C3D_FrameEnd(0);
}

void ctrglInit() {
    g_buffers.clear();
    g_nextBufferId = 1;
    g_boundArrayBuffer = 0;
    g_vertexPointer.size = 0;
    g_vertexPointer.type = 0;
    g_vertexPointer.stride = 0;
    g_vertexPointer.pointer = nullptr;
    g_texCoordPointer.size = 0;
    g_texCoordPointer.type = 0;
    g_texCoordPointer.stride = 0;
    g_texCoordPointer.pointer = nullptr;
    g_colorPointer.size = 0;
    g_colorPointer.type = 0;
    g_colorPointer.stride = 0;
    g_colorPointer.pointer = nullptr;
}

void ctrglGenBuffers(GLsizei n, GLuint* buffers) {
    for (GLsizei i = 0; i < n; ++i) {
        buffers[i] = g_nextBufferId++;
        g_buffers.emplace(buffers[i], CtrBuffer{});
    }
}

void ctrglDeleteBuffers(GLsizei n, const GLuint* buffers) {
    for (GLsizei i = 0; i < n; ++i) {
        if (g_boundArrayBuffer == buffers[i]) {
            g_boundArrayBuffer = 0;
        }
        g_buffers.erase(buffers[i]);
    }
}

void ctrglBindBuffer(GLenum target, GLuint buffer) {
    if (target != GL_ARRAY_BUFFER) return;
    g_boundArrayBuffer = buffer;
    if (buffer != 0 && g_buffers.find(buffer) == g_buffers.end()) {
        g_buffers.emplace(buffer, CtrBuffer{});
    }
}

void ctrglBufferData(GLenum target, GLsizei size, const GLvoid* data, GLenum usage) {
    (void)usage;
    if (target != GL_ARRAY_BUFFER || g_boundArrayBuffer == 0 || size <= 0 || data == nullptr) return;
    CtrBuffer& buffer = g_buffers[g_boundArrayBuffer];
    buffer.data.resize((size_t)size);
    std::memcpy(buffer.data.data(), data, (size_t)size);
}

void ctrglVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
    g_vertexPointer.size = size;
    g_vertexPointer.type = type;
    g_vertexPointer.stride = stride;
    g_vertexPointer.pointer = pointer;
}

void ctrglTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
    g_texCoordPointer.size = size;
    g_texCoordPointer.type = type;
    g_texCoordPointer.stride = stride;
    g_texCoordPointer.pointer = pointer;
}

void ctrglColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
    g_colorPointer.size = size;
    g_colorPointer.type = type;
    g_colorPointer.stride = stride;
    g_colorPointer.pointer = pointer;
}

void ctrglDrawArrays(GLenum mode, GLint first, GLsizei count) {
    (void)mode;
    (void)first;
    (void)count;
    (void)resolvePointer(g_vertexPointer);
    (void)resolvePointer(g_texCoordPointer);
    (void)resolvePointer(g_colorPointer);
}

extern "C" {
GLenum glGetError(void) { return GL_NO_ERROR; }
void glViewport(GLint, GLint, GLsizei, GLsizei) {}
void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a) { g_clearR = r; g_clearG = g; g_clearB = b; g_clearA = a; }
void glClear(GLbitfield mask) {
    u32 bits = 0;
    if (mask & GL_COLOR_BUFFER_BIT) bits |= C3D_CLEAR_COLOR;
    if (mask & GL_DEPTH_BUFFER_BIT) bits |= C3D_CLEAR_DEPTH;
    C3D_ClearBits(bits, packColor(g_clearR, g_clearG, g_clearB, g_clearA), g_clearDepth, 0);
}
void glClearDepthf(GLclampf depth) { g_clearDepth = depth; }
void glClearDepth(GLdouble depth) { g_clearDepth = (float)depth; }
void glEnable(GLenum) {}
void glDisable(GLenum) {}
void glBlendFunc(GLenum, GLenum) {}
void glDepthFunc(GLenum) {}
void glDepthMask(GLboolean) {}
void glColorMask(GLboolean, GLboolean, GLboolean, GLboolean) {}
void glScissor(GLint, GLint, GLsizei, GLsizei) {}
void glShadeModel(GLenum) {}
void glHint(GLenum, GLenum) {}
void glLineWidth(GLfloat) {}

void glMatrixMode(GLenum) {}
void glLoadIdentity(void) {}
void glPushMatrix(void) {}
void glPopMatrix(void) {}
void glTranslatef(GLfloat, GLfloat, GLfloat) {}
void glRotatef(GLfloat, GLfloat, GLfloat, GLfloat) {}
void glScalef(GLfloat, GLfloat, GLfloat) {}
void glMultMatrixf(const GLfloat*) {}
void glOrthof(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat) {}
void glOrtho(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble) {}

void glEnableClientState(GLenum) {}
void glDisableClientState(GLenum) {}
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) { ctrglVertexPointer(size, type, stride, pointer); }
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) { ctrglTexCoordPointer(size, type, stride, pointer); }
void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) { ctrglColorPointer(size, type, stride, pointer); }
void glNormalPointer(GLenum, GLsizei, const GLvoid*) {}
void glDrawArrays(GLenum mode, GLint first, GLsizei count) { ctrglDrawArrays(mode, first, count); }

void glGenTextures(GLsizei n, GLuint* textures) { for (GLsizei i = 0; i < n; ++i) textures[i] = (GLuint)(i + 1); }
void glBindTexture(GLenum, GLuint) {}
void glDeleteTextures(GLsizei, const GLuint*) {}
void glTexParameteri(GLenum, GLenum, GLint) {}
void glTexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*) {}
void glTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*) {}

void glGenBuffers(GLsizei n, GLuint* buffers) { ctrglGenBuffers(n, buffers); }
void glBindBuffer(GLenum target, GLuint buffer) { ctrglBindBuffer(target, buffer); }
void glBufferData(GLenum target, GLsizei size, const GLvoid* data, GLenum usage) { ctrglBufferData(target, size, data, usage); }
void glDeleteBuffers(GLsizei n, const GLuint* buffers) { ctrglDeleteBuffers(n, buffers); }

void glColor4f(GLfloat, GLfloat, GLfloat, GLfloat) {}
void glColor3f(GLfloat, GLfloat, GLfloat) {}
void glAlphaFunc(GLenum, GLclampf) {}
void glPolygonOffset(GLfloat, GLfloat) {}
void glFlush(void) {}

void glTranslate(GLfloat, GLfloat, GLfloat) {}
void glDepthRangef(GLclampf, GLclampf) {}
void glDepthRange(GLdouble, GLdouble) {}
void glCullFace(GLenum) {}
void glPolygonMode(GLenum, GLenum) {}
void glReadPixels(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*) {}
void glCompressedTexImage2D(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*) {}
void glFogf(GLenum, GLfloat) {}
void glFogfv(GLenum, const GLfloat*) {}
void glFogi(GLenum, GLint) {}
void glFogx(GLenum, GLint) {}
void glColorMaterial(GLenum, GLenum) {}
void glNormal3f(GLfloat, GLfloat, GLfloat) {}
GLuint glGenLists(GLsizei) { return 1; }
void glNewList(GLuint, GLenum) {}
void glEndList(void) {}
void glCallList(GLuint) {}
void glCallLists(GLsizei, GLenum, const GLvoid*) {}
void glDeleteLists(GLuint, GLsizei) {}
void glClearPolyID(GLuint) {}
void glGetFloatv(GLenum, GLfloat*) {}
void glGetFloat(GLenum, GLfloat*) {}
}

#endif
