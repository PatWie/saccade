#include <iomanip>
#include <sstream>

#include <glog/logging.h>

#include <QtWidgets>

#include "window.h"
#include "clickable_label.h"
#include "histogram.h"
#include "image_window.h"
#include "../Utils/image_data.h"
#include "marker.h"
#include "layer.h"
#include "slides.h"

GUI::ImageWindow::ImageWindow(QWidget* parent, GUI::Window* parentWindow)
  : QMainWindow(parent), _parentWindow(parentWindow) {

  setWindowTitle("Saccade - empty");

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

  connect(this, &GUI::ImageWindow::sigKeyReleaseEvent,
          _canvas, &Canvas::slotKeyReleaseEvent);

  connect(this, &GUI::ImageWindow::sigCommunicatePrevLayer,
          parentWindow, &GUI::Window::slotCommunicatePrevLayer);

  connect(this, &GUI::ImageWindow::sigCommunicateNextLayer,
          parentWindow, &GUI::Window::slotCommunicateNextLayer);

  connect(parentWindow, &GUI::Window::sigReceivePrevLayer,
          _canvas, &GUI::Canvas::slotPrevLayer);
  connect(parentWindow, &GUI::Window::sigReceiveNextLayer,
          _canvas, &GUI::Canvas::slotNextLayer);

  _canvas->slotRepaint();

  // toolbar
  _toolbar_histogram = new Histogram(this);
  _toolbar = new QToolBar(tr("histogram"));
  _toolbar->setMovable( false );
  _toolbar->addWidget( _toolbar_histogram );
  addToolBar(Qt::TopToolBarArea, _toolbar);

  // statusbar
  // ==========================================================

  _statusLabelCursorPos = new QLabel();
  _statusLabelCursorColor = new QLabel();
  statusBar()->addWidget(_statusLabelCursorPos, 1);
  statusBar()->addWidget(_statusLabelCursorColor, 1);
  _statusLabelMarkerPos = new ClickableLabel();
  _statusLabelMarkerColor = new ClickableLabel();
  statusBar()->addWidget(_statusLabelMarkerPos, 1);
  statusBar()->addWidget(_statusLabelMarkerColor, 1);
  _statusLabelZoom = new QLabel("zoom: 1");
  statusBar()->addWidget(_statusLabelZoom, 1);
  _statusLabelLoader = new QLabel();
  _ascii_loader_animation = new AsciiLoaderAnimation(_statusLabelLoader);
  statusBar()->addWidget(_statusLabelLoader, 1);
  statusBar()->setSizeGripEnabled ( false );

  connect(_statusLabelMarkerPos, &ClickableLabel::clicked,
          this, &GUI::ImageWindow::slotClickedMarkerLabelPos);
  connect(_statusLabelMarkerColor, &ClickableLabel::clicked,
          this, &GUI::ImageWindow::slotClickedMarkerLabelColor);


  // slides
  // ==========================================================

  connect( this, &GUI::ImageWindow::sigPrevLayer,
           _canvas, &GUI::Canvas::slotPrevLayer);
  connect( this, &GUI::ImageWindow::sigNextLayer,
           _canvas, &GUI::Canvas::slotNextLayer);
  connect( this, &GUI::ImageWindow::sigRemoveCurrentLayer,
           _canvas, &GUI::Canvas::slotRemoveCurrentLayer);
  connect( this, &GUI::ImageWindow::sigShiftCanvas,
           _canvas, &GUI::Canvas::slotShiftCanvas);


  // menubar
  // ==========================================================
  _openImageAct = new QAction(tr("&Open"), this );
  _openImageAct->setShortcut(tr("Ctrl+O"));
  _openImageAct->setStatusTip(tr("Open an existing image"));
  connect(_openImageAct, &QAction::triggered, this, &GUI::ImageWindow::slotOpenImage);

  _saveImageAct = new QAction(tr("&Save"), this );
  _saveImageAct->setShortcut(tr("Ctrl+S"));
  _saveImageAct->setStatusTip(tr("Save current image"));
  connect(_saveImageAct, &QAction::triggered, this, &GUI::ImageWindow::slotSaveImage);

  _saveCropAct = new QAction(tr("&Save"), this );
  _saveCropAct->setShortcut(tr("Ctrl+X"));
  _saveCropAct->setStatusTip(tr("Save current crop"));
  connect(_saveCropAct, &QAction::triggered, this, &GUI::ImageWindow::slotSaveCrop);

  _removeImageAct = new QAction(tr("&Remove"), this );
  _removeImageAct->setShortcut(tr("Del"));
  _removeImageAct->setStatusTip(tr("Remove the current image"));
  connect(_removeImageAct, &QAction::triggered, _canvas, &GUI::Canvas::slotRemoveCurrentLayer);

  _emptyCanvasAct = new QAction(tr("Remove all images in current canvas"), this );
  _emptyCanvasAct->setShortcut(tr("Ctrl+Del"));
  _emptyCanvasAct->setStatusTip(tr("Remove the all images in current canvas"));
  connect(_emptyCanvasAct, &QAction::triggered, _canvas, &GUI::Canvas::slotRemoveAllLayers);

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

  _refreshAct = new QAction(tr("&Refresh Canvas"), this );
  _refreshAct->setShortcut(tr("F5"));
  _refreshAct->setStatusTip(tr("Repaint entire window and canvas"));
  connect(_refreshAct, &QAction::triggered, this, &GUI::ImageWindow::slotRepaint);

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
  connect(_resetHistogramAct,  &QAction::triggered,
  this, [this] () { _toolbar_histogram->slotResetRange(HistogramRefreshTarget::CURRENT); });

  _resetHistogramEntireCanvasAct = new QAction(tr("&Reset the histogram for all layers"), this );
  _resetHistogramEntireCanvasAct->setShortcut(tr("Ctrl+Shift+H"));
  _resetHistogramEntireCanvasAct->setStatusTip(tr("Reset the histogram range for all layers"));
  connect(_resetHistogramEntireCanvasAct,  &QAction::triggered,
  this, [this] () { _toolbar_histogram->slotResetRange(HistogramRefreshTarget::ENTIRE_CANVAS); });

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
  connect(_closeAppAct, &QAction::triggered, _parentWindow, &GUI::Window::slotQuitApp);


  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_openImageAct);
  _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_saveCropAct);
  _fileMenu->addAction(_removeImageAct);
  _fileMenu->addAction(_emptyCanvasAct);

  _windowMenu = menuBar()->addMenu(tr("&Window"));
  _windowMenu->addAction(_newWindowAct);
  _windowMenu->addAction(_propagateWindowGeometryAct);
  _windowMenu->addAction(_arangeWindowsAct);
  _windowMenu->addAction(_refreshAct);
  _windowMenu->addAction(_fitImageAct);
  _windowMenu->addAction(_centerImageAct);
  _windowMenu->addAction(_dialogWindowAct);
  _windowMenu->addAction(_closeWindowAct);
  _windowMenu->addAction(_closeAppAct);

  _imageMenu = menuBar()->addMenu(tr("&Image"));
  _imageMenu->addAction(_resetHistogramAct);
  _imageMenu->addAction(_resetHistogramEntireCanvasAct);

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


