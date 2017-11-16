#ifndef ASCII_LOADER_H
#define ASCII_LOADER_H

#include <QString>
#include <QLabel>
#include <QTimer>
#include <QObject>

class AsciiLoader : QObject{
  Q_OBJECT

  QString symbols[4];
  QLabel* _dst;
  QTimer* _timer;
  int off;
 public:
  AsciiLoader(QLabel *dst);
  ~AsciiLoader();

  void start();
  void stop();

 public slots:
  void showAnimation();

};

#endif // ASCII_LOADER_H