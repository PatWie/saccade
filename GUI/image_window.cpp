#include <iomanip>
#include <sstream>

#include <glog/logging.h>

#include <QtWidgets>

#include "window.h"
#include "histogram.h"
#include "image_window.h"
#include "../Utils/image_data.h"
#include "marker.h"
#include "layer.h"
#include "slides.h"

GUI::ImageWindow::ImageWindow(QWidget* parent, GUI::Window* parentWindow)
  : QMainWindow(parent), _parentWindow(parentWindow) {

  setWindowTitle("Saccade - Canvas");

  QGroupBox* centralWidget = new QGroupBox(this);
  _centerLayout = new QGridLayout(centralWidget);

  _canvas = new GUI::Canvas(centralWidget, this);
  _vertSlider = new QScrollBar(Qt::Vertical, centralWidget);
  _horSlider = new QScrollBar(Qt::Horizontal, centralWidget);

  _centerLayout->addWidget(_canvas, 0, 0);
  _centerLayout->addWidget(_vertSlider, 0, 1);
  _centerLayout->addWidget(_horSlider, 1, 0);

  centralWidget->setLayout(_centerLayout);
  setCentralWidget(centralWidget);

  connect(_canvas, &Canvas::sigCommunicateCanvasChange,
          parentWindow, &GUI::Window::slotCommunicateCanvasChange);

  connect(parentWindow, &GUI::Window::sigReceiveCanvasChange,
          this, &GUI::ImageWindow::slotReceiveCanvasChange);

  connect(_canvas, &Canvas::sigCommunicateLayerChange,
          this, &GUI::ImageWindow::slotReceiveLayerChange);

  connect( _vertSlider, &QScrollBar::sliderMoved,
           this, &GUI::ImageWindow::slotVertSliderMoved);
  connect( _horSlider, &QScrollBar::sliderMoved,
           this, &GUI::ImageWindow::slotHorSliderMoved);

  _canvas->slotRepaint();

  // toolbar
  _toolbar_histogram = new Histogram(this);
  _toolbar = new QToolBar(tr("histogram"));
  _toolbar->setMovable( false );
  _toolbar->addWidget( _toolbar_histogram );
  addToolBar(Qt::TopToolBarArea, _toolbar);

  // statusbar
  // ==========================================================
  _statusLabelMouse = new QLabel("(0, 0)");
  statusBar()->addWidget(_statusLabelMouse, 1);
  _statusLabelPatch = new QLabel("0, 0, 0");
  statusBar()->addWidget(_statusLabelPatch, 1);
  _statusLabelMarker = new QLabel("");
  statusBar()->addWidget(_statusLabelMarker, 1);
  _statusLabelZoom = new QLabel("zoom: 1");
  statusBar()->addWidget(_statusLabelZoom, 1);
  statusBar()->setSizeGripEnabled ( false );


  // slides
  // ==========================================================

  connect( this, &GUI::ImageWindow::sigPrevLayer,
           _canvas, &GUI::Canvas::slotPrevLayer);
  connect( this, &GUI::ImageWindow::sigNextLayer,
           _canvas, &GUI::Canvas::slotNextLayer);
  connect( this, &GUI::ImageWindow::sigRemoveCurrentLayer,
           _canvas, &GUI::Canvas::slotRemoveCurrentLayer);


  // menubar
  // ==========================================================
  _openImageAct = new QAction(tr("&Open"), this );
  _openImageAct->setShortcut(tr("Ctrl+O"));
  _openImageAct->setStatusTip(tr("Open an existing image"));
  connect(_openImageAct, &QAction::triggered, this, &GUI::ImageWindow::slotOpenImage);

  _removeImageAct = new QAction(tr("&Remove"), this );
  _removeImageAct->setShortcut(tr("Del"));
  _removeImageAct->setStatusTip(tr("Remove the current image"));
  connect(_removeImageAct, &QAction::triggered, _canvas, &GUI::Canvas::slotRemoveCurrentLayer);

  _newWindowAct = new QAction(tr("&New"), this );
  _newWindowAct->setShortcut(tr("Ctrl+N"));
  _newWindowAct->setStatusTip(tr("Create a new Window"));
  connect(_newWindowAct, &QAction::triggered, _parentWindow, &GUI::Window::slotNewWindowAction);

  _propagateWindowGeometryAct = new QAction(tr("Propagate &Geometry"), this );
  _propagateWindowGeometryAct->setShortcut(tr("F2"));
  _propagateWindowGeometryAct->setStatusTip(tr("Resize all other windows to the same size"));
  connect(_propagateWindowGeometryAct, &QAction::triggered, this, &GUI::ImageWindow::slotCommunicateWindowGeometry);

  _arangeWindowsAct = new QAction(tr("Arange &Windows"), this );
  _arangeWindowsAct->setShortcut(tr("F3"));
  _arangeWindowsAct->setStatusTip(tr("Try to arange all windows."));
  connect(_arangeWindowsAct, &QAction::triggered, _parentWindow, &GUI::Window::slotReceiveArangeWindows);

  _centerImageAct = new QAction(tr("center image"), this );
  _centerImageAct->setShortcut(tr("Ctrl+C"));
  _centerImageAct->setStatusTip(tr("Center image within canvas."));
  connect(_centerImageAct, &QAction::triggered, _canvas, &GUI::Canvas::slotCenterImage);

  _fitImageAct = new QAction(tr("Fit to image"), this );
  _fitImageAct->setShortcut(tr("Ctrl+F"));
  _fitImageAct->setStatusTip(tr("Fit window to image."));
  connect(_fitImageAct, &QAction::triggered, _canvas, &GUI::Canvas::slotFitToImage);

  _resetHistogramAct = new QAction(tr("&Reset the histogram"), this );
  _resetHistogramAct->setShortcut(tr("Ctrl+H"));
  _resetHistogramAct->setStatusTip(tr("Reset the histogram range"));
  connect(_resetHistogramAct, &QAction::triggered, _toolbar_histogram, &GUI::Histogram::slotResetRange);

  _dialogWindowAct = new QAction(tr("&About"), this );
  _dialogWindowAct->setShortcut(tr("F1"));
  _dialogWindowAct->setStatusTip(tr("About"));
  connect(_dialogWindowAct, &QAction::triggered, _parentWindow, &GUI::Window::slotDialogWindowAction);

  _closeWindowAct = new QAction(tr("E&xit"), this );
  _closeWindowAct->setShortcut(tr("Ctrl+W"));
  _closeWindowAct->setStatusTip(tr("Close the window"));
  connect(_closeWindowAct, &QAction::triggered, this, &GUI::ImageWindow::close);

  _closeAppAct = new QAction(tr("&Quit"), this );
  _closeAppAct->setShortcut(tr("Ctrl+Q"));
  _closeAppAct->setStatusTip(tr("Close the app"));
  connect(_closeAppAct, &QAction::triggered, _parentWindow, &GUI::Window::close);


  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_openImageAct);
  _fileMenu->addAction(_removeImageAct);

  _windowMenu = menuBar()->addMenu(tr("&Window"));
  _windowMenu->addAction(_newWindowAct);
  _windowMenu->addAction(_propagateWindowGeometryAct);
  _windowMenu->addAction(_arangeWindowsAct);
  _windowMenu->addAction(_fitImageAct);
  _windowMenu->addAction(_centerImageAct);
  _windowMenu->addAction(_dialogWindowAct);
  _windowMenu->addAction(_closeWindowAct);
  _windowMenu->addAction(_closeAppAct);

  _imageMenu = menuBar()->addMenu(tr("&Image"));
  _imageMenu->addAction(_resetHistogramAct);

  _zoomInAct = new QAction(tr("Zoom in"), this);
  _zoomInAct->setStatusTip(tr("Zoom one step into image"));
  _zoomInAct->setShortcut(Qt::Key_Plus | Qt::CTRL);

  _zoomStdAct = new QAction(tr("Zoom default"), this);
  _zoomStdAct->setStatusTip(tr("Reset zoom to original resolution"));
  _zoomStdAct->setShortcut(Qt::Key_0 | Qt::CTRL);

  _zoomOutAct = new QAction(tr("Zoom out"), this);
  _zoomOutAct->setStatusTip(tr("Zoom one step out of image"));
  _zoomOutAct->setShortcut(Qt::Key_Minus | Qt::CTRL);

  _zoomFitAct = new QAction(tr("Zoom fit window"), this);
  _zoomFitAct->setStatusTip(tr("Resize image to match window"));
  _zoomFitAct->setShortcut(Qt::Key_9 | Qt::CTRL);

  _toggleMarkerAct = new QAction(tr("Toggle marker"), this);
  _toggleMarkerAct->setStatusTip(tr("Toggle marker at current mouse position"));
  _toggleMarkerAct->setShortcut(Qt::Key_M | Qt::CTRL);


  connect(_zoomInAct, &QAction::triggered, _canvas, &GUI::Canvas::slotZoomIn);
  connect(_zoomStdAct, &QAction::triggered, _canvas, &GUI::Canvas::slotNoZoom);
  connect(_zoomOutAct, &QAction::triggered, _canvas, &GUI::Canvas::slotZoomOut);
  connect(_zoomFitAct, &QAction::triggered, _canvas, &GUI::Canvas::slotFitZoomToWindow);
  connect(_toggleMarkerAct, &QAction::triggered, _canvas, &GUI::Canvas::toggleMarkerAtCursor);

  this->addAction(_zoomInAct);
  this->addAction(_zoomStdAct);
  this->addAction(_zoomOutAct);
  this->addAction(_zoomFitAct);

  _imageMenu->addAction(_zoomInAct);
  _imageMenu->addAction(_zoomStdAct);
  _imageMenu->addAction(_zoomOutAct);
  _imageMenu->addAction(_zoomFitAct);
  _imageMenu->addAction(_toggleMarkerAct);
  setAcceptDrops(true);

  connect(_toolbar_histogram, &Histogram::sigRefreshBuffer,
          this, &GUI::ImageWindow::slotRefreshBuffer);

}


