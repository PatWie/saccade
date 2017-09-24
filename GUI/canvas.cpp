#include <QDebug>
#include <iostream>
#include <QMouseEvent>

#include "canvas.h"
#include "layer.h"
#include "slides.h"
#include "marker.h"
#include "../Utils/gl_manager.h"

bool GUI::Canvas::m_glBlock = false;

// http://blog.qt.io/blog/2014/09/10/qt-weekly-19-qopenglwidget/
GUI::Canvas::Canvas(QWidget *parent, ImageWindow* parentWin)
  : QOpenGLWidget(parent), _parent(parent), _parentWin(parentWin) {

  // specify format
  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  setFormat(format);

  // generate context
  _ctx.setFormat(format);
  _ctx.create();
  makeCurrent();

  // handle everthing in custom class using the current context
  _gl = new Utils::GlManager(&_ctx);

  // we allow for dragging, clicking
  setMouseTracking(true);

  // canvas properties
  _property.zoom_factor = 1;
  _property.x = 0;
  _property.y = 0;

  // no dragging without clicking
  _dragging.active = false;
  _dragging.start.setX(0.0);
  _dragging.start.setY(0.0);

  qDebug() << "Canvas ctor";
  // _slides = new Layer();
  _slides = new Slides();
  qDebug() << "Canvas ctor2";

  emit sigUpdateScrollBars(this);

  _marker = new Marker();

}

const GUI::Layer* GUI::Canvas::layer(int i) const {
  if (!_slides->available())
    return nullptr;
  if (i == -1)
    return _slides->current();
  else
    return _slides->operator[](i);
}

const GUI::Slides* GUI::Canvas::slides() const {
  return _slides;
}

void GUI::Canvas::addLayer(Layer *layer) {
  connect( layer, SIGNAL( sigRefresh() ),
           this, SLOT( slotUpdateCanvas() ));
  _slides->add(layer);
  slotUpdateCanvas();
}

void GUI::Canvas::slotUpdateCanvas() {
  qDebug() << "GUI::Canvas::slotUpdateCanvas";
  emit sigUpdateTitle(this);
  emit sigUpdateScrollBars(this);
  update();
}


void GUI::Canvas::slotSetZoomAction(double zoom) {
  _property.zoom_factor = zoom;
  slotUpdateCanvas();
  askSynchronization();
}
void GUI::Canvas::slotZoomInAction() {
  zoom(_focus, 1);
}

void GUI::Canvas::slotZoomOutAction() {
  zoom(_focus, -1);
}

QSize GUI::Canvas::sizeHint() const {
  return QSize(1024, 1024);
}

void GUI::Canvas::askSynchronization() {
  emit sigPropertyChanged(this);
}

void GUI::Canvas::mousePressEvent(QMouseEvent* event) {

  QPoint p = screenToBuf( QPoint(event->x(), event->y()) );
  // enable dragging ?
  if ( (event->buttons() & Qt::LeftButton) ) {
    QCursor tmp;
    tmp.setShape( Qt::SizeAllCursor );
    this->setCursor(tmp);

    _dragging.active = true;
    _dragging.start.setX((((double)event->x() / (_property.zoom_factor))) - _property.x);
    _dragging.start.setY( (((double) - event->y() / (_property.zoom_factor))) - _property.y);
  }

  if ( (event->buttons() & Qt::MidButton) != 0 ) {
    _marker->active = true;
    _marker->x = p.x();
    _marker->y = p.y();
    emit sigMarkerToImageWindow(*_marker);
  }

  // right command
  if ( (event->buttons() & Qt::RightButton) != 0) {
    _marker->active = !_marker->active;
    emit sigMarkerToImageWindow(*_marker);
  }

  update();
  emit sigPropertyChanged(this);
}

