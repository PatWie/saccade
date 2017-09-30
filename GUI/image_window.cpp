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

  setWindowTitle("EagleEye - Canvas");

  QGroupBox* centralWidget = new QGroupBox(this);
  _centerLayout = new QGridLayout(centralWidget);

  _canvas = new GUI::Canvas(centralWidget, this);
  _vertScroll = new QScrollBar(Qt::Vertical, centralWidget);
  _horScroll = new QScrollBar(Qt::Horizontal, centralWidget);

  _centerLayout->addWidget(_canvas, 0, 0);
  _centerLayout->addWidget(_vertScroll, 0, 1);
  _centerLayout->addWidget(_horScroll, 1, 0);

  centralWidget->setLayout(_centerLayout);
  setCentralWidget(centralWidget);

  connect(_canvas, SIGNAL(sigPropertyChanged(Canvas*)),
          this, SLOT(slotUpdateConnectedViews(Canvas*)));

  connect(_canvas, SIGNAL(sigUpdateTitle(Canvas*)),
          this, SLOT(slotUpdateTitle(Canvas*)));

  connect(_canvas, SIGNAL(sigUpdateScrollBars(Canvas*)),
          this, SLOT(slotUpdateScrollBars(Canvas*)));

  connect(_canvas, SIGNAL(sigUpdateLayer(Canvas*)),
          this, SLOT(slotUpdateLayer(Canvas*)));

  connect( _vertScroll, SIGNAL(sliderMoved( int )),
           this, SLOT(slotVertScrollChanged( int )));
  connect( _horScroll, SIGNAL(sliderMoved( int )),
           this, SLOT(slotHorScrollChanged( int )));

  connect( _canvas, SIGNAL(sigCoordToImageWindow(QPoint)),
           this, SLOT(slotCoordToMainWindow(QPoint)));

  connect( _canvas, SIGNAL(sigMarkerToImageWindow(Marker)),
           this, SLOT(slotMarkerToMainWindow(Marker)));

  connect( _canvas, SIGNAL(sigPropertyToImagewindow(Canvas::property_t)),
           this, SLOT(slotPropertyToMainwindow(Canvas::property_t)));

  _canvas->slotUpdateCanvas();

  // statusbar
  // ==========================================================
  _statusLabelMouse = new QLabel("pos:");
  statusBar()->addWidget(_statusLabelMouse, 1);
  _statusLabelPatch = new QLabel("rgb:");
  statusBar()->addWidget(_statusLabelPatch, 1);
  _statusLabelMarker = new QLabel("");
  statusBar()->addWidget(_statusLabelMarker, 1);
  _statusLabelZoom = new QLabel("zoom: 1");
  statusBar()->addWidget(_statusLabelZoom, 1);
  statusBar()->setSizeGripEnabled ( false );


  // slides
  // ==========================================================

  connect(this, SIGNAL(sigPrevLayer()),
          _canvas, SLOT(slotPrevLayer()));
  connect(this, SIGNAL(sigNextLayer()),
          _canvas, SLOT(slotNextLayer()));
  connect( this, SIGNAL(sigRemoveCurrentLayer()),
           _canvas, SLOT(slotRemoveCurrentLayer()));

  // menubar
  // ==========================================================
  _openImageAct = new QAction(tr("&Open"), this );
  _openImageAct->setShortcut(tr("Ctrl+O"));
  _openImageAct->setStatusTip(tr("Open an existing image"));
  connect(_openImageAct, SIGNAL(triggered()), this, SLOT(slotOpenImageAction()));

  _removeImageAct = new QAction(tr("&Remove"), this );
  _removeImageAct->setShortcut(tr("Del"));
  _removeImageAct->setStatusTip(tr("Remove the current image"));
  connect(_removeImageAct, SIGNAL(triggered()), this, SLOT(slotRemoveImageAction()));


  _newWindowAct = new QAction(tr("&New"), this );
  _newWindowAct->setShortcut(tr("Ctrl+N"));
  _newWindowAct->setStatusTip(tr("Create a new Window"));
  connect(_newWindowAct, SIGNAL(triggered()), _parentWindow, SLOT(slotNewWindowAction()));

  _dialogWindowAct = new QAction(tr("&About"), this );
  _dialogWindowAct->setShortcut(tr("F1"));
  _dialogWindowAct->setStatusTip(tr("About"));
  connect(_dialogWindowAct, SIGNAL(triggered()), _parentWindow, SLOT(slotDialogWindowAction()));

  _closeWindowAct = new QAction(tr("E&xit"), this );
  _closeWindowAct->setShortcut(tr("Ctrl+W"));
  _closeWindowAct->setStatusTip(tr("Close the window"));
  connect(_closeWindowAct, SIGNAL(triggered()), this, SLOT(close()));

  _closeAppAct = new QAction(tr("E&xit"), this );
  _closeAppAct->setShortcut(tr("Ctrl+Q"));
  _closeAppAct->setStatusTip(tr("Close the app"));
  connect(_closeAppAct, SIGNAL(triggered()), _parentWindow, SLOT(close()));

  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_openImageAct);
  _fileMenu->addAction(_removeImageAct);

  _windowMenu = menuBar()->addMenu(tr("&Window"));
  _windowMenu->addAction(_newWindowAct);
  _windowMenu->addAction(_dialogWindowAct);
  _windowMenu->addAction(_closeWindowAct);
  _windowMenu->addAction(_closeAppAct);

  _zoomInAct = new QAction(this);
  _zoomInAct->setShortcut(Qt::Key_Plus | Qt::CTRL);

  _zoomStdAct = new QAction(this);
  _zoomStdAct->setShortcut(Qt::Key_0 | Qt::CTRL);

  _zoomOutAct = new QAction(this);
  _zoomOutAct->setShortcut(Qt::Key_Minus | Qt::CTRL);

  _zoomFitAct = new QAction(this);
  _zoomFitAct->setShortcut(Qt::Key_9 | Qt::CTRL);

  connect(_zoomInAct, SIGNAL(triggered()),
          _canvas, SLOT(slotZoomInAction()));
  connect(_zoomStdAct, SIGNAL(triggered()),
          this, SLOT(slotZoomStdAction()));
  connect(_zoomOutAct, SIGNAL(triggered()),
          _canvas, SLOT(slotZoomOutAction()));
  connect(_zoomFitAct, SIGNAL(triggered()),
          _canvas, SLOT(slotFitZoomToWindow()));

  connect(this, SIGNAL(sigSetZoomAction(double)),
          _canvas, SLOT(slotSetZoomAction(double)));

  this->addAction(_zoomInAct);
  this->addAction(_zoomStdAct);
  this->addAction(_zoomOutAct);
  this->addAction(_zoomFitAct);

  setAcceptDrops(true);

  // toolbar
  _toolbar_histogram = new Histogram(this);
  _toolbar = new QToolBar(tr("histogram"));
  _toolbar->setMovable( false );
  _toolbar->addWidget( _toolbar_histogram );
  addToolBar(Qt::TopToolBarArea, _toolbar);

  connect(_toolbar_histogram, SIGNAL(sigRefreshBuffer()),
          this, SLOT(slotRefreshBuffer()));

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
    layer->slotRefresh(_toolbar_histogram->data()->range()->min,
                       _toolbar_histogram->data()->range()->max);
  }
}

