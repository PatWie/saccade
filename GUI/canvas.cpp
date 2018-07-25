#include <iostream>
#include <cstring>

#include <QMouseEvent>

#include <glog/logging.h>

#include "image_window.h"
#include "canvas.h"
#include "layer.h"
#include "slides.h"
#include "marker.h"
#include "../Utils/gl_manager.h"
#include "../Utils/selection.h"

bool GUI::Canvas::_gl_block = false;

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
  _axis.pixel_size = 1;
  _axis.x = 0;
  _axis.y = 0;

  _width = 1;
  _height = 1;

  _selection.reset();
  _crop.reset();

  // no dragging without clicking
  _dragging.active = false;
  _dragging.start.setX(0.0);
  _dragging.start.setY(0.0);

  _slides = new Slides();
  _marker = new Marker();

}

const GUI::Layer* GUI::Canvas::layer(int i) const {
  // if there is any layer return it
  if (!_slides->available())
    return nullptr;
  if (i == -1)
    return _slides->current();
  else
    return _slides->operator[](i);
}

GUI::Layer* GUI::Canvas::layer(int i) {
  // if there is any layer return it
  // TODO: check const implementation (we need to remove this to apply batch ops)
  // if (!_slides->available())
  //   return nullptr;
  if (i == -1)
    return _slides->current();
  else
    return _slides->operator[](i);
}

const GUI::Slides* GUI::Canvas::slides() const {
  // return slide-manager
  // TODO: not yet decided if we really need an advanced std::vector<layer> container
  return _slides;
}

void GUI::Canvas::addLayer(Layer *layer) {

  connect(layer, &Layer::sigRefresh, this, &Canvas::slotCommunicateLayerChange);
  connect(layer, &Layer::sigHistogramFinished, this, &Canvas::slotCommunicateLayerChange);

  // canvas gets another image
  _slides->add(layer);
  slotCommunicateLayerChange();
}

QPoint GUI::Canvas::focusPixel() const {
  return _focus;
}

void GUI::Canvas::setFocusPixel(QPoint p) {
  _focus = p;
}

QSize GUI::Canvas::sizeHint() const {
  return QSize(512, 512);
}

void GUI::Canvas::slotRepaint() {
  update();
}

void GUI::Canvas::slotCommunicateLayerChange() {
  emit sigCommunicateLayerChange(this);
  update();
}

void GUI::Canvas::slotCommunicateCanvasChange() {
  emit sigCommunicateCanvasChange(this);
  update();
}


void GUI::Canvas::slotZoomIn() {
  const QPoint pos = this->mapFromGlobal(QCursor::pos());
  if (this->rect().contains(pos)) {
    zoom_rel(pos, 1);
  } else {
    zoomOnCenter(_axis.pixel_size * sqrt(2.0));
  }
}

void GUI::Canvas::slotZoomOut() {
  const QPoint pos = this->mapFromGlobal(QCursor::pos());
  if (this->rect().contains(pos)) {
    zoom_rel(pos, -1);
  } else {
    zoomOnCenter(_axis.pixel_size / sqrt(2.0));
  }
}

void GUI::Canvas::slotNoZoom() {
  zoomOnCenter(1.0);
}

void GUI::Canvas::toggleMarkerAtCursor() {
  _marker->active = !_marker->active;
  _marker->x = _focus.x();
  _marker->y = _focus.y();
  slotCommunicateCanvasChange();
}

void GUI::Canvas::slotShiftCanvas(int v, int h, bool jump) {

  const int pixelPerLine = width() / _axis.pixel_size;
  const float jump_percent = 5;
  if (jump) {
    h *= pixelPerLine / 100. * jump_percent;
    v *= pixelPerLine / 100. * jump_percent;
  }

  _axis.x += h;
  _axis.y += v;

  slotCommunicateCanvasChange();
}


