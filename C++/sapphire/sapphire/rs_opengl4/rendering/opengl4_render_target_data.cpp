#include "opengl4_render_target_data.h"

#include <rs_opengl4/rendering/opengl4_texture.h>

#include <glad/glad.h>

OpenGL4RenderTargetData::~OpenGL4RenderTargetData() {
    if (framebuffer_handle != -1) {
        glDeleteFramebuffers(1, &framebuffer_handle);
    }

    if (depth_texture_handle != -1) {
        glDeleteTextures(1, &depth_texture_handle);
    }

    if (texture_handle != -1) {
        glDeleteTextures(1, &texture_handle);
    }
}

void OpenGL4RenderTargetData::resize(int width, int height, RenderTarget *p_target) {
    glGenFramebuffers(1, &framebuffer_handle);

    /*
    glGenRenderbuffers(1, &depthbuffer_handle);
    glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer_handle);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    */

    // TODO: Use supported depth formats!

    glGenTextures(1, &depth_texture_handle);
    glBindTexture(GL_TEXTURE_2D, depth_texture_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // TODO: RenderTarget sampling?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    delete color_texture;
    delete depth_texture;

    color_texture = new OpenGL4Texture(texture_handle, false);
    depth_texture = new OpenGL4Texture(depth_texture_handle, false);
}

Texture *OpenGL4RenderTargetData::get_color_texture() {
    return color_texture;
}

Texture *OpenGL4RenderTargetData::get_depth_texture() {
    return depth_texture;
}
