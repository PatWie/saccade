#include "about_window.h"
#include "ui_about_window.h"

GUI::AboutWindow::AboutWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);
}

GUI::AboutWindow::~AboutWindow()
{
    delete ui;
}
