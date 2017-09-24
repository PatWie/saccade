#ifndef IMAGE_WINDOW_CPP
#define IMAGE_WINDOW_CPP

#include <string>
#include <QtGui>
#include <QMainWindow>
#include <QGridLayout>
#include <QDropEvent>
#include <QUrl>
#include <QLabel>
#include <QScrollBar>

#include "canvas.h"
#include "marker.h"

namespace GUI {
class Slides;
class Window;
class AboutWindow;

class ImageWindow  : public QMainWindow {
  Q_OBJECT

 public:
  ImageWindow(QWidget* parent, Window* parentWindow);
  QSize sizeHint() const;

  void synchronize(bool active);
  void loadImage(std::string fn);

  void keyPressEvent( QKeyEvent * event );

 protected:
  void dropEvent(QDropEvent *ev);
  void dragEnterEvent(QDragEnterEvent *ev);

 private slots:

 signals:
  void sigUpdateConnectedViews(Canvas*);
  void sigToggleChained(ImageWindow*, bool);

  void sigPrevLayer();
  void sigNextLayer();

  void sigCoordToMainwindow(QPoint);
  void sigMarkerToMainwindow(Marker);
  void sigPropertyToMainwindow(Canvas::property_t);
  void sigSetZoomAction(double);

 public slots:
  void slotUpdateConnectedViews(Canvas*);
  void slotSynchronizeConnectedViews( Canvas*);
  void slotUpdateTitle(Canvas*);

  void slotPrevLayer();
  void slotNextLayer();

  void slotZoomStdAction();

  void slotOpenImageAction();

  void slotUpdateScrollBars(Canvas*);
  void slotVertScrollChanged(int);
  void slotHorScrollChanged(int);

  void slotCoordToMainWindow(QPoint);
  void slotMarkerToMainWindow(Marker);
  void slotPropertyToMainwindow(Canvas::property_t);

  void slotShowCoords(QPoint);
  void slotShowZoom(double p);
  void slotShowMarkers(Marker);
  void slotShowProperty(Canvas::property_t);


 private:
  QGridLayout* _centerLayout;
  Canvas* _canvas;

  QScrollBar* _vertScroll;
  QScrollBar* _horScroll;
  QLabel* _statusLabelMouse;
  QLabel* _statusLabelPatch;
  QLabel* _statusLabelMarker;
  QLabel* _statusLabelZoom;

  QMenu* _fileMenu;
  QAction* _openImageAct;

  QMenu* _windowMenu;
  QAction* _newWindowAct;
  QAction* _dialogWindowAct;
  QAction* _closeWindowAct;

  Window* _parentWindow;

  QAction *_zoomInAct;
  QAction *_zoomStdAct;
  QAction *_zoomOutAct;
};
}; // namespace GUI

#endif // IMAGE_WINDOW_CPP