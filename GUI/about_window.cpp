#include "about_window.h"
#include "ui_about_window.h"

#include "../Utils/version.h"

GUI::AboutWindow::AboutWindow(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AboutWindow) {
  ui->setupUi(this);

  QPixmap icon_png(":Icon/256x256/eagleeye.png");
  icon_png = icon_png.scaled(QSize(128, 128), Qt::IgnoreAspectRatio);
  ui->lbl_icon->setPixmap(icon_png);

  QString labelText1 = "<p><span style=\" font-size:14pt; font-weight:600;\">"+  QString::fromStdString(Utils::GetVersionInfo()) + "</span></p>";
  ui->label->setText(labelText1);

  QString labelText = "";
  labelText.append("<p>" + QString::fromStdString(Utils::GetBuildInfo()) + "</p>");
  labelText.append("<p><a href=\"https://github.com/PatWie/eagle_eye\"><span style=\" text-decoration: underline; color:#ffffff;\">https://github.com/PatWie/eagle_eye</span></a></p>");
  labelText.append("<p><br/></p>");
  labelText.append("<p>Patrick Wieschollek<br/>mail@patwie.com</p>");

  ui->label_2->setText(labelText);
}

GUI::AboutWindow::~AboutWindow() {
  delete ui;
}


