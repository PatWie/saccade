#ifndef CLICK_LABEL_H
#define CLICK_LABEL_H

#include <QLabel>
#include <QMouseEvent>

namespace GUI {

class ClickableLabel : public QLabel {
  Q_OBJECT

 public:
  explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
  ~ClickableLabel();


 protected:
  void mousePressEvent ( QMouseEvent * event );
 signals:
  void clicked();
}
;

}

#endif