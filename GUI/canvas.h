#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include "../Utils/gl_object.h"
#include "../Utils/selection.h"
#include "marker.h"

namespace Utils {
class GlManager;
}; // namespace Utils

namespace GUI {
class ImageWindow;
class Slides;
class Layer;
// class Marker;

class Canvas  : public QOpenGLWidget {
  Q_OBJECT
 public:
  /**
   * @brief internal coordinate system for OpenGL
   */
  struct property_t {
    // number of displayed pixel per real pixel
    double pixel_size = 1.;
    // not supported yet (should be rotation)
    double angle = 0;
    // shift canvas x to right and y to top
    double x, y;
  };

 private:
  QWidget* _parent;
  ImageWindow* _parentWin;

  // current OpenGL context for the window
  QOpenGLContext _ctx;

  // pointer to checkerboard background
  Utils::GlObject<unsigned char> *_bg;

  // information for dragging image (shift image by left click)
  struct drag_t {
    QPointF start;
    bool active;
  } _dragging;

  // information for zooming at particular area (SHIFT + left click)
  Utils::selection_t _selection;

  // information for zooming at particular area (SHIFT + left click)
  Utils::selection_t _crop;

  // coordinate system for current canvas
  property_t _property;

  // geometry of current canvas
  int _width, _height;

  // each canvas can have multiple layers collected as slides
  // currently enhanced std::vector<layer*>
  Slides* _slides;

  // wrapper for OpenGL functions
  Utils::GlManager *_gl;

  // position for marker in image
  Marker* _marker;

  // focus point in canvas for broadcasting to other views
  QPoint _focus;

 public:

  Canvas(QWidget *parent, ImageWindow* parentWin);
  QSize sizeHint() const;

  // methods required by OpenGL
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();



  /**
   * @brief add a new image to current canvas as an additional layer
   */
  void addLayer(Layer* layer);

  /**
   * @brief return current layer of specified layer
   * @param i layer-id
   */
  const Layer* layer(int i = -1) const;
  Layer* layer(int i = -1);

  // will be removed probably
  const Slides* slides() const;

  /**
   * @brief broadcast synchronization signal
   * @details something has changed in this view. The folowing information are broadcasted
   *   - internal coordinate system (shift, zoom)
   *   -> other views shift at same position
   *   -> scrollbars for other views are updated
   *   -> zoom-information (statusbar) is updated
   */
  void askSynchronization();

  // handle mouse actions
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void wheelEvent( QWheelEvent * event);

  /**
   * @brief Convert from canvas-system to image-system
   * @details useful for mapping mouse->pos() to image pixel
   *
   * @param p point relative to canvas: top-left of canvas is (0, 0)
   * @return point in image coordinate (might be outside of image)
   */

  QPoint canvasToImg( QPoint p ) const;
  /**
   * @brief Convert from image-system to canvas-system
   * @details not used yet
   *
   * @param p point relative to image: top-left of image is (0, 0), which might be invisible
   * @return point in canvas coordinate (might be outside of window)
   */
  QPoint imgToCanvas( QPoint p ) const;

  /**
   * @brief zoom entire image but keeping center
   * @details this is different to the scroll-wheel action
   *
   * @param  amount of zooming
   */
  void zoomOnCenter(double);

  Utils::selection_t crop() const;
  void setCrop(Utils::selection_t);


  /**
   * @brief get a copy of used OpenGL coordinate system
   */
  property_t property() const;
  void setProperty(property_t);

  // handle markers
  void setMarker(Marker marker);
  Marker marker() const;
  QPoint focusPixel() const;
  void setFocusPixel(QPoint);

 signals:
  void sigCommunicateCanvasChange(Canvas*);
  void sigCommunicateLayerChange(Canvas*);


 public slots:
  // request changes in other views
  void slotCommunicateCanvasChange();
  void slotCommunicateLayerChange();

  // get change from other view
  // void slotReceiveCanvasChange(Canvas*);
  void slotReceiveProperty(property_t property);

  void slotRepaint();
  void slotPrevLayer();
  void slotNextLayer();
  void slotRemoveCurrentLayer();
  void slotRemoveAllLayers();

  // zoom but keep center
  void slotZoomIn();
  void slotZoomOut();
  void slotNoZoom();

  void slotShiftCanvas(int, int, bool);

  void slotFitZoomToWindow();
  void slotCenterImage();
  void slotFitToImage();
  void toggleMarkerAtCursor();
  // void slotSetZoom();


 protected:
  /**
   * @brief zoom relative to point
   * @details it tries to zoom around the point
   * 
   * @param QPoint q point on focus
   * @param int delta zoom-direction
   */
  void zoom_rel(QPoint q, int delta);
  
  /**
   * @brief generate checkerboard pattern
   * @details background texture for canvas
   * 
   * @param char buffer
   * @param int width
   * @param int height
   * @param int channels
   */
  void checkerboard(unsigned char* data,
                    unsigned int width = 512,
                    unsigned int height = 512,
                    unsigned int channels = 4);
 private slots:

 public:
  static bool _gl_block;
};
}; // namespace GUI

#endif // BUFFER_H