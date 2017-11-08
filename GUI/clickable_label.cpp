#include "clickable_label.h"

GUI::ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent) {
      Q_UNUSED(f);
    
}

GUI::ClickableLabel::~ClickableLabel() {}


void GUI::ClickableLabel::mousePressEvent ( QMouseEvent * event ) 
{
  Q_UNUSED(event);
  emit clicked();
}
