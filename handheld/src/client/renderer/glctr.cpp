#include "glctr.h"

#if defined(__3DS__)

#include <unordered_map>
#include <vector>
#include <cstring>

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
}

void ctrglInit() {
    g_buffers.clear();
    g_nextBufferId = 1;
    g_boundArrayBuffer = 0;
    g_vertexPointer = {};
    g_texCoordPointer = {};
    g_colorPointer = {};
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
    if (target != GL_ARRAY_BUFFER) {
        return;
    }

    g_boundArrayBuffer = buffer;
    if (buffer != 0 && g_buffers.find(buffer) == g_buffers.end()) {
        g_buffers.emplace(buffer, CtrBuffer{});
    }
}

void ctrglBufferData(GLenum target, GLsizei size, const GLvoid* data, GLenum usage) {
    (void)usage;
    if (target != GL_ARRAY_BUFFER || g_boundArrayBuffer == 0 || size <= 0 || data == nullptr) {
        return;
    }

    CtrBuffer& buffer = g_buffers[g_boundArrayBuffer];
    buffer.data.resize(static_cast<size_t>(size));
    std::memcpy(buffer.data.data(), data, static_cast<size_t>(size));
}

void ctrglVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
    g_vertexPointer = {size, type, stride, pointer};
    glVertexPointer(size, type, stride, resolvePointer(g_vertexPointer));
}

void ctrglTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
    g_texCoordPointer = {size, type, stride, pointer};
    glTexCoordPointer(size, type, stride, resolvePointer(g_texCoordPointer));
}

void ctrglColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
    g_colorPointer = {size, type, stride, pointer};
    glColorPointer(size, type, stride, resolvePointer(g_colorPointer));
}

void ctrglDrawArrays(GLenum mode, GLint first, GLsizei count) {
    glVertexPointer(g_vertexPointer.size, g_vertexPointer.type, g_vertexPointer.stride, resolvePointer(g_vertexPointer));
    if (g_texCoordPointer.pointer != nullptr) {
        glTexCoordPointer(g_texCoordPointer.size, g_texCoordPointer.type, g_texCoordPointer.stride, resolvePointer(g_texCoordPointer));
    }
    if (g_colorPointer.pointer != nullptr) {
        glColorPointer(g_colorPointer.size, g_colorPointer.type, g_colorPointer.stride, resolvePointer(g_colorPointer));
    }
    glDrawArrays(mode, first, count);
}

#endif