void GUI::ImageWindow::slotZoomStdAction() {
  emit sigSetZoomAction(1.0);
}


void GUI::ImageWindow::dropEvent(QDropEvent *ev) {
  QList<QUrl> urls = ev->mimeData()->urls();
  foreach (QUrl url, urls) {
    LOG(INFO) << url.toLocalFile().toStdString();
    loadImage(url.toLocalFile().toStdString());
  }
}

void GUI::ImageWindow::dragEnterEvent(QDragEnterEvent *ev) {
  ev->accept();
}

void GUI::ImageWindow::loadImage(std::string fn) {
  Layer *layer = new Layer();
  layer->loadImage(fn);
  connect(layer, SIGNAL(sigHistogramFinished()),
          this, SLOT(slotUpdateLayer()));
  _canvas->addLayer(layer);
}

void GUI::ImageWindow::slotOpenImageAction() {
  LOG(INFO) << "GUI::Window::slotOpenImageAction()";

  QStringList filenames = QFileDialog::getOpenFileNames(this,
                          tr("Open Image"), _parentWindow->_openPath, tr("Image Files (*.png *.jpg *.bmp)"));

  if ( !filenames.isEmpty() ) {
    for (int i = 0; i < filenames.count(); i++)
      loadImage(filenames.at(i).toStdString());
    // _parentWindow->_openPath = QFileInfo(filenames.at(0)).absolutePath();
  }
  // _canvas->slotUpdateCanvas();
}

void GUI::ImageWindow::slotRemoveImageAction() {
  LOG(INFO) << "GUI::ImageWindow::slotRemoveImageAction";
  emit sigRemoveCurrentLayer();
}

QSize GUI::ImageWindow::sizeHint() const {
  return QSize(512, 512);
}

void GUI::ImageWindow::slotUpdateConnectedViews(Canvas* p) {
  emit sigUpdateConnectedViews(p);
}


void GUI::ImageWindow::slotSynchronizeConnectedViews(Canvas* canvas) {
  if ( _canvas != canvas ) {
    auto p = canvas->getProperty();
    _canvas->setProperty(p);
    slotShowZoom(p.zoom_factor);
    slotUpdateScrollBars(_canvas);
  }
}

