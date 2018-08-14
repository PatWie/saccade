#ifndef GL_MANAGER_H
#define GL_MANAGER_H

#include <QOpenGLFunctions>
#include <iostream>

#include "gl_object.h"

namespace GUI {
class Canvas;
class Marker;
}; // namespace GUI

namespace Utils {

/**
 * @brief OpenGL and Qt seems to only work if there is a
 *        dedicated class with an OpenGL context
 *
 * @summary This class is responsible to create a single OpenGL context, which is shared
 * across all windows
 */
class GlManager : public QOpenGLFunctions {
  QOpenGLContext* ctx;

 public:
  GlManager(QOpenGLContext* context);
  ~GlManager();

  void printContextInformation();

  void set_size(int width, int height);
  void set_ortho(int width, int height);

  void clear();
  void identity();
  void projection_identity();
  void modelview_identity();
  void enable_texture_blend();

  /**
   * @brief prepare data for OpenGL
   * @details [long description]
   *
   * @param obj containing raw buffer and shape
   * @param type float / unsigned char
   * @param interpolation nearestneighbor / linear
   * @param generate_buffer prevent multiple buffers for same texture
   * @tparam Dtype [description]
   */
  template<typename Dtype>
  void prepare(GlObject<Dtype> *obj) {

    obj->texture_id = 0;

    if (!obj->loaded) {
      glGenBuffers(1, &obj->buffer_id);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, obj->buffer_id);
      obj->loaded = true;
    }

    glGenTextures(1, &obj->texture_id);
    glBindTexture(GL_TEXTURE_2D, obj->texture_id);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST /*obj->min_interpolation*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST /*obj->max_interpolation*/);

    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB,
                 obj->size(),
                 obj->data, GL_DYNAMIC_DRAW);
    glTexImage2D(GL_TEXTURE_2D, 0, obj->internalformat(),
                 obj->width, obj->height, 0,
                 obj->internalformat(), obj->type(), NULL);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
  }

  template<typename Dtype>
  void draw(GlObject<Dtype> *obj,
            double top, double left,
            double bottom, double right,
            double depth = 1) {

    glBindTexture (GL_TEXTURE_2D, obj->texture_id);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(left, top, depth);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(right, top, depth);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(right, bottom, depth);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(left, bottom, depth);
    glEnd();

    glBindTexture (GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
  }

  /**
   * @brief draw pixel location marker
   * @details [long description]
   *
   * @param canvas where to draw
   * @param marker marker object with coordinates
   */
  void drawMarker(GUI::Canvas* canvas, GUI::Marker* marker);
  /**
   * @brief draq marquee selection rectangle
   * @details [long description]
   *
   * @param canvas [description]
   * @param rect [description]
   * @param r color value red
   * @param g color value green
   * @param b color value blue
   */
  void drawSelection(GUI::Canvas* canvas, QRect rect,
                     float r = 1.0f, float g = 1.0f, float b = 1.0f);
  /**
   * @brief hide everything outside given rectangle by "white fog"
   * @details [long description]
   *
   * @param canvas [description]
   * @param rect [description]
   */
  void drawHighlight(GUI::Canvas* canvas, QRect rect);

};

}; // namespace Utils

#endif // GL_MANAGER_H
