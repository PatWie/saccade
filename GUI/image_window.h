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
#include "../Utils/misc.h"


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

  /**
   * @brief add image as layer to current canvas
   * @param fn path to new image
   */
  void loadImage(std::string fn);

  void keyPressEvent(QKeyEvent * event );
  void closeEvent(QCloseEvent * event);

  bool event(QEvent *e);

 protected:
  /**
   * @brief support drag and drop of images
   */
  void dropEvent(QDropEvent *ev);
  void dragEnterEvent(QDragEnterEvent *ev);

 private slots:

 signals:
  void sigPrevLayer();
  void sigNextLayer();

  void sigCommunicatePrevLayer();
  void sigCommunicateNextLayer();

  void sigRemoveCurrentLayer();
  void sigShiftCanvas(int, int, bool);

  void sigCommunicateWindowGeometry(ImageWindow*);
  void sigFocusChange(ImageWindow*);
  void sigImageWindowCloses(ImageWindow*);

 public slots:

  /**
   * @brief update canvas to be in sync with the sender
   * @details sender can be the canvas it contains itself
   * 
   * @param sender canvas which asks for synchronization
   */
  void slotReceiveCanvasChange(Canvas*);

  /**
   * @brief receive request from canvas to update visible information
   * @details repaint connected widgets as histogram, title, sliders, ...
   * 
   * @param sender canvas which asks for synchronization
   */
  void slotReceiveLayerChange();

  /**
   * @brief Trigger "open file"dialog
   */
  void slotOpenImage();
  void slotSaveImage();
  void slotSaveCrop();

  /**
   * @brief request other windows to share same window geometry
   */
  void slotCommunicateWindowGeometry();

  /**
   * @brief image content has changed and the buffer needs to update
   */
  void slotRefreshBuffer(HistogramRefreshTarget);

  void slotRepaint();

  void slotRepaintStatusbar();
  void slotRepaintTitle();
  void slotRepaintSliders();
  void slotRepaintHistogram();

  void slotVertSliderMoved(int);
  void slotHorSliderMoved(int);

  /**
   * @brief Set own window geometry according sender
   * @details This only affects height and width
   */
  void slotReceiveWindowGeometry(ImageWindow*);

 private:
  Window* _parentWindow;

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
  QAction* _saveImageAct;
  QAction* _saveCropAct;
  QAction* _removeImageAct;
  QAction* _emptyCanvasAct;

  QMenu* _windowMenu;
  QAction* _newWindowAct;
  QAction* _dialogWindowAct;
  QAction* _closeWindowAct;
  QAction* _closeAppAct;

  QMenu* _imageMenu;
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

  QAction *_resetHistogramAct;
  QAction *_resetHistogramEntireCanvasAct;

  // toolbar
  QToolBar* _toolbar;
  Histogram* _toolbar_histogram;
};
}; // namespace GUI

#endif // IMAGE_WINDOW_CPP