bool GUI::ImageWindow::event(QEvent *e) {
  if (e->type() == QEvent::WindowActivate) {
    emit sigFocusChange(this);
  }
  return QWidget::event(e);
}

void GUI::ImageWindow::slotRefreshBuffer() {
  LOG(INFO) << "GUI::Window::slotRefreshBuffer()";
  LOG(INFO) << _toolbar_histogram->data()->range()->min << " "
            <<  _toolbar_histogram->data()->range()->max;

  Layer *layer = _canvas->layer();
  if (layer != nullptr) {
    const double bin_width = _toolbar_histogram->data()->image()->max() /
                             static_cast<double>(256);
    LOG(INFO) << "bin_width = " << bin_width;
    layer->slotRefresh(_toolbar_histogram->data()->range()->min * bin_width,
                       _toolbar_histogram->data()->range()->max * bin_width);
  }
}


void GUI::ImageWindow::dropEvent(QDropEvent *ev) {
  QList<QUrl> urls = ev->mimeData()->urls();
  foreach (QUrl url, urls) {
    if (Utils::ImageData::validFile(url.toLocalFile().toStdString())) {
      LOG(INFO) << "dropped " << url.toLocalFile().toStdString();
      loadImage(url.toLocalFile().toStdString());
    }
  }
}