void GUI::Canvas::mouseMoveEvent(QMouseEvent* event) {
  QPoint p = screenToBuf(event->pos());

  // middle click --> set marker
  if ( (event->buttons() & Qt::MidButton) ) {
    _marker->active = true;
    _marker->x = p.x();
    _marker->y = p.y();
    update();
    emit sigMarkerToImageWindow(*_marker);
  }


  if ( (event->buttons() & Qt::LeftButton) != 0 && _dragging.active) {
    const double dx = event->x();
    const double dy = event->y();
    _property.x = (((dx / (_property.zoom_factor))) - _dragging.start.x());
    _property.y = ( ((-dy / (_property.zoom_factor))) - _dragging.start.y());
    update();
    emit sigPropertyChanged(this);
    emit sigUpdateScrollBars(this);
  }
  emit sigCoordToImageWindow(p);

  _focus = event->pos();
}


void GUI::Canvas::mouseReleaseEvent(QMouseEvent*) {
  if ( _dragging.active ) {
    QCursor tmp;
    tmp.setShape( Qt::ArrowCursor );
    this->setCursor(tmp);
    _dragging.active = false;
  }
}


void GUI::Canvas::wheelEvent( QWheelEvent * event) {
  if (_slides->available() > 0) {
    // QPoint q = QPoint(event->x(), event->y());
    zoom(event->pos(), event->delta());
  }
}

void GUI::Canvas::zoom(QPoint q, int delta) {
  QPoint p = screenToBuf(q);
  const double zoom1 = _property.zoom_factor;
  // checking bounds
  const bool sx = p.x() < 0;
  const bool sy = p.y() < 0;
  const bool bx = p.x() >= (int)_slides->width();
  const bool by = p.y() >= (int)_slides->height();

  const unsigned int winWidth = width();
  const unsigned int winHeight = height();

  const double zoom_delta = 1.4142135623;

  double mpercX = ((q.x()) / ((double)winWidth)) - 0.5;
  double mpercY = ((q.y()) / ((double)winHeight)) - 0.5;

  // change zoom value
  if ( delta > 0 )
    _property.zoom_factor *= zoom_delta;
  else
    _property.zoom_factor /= zoom_delta;

  // clip
  if (_property.zoom_factor <= 0.)
    _property.zoom_factor = 1.;

  // do not change relative position in image when zooming keep mouse pointer on same pixel
  double zoom2 = _property.zoom_factor;
  if ( sx || bx ) {
    _property.x = 0;
    _property.y = 0;
  } else if ( sy || by ) {
    _property.x = 0;
    _property.y = 0;
  } else {
    _property.x += ((double)winWidth * ((1.0 / zoom2) - (1.0 / zoom1)) * mpercX);
    _property.y -= ((double)winHeight * ((1.0 / zoom2) - (1.0 / zoom1)) * mpercY);
  }

  // update();
  // emit sigPropertyChanged(this);
  // emit sigUpdateScrollBars(this);
  slotUpdateCanvas();
  // emit sigCoordToImageWindow(p);

}

void GUI::Canvas::resizeGL(int width, int height) {
  _gl->set_size(width, height);
  _gl->projection_identity();
  _gl->set_ortho(width, height);
  _gl->modelview_identity();
}

void checkerboard(unsigned char* data,
                  unsigned int width = 512,
                  unsigned int height = 512,
                  unsigned int channels = 4) {
  for (unsigned int w = 0; w < width; w++ ) {
    for (unsigned int h = 0; h < height; h++ ) {
      for (unsigned int c = 0; c < channels; c++ ) {
        if (h % 2 == 1) {
          if (w % 2 == 1) {
            data[(h * width + w)*channels + c] = 255;
          } else {
            data[(h * width + w)*channels + c] = 0;
          }
        } else {
          if (w % 2 == 0) {
            data[(h * width + w)*channels + c] = 255;
          } else {
            data[(h * width + w)*channels + c] = 0;
          }
        }
      }
    }
  }
}

