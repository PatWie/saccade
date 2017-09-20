#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H


#include <QDialog>

namespace Ui {
class AboutWindow;
}

namespace GUI {
class AboutWindow : public QDialog {
  Q_OBJECT

 public:
  explicit AboutWindow(QWidget *parent = 0);
  ~AboutWindow();

 private:
  Ui::AboutWindow *ui;
};
} // GUI

#endif