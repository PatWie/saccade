#ifndef WINDOW_H
#define WINDOW_H

#include <vector>
#include <string>
#include <QtGui>
#include <QMdiArea>
#include <QApplication>
#include <QMainWindow>

#include "canvas.h"


namespace GUI {

class AboutWindow;
class ImageWindow;
class Slides;

class Window  : public QMainWindow {
  Q_OBJECT

 public:
  Window(QApplication* app);
  QSize sizeHint() const;

  QString _openPath;

 signals:
  void sigReceiveWindowGeometry(ImageWindow*);
  void sigCommunicateWindowGeometry(ImageWindow*);
  void sigReceiveCanvasChange(Canvas*);

 public slots:
  void slotDialogWindowAction();
  void slotNewWindowAction();
  void slotCommunicateCanvasChange(Canvas*);
  void slotCommunicateWindowGeometry(ImageWindow*);
  void slotFocusChanged(ImageWindow*);
  void slotReceiveArangeWindows();

  void slotImageWindowCloses(ImageWindow*);
  void slotQuitApp();

 private:
  Slides* _slides;

  QMdiArea* workspace;

  // menu
  QMenu* _windowMenu;
  QAction* _newWindowAct;
  QAction* _dialogWindowAct;



  std::vector<GUI::ImageWindow*> _windows;

  QAction* _closeAppAct;

  QApplication* _app;

};
}; // namespace GUI

#endif // WINDOW_H
