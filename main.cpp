#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QSurfaceFormat>

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "GUI/window.h"
#include "Utils/version.h"

void set_style(QPalette *p) {

  QColor white(255, 255, 255);
  QColor red(255, 0, 0);
  QColor black(0, 0, 0);
  p->setBrush(QPalette::WindowText, QColor(255, 0, 0));
  p->setColor(QPalette::Window, QColor(37, 37, 37));
  p->setColor(QPalette::WindowText, white);
  p->setColor(QPalette::Base, QColor(25, 25, 25));
  p->setColor(QPalette::AlternateBase, QColor(39, 40, 34));
  p->setColor(QPalette::ToolTipBase, white);
  p->setColor(QPalette::ToolTipText, white);
  p->setColor(QPalette::Text, white);
  p->setColor(QPalette::Button, QColor(39, 40, 34));
  p->setColor(QPalette::ButtonText, white);
  p->setColor(QPalette::BrightText, red);
  p->setColor(QPalette::Link, QColor(42, 130, 218));
  p->setColor(QPalette::Highlight, QColor(39, 40, 34, 255));
  p->setColor(QPalette::HighlightedText, QColor(251, 199, 99));
}

// call by ./saccade --logtostderr=1
int main(int argc, char *argv[]) {

  // FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, true);

  LOG(INFO) << Utils::GetVersionInfo();
  LOG(INFO) << Utils::GetBuildInfo();
  QApplication app(argc, argv);

  LOG(INFO) << "override style";
  QPalette p;
  set_style(&p);
  app.setStyle("Fusion");
  app.setPalette(p);

  GUI::Window window(&app);
  window.setWindowIcon(QIcon(":Icon/256x256/saccade.png"));
  window.setWindowTitle("Saccade");
  window.show();

  return app.exec();
}