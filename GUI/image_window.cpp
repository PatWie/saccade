#include <QtWidgets>
#include <QDebug>
#include <iomanip>
#include <sstream>

#include "window.h"
#include "image_window.h"
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
  _statusLabelMouse = new QLabel("Coordinates");
  statusBar()->addWidget(_statusLabelMouse, 1);
  _statusLabelPatch = new QLabel("Patch");
  statusBar()->addWidget(_statusLabelPatch, 1);
  _statusLabelMarker = new QLabel("Marker");
  statusBar()->addWidget(_statusLabelMarker, 1);
  _statusLabelZoom = new QLabel("Zoom");
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

  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_openImageAct);
  _fileMenu->addAction(_removeImageAct);

  _windowMenu = menuBar()->addMenu(tr("&Window"));
  _windowMenu->addAction(_newWindowAct);
  _windowMenu->addAction(_dialogWindowAct);
  _windowMenu->addAction(_closeWindowAct);

  _zoomInAct = new QAction(this);
  _zoomInAct->setShortcut(Qt::Key_Plus | Qt::CTRL);

  _zoomStdAct = new QAction(this);
  _zoomStdAct->setShortcut(Qt::Key_0 | Qt::CTRL);

  _zoomOutAct = new QAction(this);
  _zoomOutAct->setShortcut(Qt::Key_Minus | Qt::CTRL);

  connect(_zoomInAct, SIGNAL(triggered()),
          _canvas, SLOT(slotZoomInAction()));
  connect(_zoomStdAct, SIGNAL(triggered()),
          this, SLOT(slotZoomStdAction()));
  connect(_zoomOutAct, SIGNAL(triggered()),
          _canvas, SLOT(slotZoomOutAction()));

  connect(this, SIGNAL(sigSetZoomAction(double)),
          _canvas, SLOT(slotSetZoomAction(double)));

  this->addAction(_zoomInAct);
  this->addAction(_zoomStdAct);
  this->addAction(_zoomOutAct);

  setAcceptDrops(true);

}

void GUI::ImageWindow::slotZoomStdAction() {
  emit sigSetZoomAction(1.0);
}

void GUI::ImageWindow::dropEvent(QDropEvent *ev) {
  QList<QUrl> urls = ev->mimeData()->urls();
  foreach (QUrl url, urls) {
    qDebug() << url.toLocalFile();
    loadImage(url.toLocalFile().toStdString());
  }
}

void GUI::ImageWindow::dragEnterEvent(QDragEnterEvent *ev) {
  ev->accept();
}

void GUI::ImageWindow::loadImage(std::string fn) {
  Layer *layer = new Layer();
  layer->loadImage(fn);
  _canvas->addLayer(layer);
}

void GUI::ImageWindow::slotOpenImageAction() {
  qDebug() << "GUI::Window::slotOpenImageAction()";

  QStringList filenames = QFileDialog::getOpenFileNames(this,
                          tr("Open Image"), _parentWindow->_openPath, tr("Image Files (*.png *.jpg *.bmp)"));

  if ( !filenames.isEmpty() ) {
    for (int i = 0; i < filenames.count(); i++)
      loadImage(filenames.at(i).toStdString());
    _parentWindow->_openPath = QFileInfo(filenames.at(0)).absolutePath();
  }
  _canvas->slotUpdateCanvas();
}

void GUI::ImageWindow::slotRemoveImageAction() {
  qDebug() << "GUI::ImageWindow::slotRemoveImageAction";
  emit sigRemoveCurrentLayer();
}

QSize GUI::ImageWindow::sizeHint() const {
  return QSize(1024, 1024);
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
  stream << "Mouse: " << p.y() << ", " << p.x();

  std::string str = stream.str();
  _statusLabelMouse->setText(str.c_str());

  if (_canvas->slides() != nullptr) {
    std::stringstream stream;
    QPoint top_left = QPoint(0, 0);
    QPoint bottom_right = QPoint(width() - 1, height() - 1);

    top_left = _canvas->screenToBuf(top_left);
    bottom_right = _canvas->screenToBuf(bottom_right);

    stream << "Patch "
           << "(" << top_left.y() << " " << top_left.x() << ") "
           << "(" << bottom_right.y() << " " << bottom_right.x() << ") "
           << "";

    _statusLabelPatch->setText(stream.str().c_str());

  }
}

void GUI::ImageWindow::slotShowZoom(double p) {
  std::ostringstream out;
  out << std::setprecision(3) << p;

  std::string val = "Zoom " + out.str();
  _statusLabelZoom->setText(val.c_str());
}

void GUI::ImageWindow::slotShowProperty(Canvas::property_t p) {
  slotShowZoom(p.zoom_factor);
}



void GUI::ImageWindow::slotShowMarkers(Marker m) {
  _canvas->setMarker(m);

  std::string val = "Marker " + std::to_string((int)m.y) + " " + std::to_string((int)m.x);
  _statusLabelMarker->setText(val.c_str());
}
