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
class Histogram;

class ImageWindow  : public QMainWindow {
  Q_OBJECT

 public:
  ImageWindow(QWidget* parent, Window* parentWindow);
  QSize sizeHint() const;


  void loadImage(std::string fn);

  void keyPressEvent(QKeyEvent * event );

  bool event(QEvent *e);
  void closeEvent(QCloseEvent * event);

 protected:
  void dropEvent(QDropEvent *ev);
  void dragEnterEvent(QDragEnterEvent *ev);

 private slots:

 signals:
  void sigPrevLayer();
  void sigNextLayer();
  void sigRemoveCurrentLayer();

  void sigCommunicateWindowGeometry(ImageWindow*);
  void sigFocusChange(ImageWindow*);
  void sigImageWindowCloses(ImageWindow*);

 public slots:

  // get change from other view
  void slotReceiveCanvasChange(Canvas*);
  void slotReceiveLayerChange();

  void slotOpenImage();
  void slotCommunicateWindowGeometry();

  void slotRefreshBuffer();
  void slotRepaint();
  void slotRepaintStatusbar();
  void slotRepaintTitle();
  void slotRepaintSliders();
  void slotRepaintHistogram();
  void slotVertSliderMoved(int);
  void slotHorSliderMoved(int);
  void slotReceiveWindowGeometry(ImageWindow*);

 private:
  QGridLayout* _centerLayout;
  Canvas* _canvas;

  QScrollBar* _vertSlider;
  QScrollBar* _horSlider;
  QLabel* _statusLabelMouse;
  QLabel* _statusLabelPatch;
  QLabel* _statusLabelMarker;
  QLabel* _statusLabelZoom;

  QMenu* _fileMenu;
  QAction* _openImageAct;
  QAction* _removeImageAct;

  QMenu* _windowMenu;
  QAction* _newWindowAct;
  QAction* _dialogWindowAct;
  QAction* _closeWindowAct;
  QAction* _closeAppAct;

  Window* _parentWindow;

  QAction *_propagateWindowGeometryAct;
  QAction *_arangeWindowsAct;
  QAction *_fitImageAct;
  QAction *_centerImageAct;
  QAction *_zoomInAct;
  QAction *_zoomStdAct;
  QAction *_zoomOutAct;
  QAction *_zoomFitAct;
  QAction *_toggleMarkerAct;
  QAction *_refreshAct;

  QAction *_zoomInTestAct;
  QAction *_zoomOutTestAct;

  QMenu* _imageMenu;
  QAction *_resetHistogramAct;

  // toolbar
  QToolBar* _toolbar;
  Histogram* _toolbar_histogram;
};
}; // namespace GUI

#endif // IMAGE_WINDOW_CPP