QPoint GUI::Canvas::screenToBuf( QPoint p ) const {
  if (!_slides->available())
    return QPoint(0, 0);

  const double image_width = _slides->width();
  const double image_height = _slides->height();
  const double zoom = _property.zoom_factor;
  const double alpha = _property.angle * -(M_PI / 180.0);

  const double bufX = _property.x;
  const double bufY = -_property.y;

  const double canvas_width = ((double)width()) - 1.0;
  const double canvas_height = ((double)height()) - 1.0;
  const double tmpWidth = image_width * zoom - (canvas_width);
  const double tmpHeight = image_height * zoom - (canvas_height);

  double x = ((double)((p.x())) + 0.5 * tmpWidth - bufX * zoom) - 0.5 * image_width * zoom;
  double y = (((double)p.y() - 1.0) + 0.5 * tmpHeight - bufY * zoom) - 0.5 * image_height * zoom;

  double px = (x * cos(alpha) - y * sin(alpha)) / zoom + 0.5 * image_width;
  double py = (y * cos(alpha) + x * sin(alpha)) / zoom + 0.5 * image_height;

  return QPoint( (int)px, (int)py);
}


GUI::Canvas::property_t GUI::Canvas::getProperty() {
  return _property;
}

void GUI::Canvas::setProperty( property_t property ) {
  if (_slides->available()) {
    _property = property;
    update();
  }
}

GUI::Marker GUI::Canvas::getMarker() {
  return *_marker;
}

void GUI::Canvas::setMarker( GUI::Marker marker ) {
  *_marker = marker;
  update();
}

void GUI::Canvas::updatePropertyByScrollbar( property_t property ) {

  _property.x = property.x;
  _property.y = property.y;
  emit sigPropertyChanged(this);
  update();
}

void GUI::Canvas::initializeGL() {
  qDebug() << "GUI::Canvas::initializeGL()";
  _gl->initializeOpenGLFunctions();
  _gl->printContextInformation();

  _gl->glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
  _gl->set_size(width(), height());
  _gl->projection_identity();
  _gl->modelview_identity();
  _gl->enable_texture_blend();

  // generate checkerboard texture
  _bg = new Utils::GlObject<unsigned char>(512, 512, 4);
  _bg->allocate();
  checkerboard(_bg->data, _bg->width, _bg->height, _bg->channels);

  _gl->prepare(_bg);
  _gl->draw(_bg, -4000, 4000, 4000, -4000, 0);


  delete[] _bg->data;
}


void GUI::Canvas::paintGL() {
  while ( !__sync_bool_compare_and_swap (&m_glBlock, false, true));

  _gl->identity();
  _gl->clear();

  // draw background
  _gl->draw(_bg, -4000, 4000, 4000, -4000, 0);

  if (_slides->available()) {
    // get corner points
    QPoint top_left = QPoint(0, 0);
    QPoint top_right = QPoint(width() - 1, 0);
    QPoint bottom_left = QPoint(0, height() - 1);
    QPoint bottom_right = QPoint(width() - 1, height() - 1);
    // map to image
    top_left = screenToBuf(top_left);
    top_right = screenToBuf(top_right);
    bottom_left = screenToBuf(bottom_left);
    bottom_right = screenToBuf(bottom_right);

    int left, right, top, bottom;
    left = std::min( top_left.x(), std::min( top_right.x(), std::min( bottom_left.x(), bottom_right.x() )));
    top = std::min( top_left.y(), std::min( top_right.y(), std::min( bottom_left.y(), bottom_right.y() )));
    right = std::max( top_left.x(), std::max( top_right.x(), std::max( bottom_left.x(), bottom_right.x() )));
    bottom = std::max( top_left.y(), std::max( top_right.y(), std::max( bottom_left.y(), bottom_right.y() )));

    glColor3d( 1.0, 0.0, 0.0 );
    glScaled(_property.zoom_factor, -_property.zoom_factor, 1.0);
    glTranslated(_property.x, -_property.y, 0.0);
    glRotatef(0, 0, 0, 1.0);


    const double image_width = _slides->width();
    const double image_height = _slides->height();
    glTranslated(-0.5 * image_width,
                 -0.5 * image_height, 0.0);

    _slides->draw(_gl, top, left,
                  bottom, right,
                  _property.zoom_factor);

    _gl->drawMarker(this, _marker);
  }

  m_glBlock = false;
}


void GUI::Canvas::slotPrevLayer() {
  _slides->backward();
  slotUpdateCanvas();
}

void GUI::Canvas::slotNextLayer() {
  _slides->forward();
  slotUpdateCanvas();
}


/*
QOpenGLFunctions *functions = QOpenGLContext::currentContext()->functions();
*/