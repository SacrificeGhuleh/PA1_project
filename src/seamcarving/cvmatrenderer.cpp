//
// Created by zvone on 02-Feb-20.
//

#include "cvmatrenderer.h"


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
  
  #include <GL/gl3w.h>    // Initialize with gl3wInit()

#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
  #include <glad/glad.h>  // Initialize with gladLoadGL()
#else

  #include IMGUI_IMPL_OPENGL_LOADER_CUSTOM

#endif

// Include glfw3.h after our OpenGL definitions

#include <GLFW/glfw3.h>


CvMatRenderer::CvMatRenderer(unsigned int rows, unsigned int cols, const char *imgName) :
    name(imgName),
    rows(rows),
    cols(cols) {
  assign();
}

CvMatRenderer::CvMatRenderer(unsigned int rows, unsigned int cols, const uint8_t *data, const char *imgName) :
    name(imgName),
    rows(rows),
    cols(cols),
    data_(data) {
  assign();
}


void CvMatRenderer::render(float scale) {
  if (ImGui::Begin(name)) {
    assert(scale > 0.f);
    //refresh and render only if not collapsed
    refresh();
    ImGui::Text("pointer = %p", getGpuTexturePtrId());
    ImGui::Text("size = %d x %d", this->width, this->height);
    ImGui::Image((void *) (intptr_t) getGpuTexturePtrId(), ImVec2(this->width * scale, this->height * scale));
  }
  ImGui::End();
}

unsigned int CvMatRenderer::getGpuTexturePtrId() const {
  return gpuTexturePtrId;
}

void CvMatRenderer::refresh() {
  glBindTexture(GL_TEXTURE_2D, gpuTexturePtrId);
  
  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // Upload pixels into texture

//  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  
  //if (T_CHANNELS == 3) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_BGR, GL_UNSIGNED_BYTE, this->data_);
  /*} else if (T_CHANNELS == 4) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->data_);
  } else {
    throw "Invalid number of channels";
  }*/
}

void CvMatRenderer::assign() {
  // Create a OpenGL texture identifier
  glGenTextures(1, &gpuTexturePtrId);
  glBindTexture(GL_TEXTURE_2D, gpuTexturePtrId);
  
  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // Upload pixels into texture


//  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  
  
  //if (T_CHANNELS == 3) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_BGR, GL_BYTE, this->data_);
  /*} else if (T_CHANNELS == 4) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->data_);
  } else {
    throw "Invalid number of channels";
  }*/
}

void CvMatRenderer::refresh(unsigned int rows, unsigned int cols, const uint8_t *data) {
  this->data_ = data;
  this->rows = rows;
  this->cols = cols;
  refresh();
}
