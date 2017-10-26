#include <iostream>
#include <algorithm>

#include <glog/logging.h>

#include <QtWidgets>
#include <QDebug>

#include "window.h"
#include "image_window.h"
#include "about_window.h"
#include "histogram.h"
#include "canvas.h"
#include "slides.h"
#include "Utils/histogram_data.h"

GUI::Window::Window(QApplication* app) : _app(app) {
  // workspace = new QMdiArea(this);
  // workspace->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  // workspace->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  // setCentralWidget(workspace);

  _openPath = QDir::currentPath();

  // menu
  _newWindowAct = new QAction(tr("&New"), this);
  _newWindowAct->setShortcut(tr("Ctrl+N"));
  _newWindowAct->setStatusTip(tr("Create a new Window"));

  _dialogWindowAct = new QAction(tr("&About"), this);
  _dialogWindowAct->setShortcut(tr("F1"));
  _dialogWindowAct->setStatusTip(tr("Information and Version"));

  _closeAppAct = new QAction(tr("E&xit"), this);
  _closeAppAct->setShortcut(tr("Ctrl+Q"));
  _closeAppAct->setStatusTip(tr("Close the app"));

  connect(_newWindowAct, &QAction::triggered, this, &GUI::Window::slotNewWindowAction);
  connect(_dialogWindowAct, &QAction::triggered, this, &GUI::Window::slotDialogWindowAction);
  connect(_closeAppAct, &QAction::triggered, this, &GUI::Window::close);

  _windowMenu = menuBar()->addMenu(tr("&Window"));
  _windowMenu->addAction(_newWindowAct);
  _windowMenu->addAction(_dialogWindowAct);
  _windowMenu->addAction(_closeAppAct);

  // fire up
  slotNewWindowAction();
}

QSize GUI::Window::sizeHint() const {
  return QSize(64, 64);
}

void GUI::Window::slotFocusChanged(ImageWindow* obj) {
  DLOG(INFO) << "GUI::Window::slotFocusChanged()";
  DLOG(INFO) << obj->metaObject()->className();

}

void GUI::Window::slotImageWindowCloses(ImageWindow* sender){
  _windows.erase(std::remove(_windows.begin(), _windows.end(), sender), _windows.end());
  if(_windows.size() == 0){
    QCoreApplication::quit();
  }
}
void GUI::Window::slotQuitApp(){
  QCoreApplication::quit();
}


void GUI::Window::slotNewWindowAction() {
  DLOG(INFO) << "GUI::Window::slotNewWindowAction()";

  GUI::ImageWindow* tmpWindow = new GUI::ImageWindow(this, this);
  tmpWindow->setMinimumSize(200, 200);
  tmpWindow->show();

  // incoming messages

  connect(tmpWindow, &GUI::ImageWindow::sigFocusChange,
          this, &GUI::Window::slotFocusChanged);

  connect(tmpWindow, &GUI::ImageWindow::sigCommunicateWindowGeometry,
          this, &GUI::Window::slotCommunicateWindowGeometry);

  connect(tmpWindow, &GUI::ImageWindow::sigImageWindowCloses,
          this, &GUI::Window::slotImageWindowCloses);

  // outgoing messages
  connect(this, &GUI::Window::sigReceiveWindowGeometry,
          tmpWindow, &GUI::ImageWindow::slotReceiveWindowGeometry);

  _windows.push_back(tmpWindow);

}
void GUI::Window::slotDialogWindowAction() {
  DLOG(INFO) << "GUI::Window::slotDialogWindowAction()";
  AboutWindow* dialog = new AboutWindow(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  dialog->setGeometry(
    QStyle::alignedRect(
      Qt::LeftToRight,
      Qt::AlignCenter,
      dialog->size(),
      _app->desktop()->availableGeometry()
    )
  );
  dialog->show();
}

void GUI::Window::slotCommunicateCanvasChange(Canvas* sender) {
  emit sigReceiveCanvasChange(sender);
}

void GUI::Window::slotCommunicateWindowGeometry(ImageWindow* sender) {
  emit sigReceiveWindowGeometry(sender);
}

void GUI::Window::slotCommunicatePrevLayer(){
  emit sigReceivePrevLayer();
}
void GUI::Window::slotCommunicateNextLayer(){
  emit sigReceiveNextLayer();
}

void GUI::Window::slotReceiveArangeWindows() {
  std::vector<GUI::ImageWindow*> sorted_windows = _windows;
  std::sort(sorted_windows.begin(), sorted_windows.end(),
  [](const GUI::ImageWindow * lhs, const GUI::ImageWindow * rhs) {
    auto lhs_pos = lhs->mapToGlobal(lhs->pos());
    auto rhs_pos = rhs->mapToGlobal(rhs->pos());

    float lhs_dist = lhs_pos.x() * lhs_pos.x() + lhs_pos.y() * lhs_pos.y();
    float rhs_dist = rhs_pos.x() * rhs_pos.x() + rhs_pos.y() * rhs_pos.y();
    return lhs_dist < rhs_dist;
  });

  QRect rec = QApplication::desktop()->availableGeometry(sorted_windows[0]);
  // const int height = rec.height();
  const int width = rec.width();
  const int padding = 4;

  int cur_x = sorted_windows[0]->pos().x();
  int cur_y = sorted_windows[0]->pos().y();
  const int start_x = cur_x;
  int cur_line_height = sorted_windows[0]->height();

  for (auto && wnd : sorted_windows) {
    if (cur_x + padding + wnd->width() > width) {
      cur_x = start_x;
      cur_y += cur_line_height;
      cur_line_height = 0;
    }
    wnd->move(cur_x, cur_y);
    cur_x += wnd->width() + padding;
    // cur_y += wnd->height();
    cur_line_height = std::max(cur_line_height, wnd->height());
  }
}