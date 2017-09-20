#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QSurfaceFormat>
#include "GUI/window.h"

// based on https://sourceforge.net/p/shiver/idisplay

void set_style(QPalette *p){

  QColor white(255, 255, 255);
  QColor red(255, 0, 0);
  QColor black(0, 0, 0);
  p->setBrush(QPalette::WindowText, QColor(255, 0, 0));

  p->setColor(QPalette::Window, QColor(53, 53, 53));
  p->setColor(QPalette::WindowText, white);
  p->setColor(QPalette::Base, QColor(25, 25, 25));
  p->setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  p->setColor(QPalette::ToolTipBase, white);
  p->setColor(QPalette::ToolTipText, white);
  p->setColor(QPalette::Text, white);
  p->setColor(QPalette::Button, QColor(53, 53, 53));
  p->setColor(QPalette::ButtonText, white);
  p->setColor(QPalette::BrightText, red);
  p->setColor(QPalette::Link, QColor(42, 130, 218));
  p->setColor(QPalette::Highlight, QColor(42, 130, 218));
  p->setColor(QPalette::HighlightedText, black);
}

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);

  QPalette p;
  set_style(&p);
  app.setStyle("Fusion");
  app.setPalette(p);

  GUI::Window window(&app);

  window.setWindowIcon(QIcon(":Icon/256x256/eagleeye.png"));
  window.setWindowTitle("EagleEye");
  window.show();
  return app.exec();
}