void GUI::ImageWindow::slotClickedMarkerLabelPos() {
  QClipboard *p_Clipboard = QApplication::clipboard();
  if (_canvas->marker().active) {
    p_Clipboard->setText(QString::fromStdString(_canvas->marker().textLocation()));
  }
}

void GUI::ImageWindow::slotClickedMarkerLabelColor() {
  QClipboard *p_Clipboard = QApplication::clipboard();
  Marker m = _canvas->marker();
  if (m.active) {
    p_Clipboard->setText(QString::fromStdString(_canvas->slides()->current()->img()->colorString(m.y, m.x, false)));
  }
}



bool GUI::ImageWindow::event(QEvent *e) {
  if (e->type() == QEvent::WindowActivate) {
    emit sigFocusChange(this);
  }
  return QWidget::event(e);
}

void GUI::ImageWindow::slotRefreshBuffer(HistogramRefreshTarget target) {
  DLOG(INFO) << "GUI::Window::slotRefreshBuffer()";
  DLOG(INFO) << _toolbar_histogram->data()->range()->min << " "
             <<  _toolbar_histogram->data()->range()->max;

  Layer *layer = _canvas->layer();
  if (layer != nullptr) {
    const double bin_width = _toolbar_histogram->data()->image()->max() /
                             static_cast<double>(256);
    DLOG(INFO) << "bin_width = " << bin_width;
    if (target == HistogramRefreshTarget::CURRENT) {
      layer->slotRefresh(_toolbar_histogram->data()->range()->min * bin_width,
                         _toolbar_histogram->data()->range()->max * bin_width);
    } else {
      DLOG(INFO) << "has " << _canvas->slides()->num() << " layers";
      const int new_min = _toolbar_histogram->data()->range()->min * bin_width;
      const int new_max = _toolbar_histogram->data()->range()->max * bin_width;
      for (unsigned int n = 0; n < _canvas->slides()->num(); ++n) {
        Layer *layer = _canvas->layer(n);
        layer->histogram()->range()->min = new_min;
        layer->histogram()->range()->max = new_max;
        DLOG(INFO) << "update layer " << n;
        DLOG(INFO) << "update layer " << layer->available();
        layer->slotRefresh(new_min, new_max);
      }
    }
  }
}