void GUI::ImageWindow::dragEnterEvent(QDragEnterEvent *ev) {
  ev->accept();
}

void GUI::ImageWindow::loadImage(std::string fn) {
  Layer *layer = new Layer();
  layer->loadImage(fn);

  connect(layer, &Layer::sigHistogramFinished,
          this, &GUI::ImageWindow::slotRepaint);

  _canvas->addLayer(layer);
}

void GUI::ImageWindow::slotOpenImage() {
  LOG(INFO) << "GUI::Window::slotOpenImage()";

  QStringList filenames = QFileDialog::getOpenFileNames(this,
                          tr("Open Image"), _parentWindow->_openPath,
                          tr("Image Files (*.png *.jpg *.jpeg *.bmp *.ppm *.tif *.CR2 *.JPG *.JPEG, *.JPE)"));

  if ( !filenames.isEmpty() ) {
    for (int i = 0; i < filenames.count(); i++)
      loadImage(filenames.at(i).toStdString());
  }
}


QSize GUI::ImageWindow::sizeHint() const {
  return QSize(512, 512);
}


void GUI::ImageWindow::slotReceiveCanvasChange(Canvas* sender) {
  if ( _canvas != sender ) {
    // update canvas
    _canvas->setProperty(sender->property());
    _canvas->setFocusPixel(sender->focusPixel());
    _canvas->setMarker(sender->marker());

    slotRepaint();

  }
  slotRepaintStatusbar();
  slotRepaintSliders();
}

void GUI::ImageWindow::slotReceiveLayerChange() {
  LOG(INFO) << "GUI::ImageWindow::slotReceiveLayerChange()";
  slotRepaintHistogram();
  slotRepaintStatusbar();
  slotRepaintTitle();
  slotRepaintSliders();
}


void GUI::ImageWindow::keyPressEvent( QKeyEvent * event ) {

  switch (event->key()) {
  case Qt::Key_Left:
  case Qt::Key_Down:
    emit sigPrevLayer();
    break;
  case Qt::Key_Right:
  case Qt::Key_Up:
    emit sigNextLayer();
    break;
  default:
    QMainWindow::keyPressEvent(event);
  }
  QMainWindow::keyPressEvent(event);
}


