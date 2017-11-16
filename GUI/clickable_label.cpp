#include "clickable_label.h"

GUI::ClickableLabel::ClickableLabel(QWidget* parent)
  : QLabel(parent) {

}

GUI::ClickableLabel::~ClickableLabel() {}


void GUI::ClickableLabel::mousePressEvent ( QMouseEvent * event ) {
  Q_UNUSED(event);
  emit clicked();
}