void GUI::ImageWindow::slotUpdateTitle(Canvas* canvas) {
  if (canvas->layer() != nullptr)
    setWindowTitle(("EagleEye - " + canvas->layer()->path()).c_str());
}

void GUI::ImageWindow::synchronize(bool active) {
  emit sigToggleChained(this, active);
}

void GUI::ImageWindow::keyPressEvent( QKeyEvent * event ) {

  switch (event->key()) {
  case Qt::Key_Left:
  case Qt::Key_Down:
    slotPrevLayer();
    break;
  case Qt::Key_Right:
  case Qt::Key_Up:
    slotNextLayer();
    break;
  default:
    QMainWindow::keyPressEvent(event);
  }
  QMainWindow::keyPressEvent(event);
}

void GUI::ImageWindow::slotPrevLayer() {
  emit sigPrevLayer();
}
void GUI::ImageWindow::slotNextLayer() {
  emit sigNextLayer();
}

void GUI::ImageWindow::slotUpdateLayer(Canvas* canvas) {
  LOG(INFO) << "GUI::ImageWindow::slotUpdateLayer()";
  const GUI::Layer *current = canvas->slides()->current();
  if (current != nullptr) {
    _toolbar_histogram->setData(current->histogram());
    _toolbar_histogram->update();
  }

}
void GUI::ImageWindow::slotUpdateScrollBars(Canvas* canvas) {
  if (canvas->layer() == nullptr)
    return;
  const int imgWidth = canvas->layer()->width();
  const int imgHeight = canvas->layer()->height();

  const int winWidth = canvas->width();
  const int winheight = canvas->height();

  GUI::Canvas::property_t prop = canvas->getProperty();

  if ( imgWidth * prop.zoom_factor < winWidth ) {
    _horScroll->setRange(0, 0);
    _horScroll->setValue(0);
  } else {
    const int radius = (int) (0.5 * ((double)imgWidth - (double)winWidth / prop.zoom_factor));
    _horScroll->setRange(-radius - 1, radius + 1);
    _horScroll->setValue(-(int)prop.x);
  }
  if ( imgHeight * prop.zoom_factor < winheight ) {
    _vertScroll->setRange(0, 0);
    _vertScroll->setValue(0);
  } else {
    const int radius = (int) (0.5 * ((double)imgHeight - (double)winheight / prop.zoom_factor));
    _vertScroll->setRange(-radius - 1, radius + 1);
    _vertScroll->setValue((int)prop.y);
  }
}

// TODO: mirrored
void GUI::ImageWindow::slotVertScrollChanged( int value ) {
  Canvas::property_t p = _canvas->getProperty();
  p.y = value;
  _canvas->updatePropertyByScrollbar(p);
}

// TODO: mirrored
void GUI::ImageWindow::slotHorScrollChanged( int value ) {
  Canvas::property_t p = _canvas->getProperty();
  p.x = -value;
  _canvas->updatePropertyByScrollbar(p);
}

void GUI::ImageWindow::slotCoordToMainWindow(QPoint p) {
  emit sigCoordToMainwindow(p);
}

void GUI::ImageWindow::slotMarkerToMainWindow(Marker m) {
  emit sigMarkerToMainwindow(m);
}

void GUI::ImageWindow::slotPropertyToMainwindow(Canvas::property_t p) {
  emit sigPropertyToMainwindow(p);
}


void GUI::ImageWindow::slotShowCoords(QPoint p) {
  std::stringstream stream;
  stream << "pos: " << p.y() << ", " << p.x();

  std::string str = stream.str();
  _statusLabelMouse->setText(str.c_str());

  if (_canvas->layer() != nullptr) {
    std::stringstream stream;
    stream << std::setprecision(3);
    stream << "rgb: ";
    const int ch = _canvas->slides()->current()->height();
    const int cw = _canvas->slides()->current()->width();

    const int h = p.y();
    const int w = p.x();

    if (0 <= w && w < cw)
      if (0 <= h && h < ch) {
        // within image
        for (int c = 0; c < _canvas->slides()->current()->img()->channels(); ++c) {
          stream << _canvas->slides()->current()->img()->value(h, w, c) << " ";
        }
      }
    _statusLabelPatch->setText(stream.str().c_str());
  }
}

void GUI::ImageWindow::slotShowZoom(double p) {
  std::ostringstream out;
  out << std::setprecision(3) << p;

  std::string val = "zoom: " + out.str();
  _statusLabelZoom->setText(val.c_str());
}

void GUI::ImageWindow::slotShowProperty(Canvas::property_t p) {
  slotShowZoom(p.zoom_factor);
}



void GUI::ImageWindow::slotShowMarkers(Marker m) {
  _canvas->setMarker(m);
  std::string val = "";
  if (m.active)
    val = "marker: " + std::to_string((int)m.y) + " " + std::to_string((int)m.x);
  _statusLabelMarker->setText(val.c_str());
}
