#ifndef ASCII_LOADER_H
#define ASCII_LOADER_H

#include <QString>
#include <QLabel>
#include <QTimer>
#include <QObject>

class AsciiLoaderAnimation : QObject{
  Q_OBJECT

  QString symbols[4];
  QLabel* _dst;
  QTimer* _timer;
  int off;
 public:
  AsciiLoaderAnimation(QLabel *dst);
  ~AsciiLoaderAnimation();

  void start();
  void stop();

 public slots:
  void play();

};

#endif // ASCII_LOADER_H