/**
 * @brief move canvas such that pixel is in the center
 * @details [long description]
 *
 * @param imgCoord coordinate in image (w, h)
 * @return [description]
 */
void GUI::Canvas::zoomOnCenter(double amount) {

  const QPoint want = canvasToImg(QPoint(width() / 2., height() / 2.));
  _axis.pixel_size = amount;

  const QPoint actual = canvasToImg(QPoint(width() / 2., height() / 2.));
  _axis.x += actual.x() - want.x();
  _axis.y -= actual.y() - want.y();

  slotCommunicateCanvasChange();

}

void GUI::Canvas::slotKeyReleaseEvent (QKeyEvent *event){
  DLOG(INFO) << "keyReleaseEvent";
  DLOG(INFO) << event->text().toStdString();
  DLOG(INFO) << event->modifiers();
  DLOG(INFO) << event->nativeScanCode();

  if(event->nativeScanCode() == 50){
    DLOG(INFO) << "!pressedShift";
    _selection.setActive(false);
    update();
  }

}

void GUI::Canvas::mousePressEvent(QMouseEvent* event) {
  // get position within image coordinates
  _focus = canvasToImg(event->pos());

  Qt::KeyboardModifiers keymod = QGuiApplication::keyboardModifiers();
  const bool pressedShift = keymod == Qt::ShiftModifier;
  const bool pressedCtrl = keymod == Qt::ControlModifier;
  // const bool pressedAlt = keymod == Qt::AltModifier;

  // left click: enable dragging/shifting
  if ( (event->buttons() & Qt::LeftButton) ) {

    if (pressedShift) {
      _selection.setActive();
      _selection.setStart(_focus);
    } else if (pressedCtrl) {
      _crop.setActive();
      _crop.setStart(_focus);
    } else {
      QCursor tmp;
      tmp.setShape( Qt::SizeAllCursor );
      this->setCursor(tmp);

      _dragging.active = true;
      const double ex = event->x();
      const double ey = -event->y();
      _dragging.start.setX(ex / _axis.pixel_size - _axis.x);
      _dragging.start.setY(ey / _axis.pixel_size - _axis.y);
    }
  }

  // middle click: set marker
  if ( (event->buttons() & Qt::MidButton) != 0 ) {
    _marker->active = true;
    _marker->x = _focus.x();
    _marker->y = _focus.y();
  }

  // right click: remove marker
  if (( (event->buttons() & Qt::RightButton) != 0) && !pressedCtrl) {
    _marker->active = !_marker->active;
  }
  if (( (event->buttons() & Qt::RightButton) != 0) && pressedCtrl) {
    _crop.toggleActive();
  }

  slotCommunicateCanvasChange();
}

void GUI::Canvas::mouseMoveEvent(QMouseEvent* event) {
  _focus = canvasToImg(event->pos());

  Qt::KeyboardModifiers keymod = QGuiApplication::keyboardModifiers();
  const bool pressedShift = keymod == Qt::ShiftModifier;
  const bool pressedCtrl = keymod == Qt::ControlModifier;
  // const bool pressedAlt = keymod == Qt::AltModifier;

  // middle click active --> move marker
  if ( (event->buttons() & Qt::MidButton) ) {
    _marker->active = true;
    _marker->x = _focus.x();
    _marker->y = _focus.y();
  }

  if ( (event->buttons() & Qt::LeftButton) != 0 && (_selection.active())) {
    if (pressedShift) {
      _selection.setStop(_focus);
    } else {
      _selection.setActive(false);
    }


    update();
  } else if ( (event->buttons() & Qt::LeftButton) != 0 && _dragging.active) {
    const double dx = event->x();
    const double dy = event->y();
    _axis.x = (dx / _axis.pixel_size) - _dragging.start.x();
    _axis.y = -(dy / _axis.pixel_size) - _dragging.start.y();
  }

  if ((event->buttons() & Qt::LeftButton) != 0 && (_crop.active())) {
    if (pressedCtrl) {
      _crop.setStop(_focus);
    }
    // else {
    //   _crop.setActive(false);
    // }
  }

  slotCommunicateCanvasChange();
  update();

}