// TODO: mirrored
void GUI::ImageWindow::slotVertSliderMoved(int value) {
  Canvas::property_t p = _canvas->property();
  p.y = value;
  _canvas->slotReceiveProperty(p);
}

// TODO: mirrored
void GUI::ImageWindow::slotHorSliderMoved(int value) {
  Canvas::property_t p = _canvas->property();
  p.x = -value;
  _canvas->slotReceiveProperty(p);
}

void GUI::ImageWindow::slotRepaint() {
  if (_canvas->layer() == nullptr)
    return;
  const GUI::Layer *current = _canvas->slides()->current();
  if (current != nullptr) {
    _canvas->slotRepaint();
  }
}


void GUI::ImageWindow::slotRepaintTitle() {
  if (_canvas->layer() == nullptr)
    return;

  const GUI::Layer *current = _canvas->slides()->current();
  if (current != nullptr) {
    // update title
    setWindowTitle(("Saccade - " + current->path()).c_str());
  }
}

void GUI::ImageWindow::slotRepaintStatusbar() {
  if (_canvas->layer() == nullptr){
    _statusLabelMouse->setText("");
    _statusLabelPatch->setText("");
    _statusLabelMarker->setText("");
    _statusLabelZoom->setText("");
    return;
  }

  const GUI::Layer *current = _canvas->slides()->current();
  if (current != nullptr) {

    // update pixel
    std::stringstream pixelPosText;
    const QPoint p = _canvas->focusPixel();
    pixelPosText << "(" << p.y() << ", " << p.x() << ")";
    _statusLabelMouse->setText(pixelPosText.str().c_str());

    std::string pixelColorText = "" + _canvas->slides()->current()->img()->color(p.y(), p.x());
    _statusLabelPatch->setText(pixelColorText.c_str());

    // update marker
    Marker m = _canvas->marker();
    std::string markerText = "";
    if (m.active) {
      markerText = "marker: (" + std::to_string((int)m.y) + ", " + std::to_string((int)m.x) + ")";
      markerText = markerText + " " + _canvas->slides()->current()->img()->color(m.y, m.x);
    }
    _statusLabelMarker->setText(markerText.c_str());

    // update zoom
    std::ostringstream zoomText;
    zoomText << "zoom: " << std::setprecision(3) << _canvas->property().pixel_size;
    _statusLabelZoom->setText(zoomText.str().c_str());

  }
}

void GUI::ImageWindow::slotRepaintSliders() {
  if (_canvas->layer() == nullptr)
    return;
  const int imgWidth = _canvas->layer()->width();
  const int imgHeight = _canvas->layer()->height();

  const int winWidth = _canvas->width();
  const int winheight = _canvas->height();

  GUI::Canvas::property_t prop = _canvas->property();

  if ( imgWidth * prop.pixel_size < winWidth ) {
    _horSlider->setRange(0, 0);
    _horSlider->setValue(0);
  } else {
    const int radius = (int) (0.5 * ((double)imgWidth - (double)winWidth / prop.pixel_size));
    _horSlider->setRange(-radius - 1, radius + 1);
    _horSlider->setValue(-(int)prop.x);
  }
  if ( imgHeight * prop.pixel_size < winheight ) {
    _vertSlider->setRange(0, 0);
    _vertSlider->setValue(0);
  } else {
    const int radius = (int) (0.5 * ((double)imgHeight - (double)winheight / prop.pixel_size));
    _vertSlider->setRange(-radius - 1, radius + 1);
    _vertSlider->setValue((int)prop.y);
  }
}

void GUI::ImageWindow::slotRepaintHistogram() {
  if (_canvas->layer() == nullptr){
    _toolbar_histogram->setData(nullptr);
    _toolbar_histogram->update();
    return;
  }

  const GUI::Layer *current = _canvas->slides()->current();
  if (current != nullptr) {
    // update histogram
    _toolbar_histogram->setData(current->histogram());
    _toolbar_histogram->update();
  }
}


void GUI::ImageWindow::slotCommunicateWindowGeometry() {
  // LOG(INFO) << "GUI::ImageWindow::slotCommunicateWindowGeometry";
  emit sigCommunicateWindowGeometry(this);
}

void GUI::ImageWindow::slotReceiveWindowGeometry(ImageWindow* sender) {
  // LOG(INFO) << "GUI::ImageWindow::slotReceiveWindowGeometry";
  if ( this != sender ) {
    resize(sender->width(), sender->height());
  }
}