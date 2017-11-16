#ifndef SELECTION_H
#define SELECTION_H

#include <QRect>

namespace Utils {
class selection_t {
  QPoint _start;
  QPoint _stop;
  bool _active;

 public:
  const QRect area() const;

  QPoint start() const;
  QPoint stop() const;
  bool active() const;

  void setStart(QPoint p);
  void setStop(QPoint p);
  void setActive(bool a=true);
  void toggleActive();

  void reset();
};
}; // namespace Utils

#endif // SELECTION_H