void GUI::Canvas::mouseReleaseEvent(QMouseEvent* event) {
  _focus = canvasToImg(event->pos());

  if (_selection.active()) {
    _selection.setActive(false);
    _selection.setStop(_focus);

    const QRect selected_area = _selection.rectangle();

    const QPoint want = selected_area.center();
    const double zoom_width = ((double)_width / (double)selected_area.width());
    const double zoom_height = ((double)_height / (double)selected_area.height());
    _axis.pixel_size = std::min(zoom_width, zoom_height);
    // _axis.pixel_size *= sqrt(2);

    const QPoint actual = canvasToImg(QPoint(width() / 2., height() / 2.));
    _axis.x += actual.x() - want.x();
    _axis.y -= actual.y() - want.y();

    slotCommunicateCanvasChange();

  } else if ( _dragging.active ) {
    QCursor tmp;
    tmp.setShape( Qt::ArrowCursor );
    this->setCursor(tmp);
    _dragging.active = false;
  } else if (_crop.active()) {
    if ((event->buttons() & Qt::LeftButton) != 0){
      _crop.setStop(_focus);
    }


  }
}


void GUI::Canvas::wheelEvent( QWheelEvent * event) {
  if (_slides->available() > 0) {
    zoom_rel(event->pos(), event->delta());
  }
}

void GUI::Canvas::zoom_rel(QPoint q, int delta) {

  const double zoom_delta = sqrt(2.0);
  const double old_Pixel_size =  _axis.pixel_size;

  double new_pixelsize = 0;
  if ( delta > 0 )
    new_pixelsize = _axis.pixel_size * zoom_delta;
  else
    new_pixelsize = _axis.pixel_size / zoom_delta;

  const unsigned int winWidth = width();
  const unsigned int winHeight = height();
  double mpercX = ((q.x()) / ((double)winWidth)) - 0.5;
  double mpercY = ((q.y()) / ((double)winHeight)) - 0.5;

  // change zoom value
  if (new_pixelsize <= 0.)
    new_pixelsize = 1.;

  QPoint q_img = canvasToImg(q);
  if ( (q_img.x() < 0) ||
       (q_img.y() < 0) ||
       (q_img.x() >= (int)_slides->width()) ||
       (q_img.y() >= (int)_slides->height())) {
    _axis.x = 0;
    _axis.y = 0;
    _axis.pixel_size = new_pixelsize;
  } else {
    _axis.x += ((double)winWidth * ((1.0 / new_pixelsize) - (1.0 / old_Pixel_size)) * mpercX);
    _axis.y -= ((double)winHeight * ((1.0 / new_pixelsize) - (1.0 / old_Pixel_size)) * mpercY);
    _axis.pixel_size = new_pixelsize;
  }

  slotCommunicateCanvasChange();
}

void GUI::Canvas::slotFitZoomToWindow() {
  DLOG(INFO) << "GUI::Canvas::slotFitZoomToWindow";
  const double zoom_width = ((double)_width / (double)_slides->width());
  const double zoom_height = ((double)_height / (double)_slides->height());
  _axis.pixel_size = std::min(zoom_width, zoom_height);

  _axis.x = 0;
  _axis.y = 0;

  slotCommunicateCanvasChange();
}

void GUI::Canvas::slotCenterImage() {
  _axis.x = 0;
  _axis.y = 0;

  slotCommunicateCanvasChange();
}

void GUI::Canvas::slotFitToImage() {
  const int img_width = _slides->width() * _axis.pixel_size;
  const int img_height = _slides->height() * _axis.pixel_size;

  const int padding_w = (_parentWin->width() - width());
  const int padding_h = (_parentWin->height() - height());

  _parentWin->resize(img_width + padding_w + 1, img_height + padding_h + 1);

  _axis.x = 0;
  _axis.y = 0;

  slotCommunicateCanvasChange();
}



