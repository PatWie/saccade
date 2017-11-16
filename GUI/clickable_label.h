#ifndef CLICK_LABEL_H
#define CLICK_LABEL_H

#include <QLabel>
#include <QMouseEvent>

namespace GUI {

class ClickableLabel : public QLabel {
  Q_OBJECT

 public:
  explicit ClickableLabel(QWidget* parent = Q_NULLPTR);
  ~ClickableLabel();


 protected:
  void mousePressEvent ( QMouseEvent * event );
 signals:
  void clicked();
}
;

}

#endif