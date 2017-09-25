#include "about_window.h"
#include "ui_about_window.h"

GUI::AboutWindow::AboutWindow(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AboutWindow) {
  ui->setupUi(this);

  QPixmap icon_png(":Icon/256x256/eagleeye.png");
  icon_png = icon_png.scaled(QSize(128, 128), Qt::IgnoreAspectRatio);
  ui->lbl_icon->setPixmap(icon_png);
}

GUI::AboutWindow::~AboutWindow() {
  delete ui;
}