void GUI::ImageWindow::dropEvent(QDropEvent *ev) {
  QList<QUrl> urls = ev->mimeData()->urls();
  foreach (QUrl url, urls) {
    if (Utils::ImageData::knownImageFormat(url.toLocalFile().toStdString())) {
      DLOG(INFO) << "dropped " << url.toLocalFile().toStdString();
      loadImage(url.toLocalFile().toStdString());
    }
  }
}

void GUI::ImageWindow::dragEnterEvent(QDragEnterEvent *ev) {
  ev->accept();
}

void GUI::ImageWindow::loadImage(std::string fn) {

  _ascii_loader_animation->start();
  Layer *layer = new Layer();
  layer->loadImage(fn);

  connect(layer, &Layer::sigHistogramFinished,
          this, &GUI::ImageWindow::slotRepaint);

  connect(layer, &Layer::sigHistogramFinished,
          this, &GUI::ImageWindow::slotLoadingFinished);

  _canvas->addLayer(layer);
}

void GUI::ImageWindow::slotLoadingFinished(){
  _ascii_loader_animation->stop();
}

void GUI::ImageWindow::slotSaveImage() {
  DLOG(INFO) << "GUI::Window::slotSaveImage()";

  if (_canvas->layer() == nullptr) {

  } else {
    // there is a layer
    const GUI::Layer *current = _canvas->slides()->current();
    if (current != nullptr) {
      current->buffer()->write(current->path() + "_edit.png");
    }
  }

}

void GUI::ImageWindow::slotSaveCrop() {
  DLOG(INFO) << "GUI::Window::slotSaveCrop()";

  if (!_canvas->crop().active())
    return;

  if (_canvas->layer() == nullptr) {

  } else {
    const QRect c = _canvas->crop().rectangle();
    // there is a layer and we have an active crop
    const GUI::Layer *current = _canvas->slides()->current();
    if (current != nullptr) {
      std::string fn = current->path() + "-crop-"
                       + "t" + std::to_string(c.top()) + "-"
                       + "l" + std::to_string(c.left()) + "-"
                       + "b" + std::to_string(c.bottom()) + "-"
                       + "r" + std::to_string(c.right())
                       + ".png";
      current->buffer()->write(fn, c.top(), c.left(), c.bottom(), c.right());
    }
  }

}




