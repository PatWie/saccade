#include <glog/logging.h>

#include "gl_manager.h"
#include "../GUI/marker.h"
#include "../GUI/slides.h"
#include "../GUI/canvas.h"

Utils::GlManager::GlManager(QOpenGLContext* context) {
  ctx = context;
}
Utils::GlManager::~GlManager() {}

void Utils::GlManager::set_size(int width, int height) {
  glViewport( 0, 0, width, height );
}

void Utils::GlManager::set_ortho(int width, int height) {
  glOrtho( -0.5 * ((double)width),
           0.5 * ((double)width),
           -0.5 * (double)height,
           0.5 * (double)height,
           -2.0, 2.0);
}

void Utils::GlManager::clear() {
  glClear( GL_COLOR_BUFFER_BIT );
}

void Utils::GlManager::identity() {
  glLoadIdentity();
}

void Utils::GlManager::projection_identity() {
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
}

void Utils::GlManager::modelview_identity() {
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

void Utils::GlManager::enable_texture_blend() {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
}

void Utils::GlManager::printContextInformation() {
  QString glType;
  QString glVersion;
  QString glProfile;

  // Get Version Information
  glType = (ctx->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
  glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

  // Get Profile Information
#define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
  switch (ctx->format().profile()) {
    CASE(NoProfile);
    CASE(CoreProfile);
    CASE(CompatibilityProfile);
  }
#undef CASE

  QString info;
  info = glType + glVersion + "(" + glProfile + ")";
  LOG(INFO) << info.toStdString();
}


void Utils::GlManager::drawMarker(GUI::Canvas* canvas, GUI::Marker* marker) {

  if (!marker->active)
    return;

  GUI::Canvas::property_t prop = canvas->getProperty();
  const double zoom = prop.zoom_factor;

  glLoadIdentity();
  glScaled(prop.zoom_factor, -prop.zoom_factor, 1.0);
  glTranslated(prop.x, -prop.y, 0.0);
  glRotatef(prop.angle, 0.0, 0.0, 1.0);
  glTranslated(-0.5 * (double)canvas->slides()->width(),
               -0.5 * (double)canvas->slides()->height(), 0.0);


  int x = marker->x;
  int y = marker->y;
  glBindTexture( GL_TEXTURE_2D, 0 );
  glBegin(GL_TRIANGLES);
  double x2, y2;

  x2 = x + 1.0;
  y2 = y + 0.5;
  glColor3d( 1.0, 1.0, 1.0 );
  glVertex3f(x2, y2, -1.0);
  glColor3d( 0.0, 0.0, 0.0 );
  glVertex3f(x2 + ((double)10 / zoom), y2 - ((double)5 / zoom), -1.0);
  glVertex3f(x2 + ((double)10 / zoom), y2 + ((double)5 / zoom), -1.0);

  x2 = x + 0.5;
  y2 = y + 1.0;
  glColor3d( 1.0, 1.0, 1.0 );
  glVertex3f(x2, y2, -1.0);
  glColor3d( 0.0, 0.0, 0.0 );
  glVertex3f(x2 + ((double)5 / zoom), y2 + ((double)10 / zoom), -1.0);
  glVertex3f(x2 - ((double)5 / zoom), y2 + ((double)10 / zoom), -1.0);

  x2 = x;
  y2 = y + 0.5;
  glColor3d( 1.0, 1.0, 1.0 );
  glVertex3f(x2, y2, -1.0);
  glColor3d( 0.0, 0.0, 0.0 );
  glVertex3f(x2 - ((double)10 / zoom), y2 + ((double)5 / zoom), -1.0);
  glVertex3f(x2 - ((double)10 / zoom), y2 - ((double)5 / zoom), -1.0);

  x2 = x + 0.5;
  y2 = y;
  glColor3d( 1.0, 1.0, 1.0 );
  glVertex3f(x2, y2, -1.0);
  glColor3d( 0.0, 0.0, 0.0 );
  glVertex3f(x2 + ((double)5 / zoom), y2 - ((double)10 / zoom), -1.0);
  glVertex3f(x2 - ((double)5 / zoom), y2 - ((double)10 / zoom), -1.0);

  glEnd();
}
