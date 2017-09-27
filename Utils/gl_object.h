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


  // requires to be H, W, C
  size_t height, width, channels;
  Dtype *data;
  GLuint texture_id;
  GLuint buffer_id;

  bool loaded;
  GLint interpolation;
  GLenum _type;

  GlObject(size_t h = 0, size_t w = 0, size_t c = 0)
    : height(h), width(w), channels(c),
      data(nullptr),
      texture_id(0), buffer_id(0),
      loaded(false), interpolation(GL_NEAREST) {

    if (std::is_same<Dtype, float>::value)
      _type = GL_FLOAT;
    if (std::is_same<Dtype, unsigned char>::value)
      _type = GL_UNSIGNED_BYTE;
  }

  size_t len() const {
    return width * height * channels;
  }

  size_t size() const {
    return len() * sizeof(Dtype);
  }

  void allocate() {
    data = new Dtype[len()];
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
    if(channels == 3)
      return GL_RGB;
    if(channels == 4)
      return GL_RGBA;
    return GL_RGB;
  }

};

}; // namespace Utils

#endif // GL_OBJECT_H