void GUI::ImageWindow::slotOpenImage() {
  DLOG(INFO) << "GUI::Window::slotOpenImage()";

  QStringList filenames = QFileDialog::getOpenFileNames(this,
                          tr("Open Image"), _parentWindow->_openPath,
                          tr("Image Files (*.png *.jpg *.jpeg *.bmp *.ppm *.tif *.CR2 *.JPG *.JPEG, *.JPE, *.flo)"));

  if ( !filenames.isEmpty() ) {
    for (int i = 0; i < filenames.count(); i++)
      loadImage(filenames.at(i).toStdString());
  }
}


QSize GUI::ImageWindow::sizeHint() const {
  return QSize(512, 512);
}

void GUI::ImageWindow::keyReleaseEvent (QKeyEvent *event){
  emit sigKeyReleaseEvent(event);
  // DLOG(INFO) << "keyReleaseEvent";
  // Qt::KeyboardModifiers keymod = QGuiApplication::keyboardModifiers();
  // const bool pressedShift = keymod == Qt::ShiftModifier;
  // const bool pressedCtrl = keymod == Qt::ControlModifier;

  // DLOG(INFO) << event->text().toStdString();
  // DLOG(INFO) << event->modifiers();
  // if(!pressedShift)
  //   _selection.setActive(false);
  // if(!pressedCtrl)
  //   _crop.setActive(false);
}

void GUI::ImageWindow::slotReceiveCanvasChange(Canvas* sender) {
  if ( _canvas != sender ) {
    // update canvas
    _canvas->setAxis(sender->axis());
    _canvas->setFocusPixel(sender->focusPixel());
    _canvas->setMarker(sender->marker());
    _canvas->setCrop(sender->crop());
    _canvas->setSelection(sender->selection());

    slotRepaint();

  }
  slotRepaintStatusbar();
  slotRepaintSliders();
}

void GUI::ImageWindow::slotReceiveLayerChange() {
  DLOG(INFO) << "GUI::ImageWindow::slotReceiveLayerChange()";
  slotRepaintHistogram();
  slotRepaintStatusbar();
  slotRepaintTitle();
  slotRepaintSliders();
}


void GUI::ImageWindow::keyPressEvent( QKeyEvent * event ) {

  Qt::KeyboardModifiers keymod = QGuiApplication::keyboardModifiers();

  const bool pressedShift = keymod == Qt::ShiftModifier;
  const bool pressedCtrl = keymod == Qt::ControlModifier;

  if (pressedShift) {
    // shift image position
    switch (event->key()) {
    case Qt::Key_Left:
      emit sigShiftCanvas(0, 1, true); break;
    case Qt::Key_Down:
      emit sigShiftCanvas(1, 0, true); break;
    case Qt::Key_Right:
      emit sigShiftCanvas(0, -1, true); break;
    case Qt::Key_Up:
      emit sigShiftCanvas(-1, 0, true); break;
      break;
    default:
      QMainWindow::keyPressEvent(event);
    }
  } else if (pressedCtrl) {
    // switch layer in all windows
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Down:
      emit sigCommunicatePrevLayer(); break;
    case Qt::Key_Right:
    case Qt::Key_Up:
      emit sigCommunicateNextLayer(); break;
    default:
      QMainWindow::keyPressEvent(event);
    }
  } else {
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Down:
      emit sigPrevLayer(); break;
    case Qt::Key_Right:
    case Qt::Key_Up:
      emit sigNextLayer(); break;
    default:
      QMainWindow::keyPressEvent(event);
    }
  }

}

void GUI::ImageWindow::slotVertSliderMoved(int value) {
  Canvas::axis_t axis = _canvas->axis();
  axis.y = value;
  _canvas->slotReceiveProperty(axis);
}

void GUI::ImageWindow::slotHorSliderMoved(int value) {
  Canvas::axis_t axis = _canvas->axis();
  axis.x = -value;
  _canvas->slotReceiveProperty(axis);
}

