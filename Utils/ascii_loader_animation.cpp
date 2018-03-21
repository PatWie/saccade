#include "ascii_loader_animation.h"
#include <QTime>
#include <glog/logging.h>


AsciiLoaderAnimation::AsciiLoaderAnimation(QLabel *dst) {
  symbols[0] = QString::fromUtf8("\u25D0");
  symbols[1] = QString::fromUtf8("\u25D3");
  symbols[2] = QString::fromUtf8("\u25D1");
  symbols[3] = QString::fromUtf8("\u25D2");

  _dst = dst;

  _timer = new QTimer(this);
  off = 0;
  connect(_timer, SIGNAL(timeout()), this, SLOT(showAnimation()));
}

AsciiLoaderAnimation::~AsciiLoaderAnimation() {}


void AsciiLoaderAnimation::start() {
  off = 0;
  _timer->start(500);
}

void AsciiLoaderAnimation::stop() {
  off = 0;
  _timer->stop();
  _dst->setText("");
}

void AsciiLoaderAnimation::play() {
  off = (off + 1) % 4;
  _dst->setText(symbols[off]);

}