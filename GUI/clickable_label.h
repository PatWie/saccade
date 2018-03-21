#ifndef CLICK_LABEL_H
#define CLICK_LABEL_H

#include <QLabel>
#include <QMouseEvent>

namespace GUI {

/**
 * @brief QtLabels with click event handler
 * @details Default QtLabels do not handle the click event. This is a work-around.
 *
 * @param parent [description]
 * @return [description]
 */
class ClickableLabel : public QLabel {
  Q_OBJECT

 public:
  explicit ClickableLabel(QWidget* parent = Q_NULLPTR);
  ~ClickableLabel();

 protected:
  void mousePressEvent (QMouseEvent * event);

 signals:
  void clicked();
}
;

}

#endif