void GUI::ImageWindow::slotRepaint() {
  if (_canvas->layer() == nullptr) {
    _canvas->update();
  } else {
    const GUI::Layer *current = _canvas->slides()->current();
    if (current != nullptr) {
      _canvas->slotRepaint();
    }
  }
}


void GUI::ImageWindow::slotRepaintTitle() {
  if (_canvas->layer() == nullptr)
    setWindowTitle("Saccade - empty");
  else {
    const GUI::Layer *current = _canvas->slides()->current();
    if (current != nullptr) {
      // update title
      setWindowTitle(("Saccade - " + current->path()).c_str());
    }
  }
}

void GUI::ImageWindow::slotRepaintStatusbar() {
  if (_canvas->layer() == nullptr) {
    _statusLabelCursorPos->setText("");
    _statusLabelCursorColor->setText("");
    // _statusLabelPatch->setText("");
    _statusLabelMarkerPos->setText("");
    _statusLabelMarkerColor->setText("");
    _statusLabelZoom->setText("");
    return;
  }

  const GUI::Layer *current = _canvas->slides()->current();
  if (current != nullptr) {

    // update pixel
    std::stringstream pixelPosText;
    const QPoint p = _canvas->focusPixel();
    pixelPosText << "(" << p.y() << ", " << p.x() << ")";
    _statusLabelCursorPos->setText(pixelPosText.str().c_str());

    std::string pixelColorText = "" + _canvas->slides()->current()->img()->colorString(p.y(), p.x());
    _statusLabelCursorColor->setText(pixelColorText.c_str());

    // update marker
    Marker m = _canvas->marker();
    std::string markerPosText = "";
    std::string markerColorText = "";
    if (m.active) {
      markerPosText = "marker: (" + m.textLocation() + ")";
      markerColorText = _canvas->slides()->current()->img()->colorString(m.y, m.x);
    }
    _statusLabelMarkerPos->setText(markerPosText.c_str());
    _statusLabelMarkerColor->setText(markerColorText.c_str());

    // update zoom
    std::ostringstream zoomText;
    zoomText << "zoom: " << std::setprecision(3) << _canvas->axis().pixel_size;
    _statusLabelZoom->setText(zoomText.str().c_str());

  }
}

void GUI::ImageWindow::slotRepaintSliders() {
  if (_canvas->layer() == nullptr)
    return;
  const double imgWidth = _canvas->layer()->width();
  const double imgHeight = _canvas->layer()->height();

  const double winWidth = _canvas->width();
  const double winheight = _canvas->height();

  GUI::Canvas::axis_t prop = _canvas->axis();

  if ( imgWidth * prop.pixel_size < winWidth ) {
    _horSlider->setRange(0, 0);
    _horSlider->setValue(0);
  } else {
    const int radius = 0.5 * (imgWidth - winWidth / prop.pixel_size);
    _horSlider->setRange(-radius - 1, radius + 1);
    _horSlider->setValue(-prop.x);
  }
  if ( imgHeight * prop.pixel_size < winheight ) {
    _vertSlider->setRange(0, 0);
    _vertSlider->setValue(0);
  } else {
    const int radius = 0.5 * (imgHeight - winheight / prop.pixel_size);
    _vertSlider->setRange(-radius - 1, radius + 1);
    _vertSlider->setValue(prop.y);
  }
}

void GUI::ImageWindow::slotRepaintHistogram() {
  if (_canvas->layer() == nullptr) {
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
  DLOG(INFO) << "GUI::ImageWindow::slotCommunicateWindowGeometry";
  emit sigCommunicateWindowGeometry(this);
}

void GUI::ImageWindow::slotReceiveWindowGeometry(ImageWindow* sender) {
  DLOG(INFO) << "GUI::ImageWindow::slotReceiveWindowGeometry";
  if ( this != sender ) {
    resize(sender->width(), sender->height());
  }
}

void GUI::ImageWindow::closeEvent(QCloseEvent * event) {
  DLOG(INFO) << "GUI::ImageWindow::closeEvent";

  event->ignore();
  emit sigImageWindowCloses(this);
  event->accept();
};