void GUI::Canvas::resizeGL(int width, int height) {
  _width = width;
  _height = height;

  _gl->set_size(width, height);
  _gl->projection_identity();
  _gl->set_ortho(width, height);
  _gl->modelview_identity();
}

void GUI::Canvas::checkerboard(unsigned char* data,
                               unsigned int width,
                               unsigned int height,
                               unsigned int channels) {

  memset(data, 0, width * height * channels);

  for (unsigned int h = 0; h < height; h++ ) {
    const int off = h % 2;
    // for (unsigned int w = 0; w < width; w += 2 ) {
    for (unsigned int w = 0; w < width; w += 1 ) {
      for (unsigned int c = 0; c < channels; c++ ) {
        // data[(h * width + w + off)*channels + c] = 255;
        data[(h * width + w + off)*channels + c] = (w % 2 == 0) ? 89 : 43;
        // data[(h * width + w + off)*channels + c] = (w % 2 == 0) ? 86 : 43;
      }
    }
  }
}

/**
 * @brief from global canvas position to coordinate within image
 * @details [long description]
 *
 * @param p [description]
 * @return [description]
 */
QPoint GUI::Canvas::canvasToImg( QPoint p ) const {
  if (!_slides->available())
    return QPoint(0, 0);

  const double pixel_size = _axis.pixel_size;
  const double image_width = _slides->width();
  const double image_height = _slides->height();
  const double canvas_width = width() - 1.0;
  const double canvas_height = height() - 1.0;

  const double padding_w = 0.5 * (image_width * pixel_size - canvas_width);
  const double padding_h = 0.5 * (image_height * pixel_size - canvas_height);

  double px = p.x();
  double py = p.y() - 1.0;

  // with rotation
  // const double alpha = _axis.angle * -(M_PI / 180.0);
  // double x = (px + padding_w - _axis.x * pixel_size) - 0.5 * image_width * pixel_size;
  // double y = (py + padding_h + _axis.y * pixel_size) - 0.5 * image_height * pixel_size;
  // px = (x * cos(alpha) - y * sin(alpha)) / pixel_size + 0.5 * image_width;
  // py = (y * cos(alpha) + x * sin(alpha)) / pixel_size + 0.5 * image_height;

  px = (px + padding_w)  / pixel_size - _axis.x;
  py = (py + padding_h)  / pixel_size + _axis.y;

  return QPoint( (int)px, (int)py);
}

QPoint GUI::Canvas::imgToCanvas( QPoint p ) const {
  if (!_slides->available())
    return QPoint(0, 0);

  const double pixel_size = _axis.pixel_size;
  const double image_width = _slides->width();
  const double image_height = _slides->height();
  const double canvas_width = width() - 1.0;
  const double canvas_height = height() - 1.0;

  const double padding_w = 0.5 * (image_width * pixel_size - canvas_width);
  const double padding_h = 0.5 * (image_height * pixel_size - canvas_height);

  double px = p.x();
  double py = p.y();

  px = (px + _axis.x) * pixel_size - padding_w;
  py = (py - _axis.y) * pixel_size - padding_h + 1;

  return QPoint( (int)px, (int)py);
}

Utils::selection_t GUI::Canvas::crop() const {
  return _crop;
}

void GUI::Canvas::setCrop(Utils::selection_t c) {
  _crop = c;
}

GUI::Canvas::axis_t GUI::Canvas::axis() const {
  return _axis;
}


void GUI::Canvas::setAxis(axis_t a) {
  _axis = a;
}

GUI::Marker GUI::Canvas::marker() const {
  return *_marker;
}

void GUI::Canvas::setMarker(Marker m) {
  *_marker = m;
}


