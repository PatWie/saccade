#ifndef GL_OBJECT_H
#define GL_OBJECT_H

#include "misc.h"
#include <iostream>
#include <type_traits>
#include <QDebug>
#include <QOpenGLFunctions>

namespace Utils {
template<typename Dtype>
class GlObject  : protected QOpenGLFunctions {
 public:

  // dimension of image data
  size_t height, width, channels;
  // buffer data
  Dtype *data;

  // OpenGL information
  GLuint texture_id;
  GLuint buffer_id;
  GLint min_interpolation;
  GLint max_interpolation;
  GLenum _type;

  // flags
  bool loaded;

  GlObject(size_t h = 0, size_t w = 0, size_t c = 0)
    : height(h), width(w), channels(c),
      data(nullptr),
      texture_id(0), buffer_id(0),
      loaded(false), min_interpolation(GL_LINEAR), max_interpolation(GL_NEAREST) {

    // saccade currently only supports float and byte data in OpenGL
    if (std::is_same<Dtype, float>::value)
      _type = GL_FLOAT;
    if (std::is_same<Dtype, unsigned char>::value)
      _type = GL_UNSIGNED_BYTE;
  }

  size_t elements() const {
    return width * height * channels;
  }

  size_t size() const {
    return elements() * sizeof(Dtype);
  }

  void allocate() {
    data = new Dtype[elements()];
  }

  GLenum type() const {
    return _type;
  }

  GLint format() const {
    return internalformat();
  }

  GLint internalformat() const {
    if(channels == 1)
      return GL_LUMINANCE;
    else if(channels == 3)
      return GL_RGB;
    else if(channels == 4)
      return GL_RGBA;
    else
      return GL_RGB;
  }

};

}; // namespace Utils

#endif // GL_OBJECT_H
