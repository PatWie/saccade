#include "ascii_loader.h"
#include <QTime>
#include <glog/logging.h>


AsciiLoader::AsciiLoader(QLabel *dst) {
  symbols[0] = QString::fromUtf8("\u25D0");
  symbols[1] = QString::fromUtf8("\u25D3");
  symbols[2] = QString::fromUtf8("\u25D1");
  symbols[3] = QString::fromUtf8("\u25D2");

  _dst = dst;

  _timer = new QTimer(this);
  off = 0;
  connect(_timer, SIGNAL(timeout()), this, SLOT(showAnimation()));
}

AsciiLoader::~AsciiLoader() {}


void AsciiLoader::start() {
  off = 0;
  _timer->start(500);
}

void AsciiLoader::stop() {
  off = 0;
  _timer->stop();
  _dst->setText("");
}

void AsciiLoader::showAnimation() {
  off = (off + 1) % 4;
  _dst->setText(symbols[off]);
  
}