Utils::selection_t GUI::Canvas::selection() const {
  return _selection;
}

void GUI::Canvas::setSelection(Utils::selection_t s) {
  _selection = s;
}


void GUI::Canvas::slotReceiveProperty(axis_t axis) {
  _axis.x = axis.x;
  _axis.y = axis.y;
  slotCommunicateCanvasChange();
}

void GUI::Canvas::initializeGL() {
  DLOG(INFO) << "GUI::Canvas::initializeGL()";
  _gl->initializeOpenGLFunctions();
  _gl->printContextInformation();

  // _gl->glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
  _gl->glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
  while ( !__sync_bool_compare_and_swap (&_gl_block, false, true));

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
    top_left = canvasToImg(top_left);
    top_right = canvasToImg(top_right);
    bottom_left = canvasToImg(bottom_left);
    bottom_right = canvasToImg(bottom_right);

    int left, right, top, bottom;
    left = std::min( top_left.x(), std::min( top_right.x(), std::min( bottom_left.x(), bottom_right.x() )));
    top = std::min( top_left.y(), std::min( top_right.y(), std::min( bottom_left.y(), bottom_right.y() )));
    right = std::max( top_left.x(), std::max( top_right.x(), std::max( bottom_left.x(), bottom_right.x() )));
    bottom = std::max( top_left.y(), std::max( top_right.y(), std::max( bottom_left.y(), bottom_right.y() )));


    glColor3d( 1.0, 0.0, 0.0 );
    glScaled(_axis.pixel_size, -_axis.pixel_size, 1.0);
    glTranslated(_axis.x, -_axis.y, 0.0);
    glRotatef(0, 0, 0, 1.0);


    const double image_width = _slides->width();
    const double image_height = _slides->height();
    glTranslated(-0.5 * image_width,
                 -0.5 * image_height, 0.0);

    _slides->draw(_gl, top, left,
                  bottom, right,
                  _axis.pixel_size);

    _gl->drawMarker(this, _marker);
    if (_selection.active()) {
      _gl->drawSelection(this, _selection.rectangle());
    }

    if (_crop.active()) {

      // _gl->drawSelection(this, _crop.rect, 1., 0., 0.);
      const QRect cropping_region = _crop.rectangle();
      _gl->drawSelection(this, cropping_region, 251. / 255., 199. / 255., 99. / 255.);
      // on left
      {
        float leftBorder =  cropping_region.x();
        float bottomBorder =  QPoint(bottom_left).y();
        QRect left(top_left, QPoint(leftBorder, bottomBorder));
        _gl->drawHighlight(this, left);
      }
      // on right
      {
        QRect left(QPoint(cropping_region.topRight().x(), top_left.y()),
                   bottom_right);
        _gl->drawHighlight(this, left);
      }
      // on top
      {
        QRect left(QPoint(cropping_region.x(), top_left.y()),
                   cropping_region.topRight());
        _gl->drawHighlight(this, left);
      }

      // on bottom
      {
        QRect left(cropping_region.bottomLeft(),
                   QPoint(cropping_region.topRight().x(), bottom_right.y()) );
        _gl->drawHighlight(this, left);
      }
      // _gl->drawHighlight(this, _crop.rect);
    }
  }


  _gl_block = false;
}


void GUI::Canvas::slotPrevLayer() {
  _slides->backward();
  slotCommunicateLayerChange();
}

void GUI::Canvas::slotRemoveCurrentLayer() {
  if (_slides->available()) {
    _slides->remove();
    slotCommunicateLayerChange();
  }
  DLOG(INFO) << "remove layer";
}

void GUI::Canvas::slotRemoveAllLayers() {
  while (_slides->available()) {
    _slides->remove();
  }
  slotCommunicateLayerChange();
  DLOG(INFO) << "remove all layers";
}

void GUI::Canvas::slotNextLayer() {
  _slides->forward();
  slotCommunicateLayerChange();
}
