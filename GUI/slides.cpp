#include "slides.h"
#include "layer.h"


GUI::Slides::Slides() {
  _id = -1;
}

const GUI::Layer* GUI::Slides::current() const {
  return _slides[_id];
}

GUI::Layer* GUI::Slides::current() {
  return _slides[_id];
}

const GUI::Layer* GUI::Slides::operator[](int i) const {
  return _slides[i];
}

GUI::Layer* GUI::Slides::operator[](int i) {
  return _slides[i];
}

void GUI::Slides::add(Layer* l) {
  _slides.push_back(l);
  if (_slides.size() == 1)
    _id = 0;
}

void GUI::Slides::backward() {
  if (_slides.size() > 0) {
    _id--;
    if (_id < 0)
      _id = 0;
  }

}
void GUI::Slides::forward() {
  if (_slides.size() > 0) {
    _id++;
    if (_id >= _slides.size())
      _id = _slides.size() - 1;
  }

}

void GUI::Slides::draw(Utils::GlManager *gl,
                       uint top, uint left,
                       uint bottom, uint right,
                       double zoom) {
  current()->draw(gl, top, left, bottom, right, zoom);
}

size_t GUI::Slides::width() const {
  if (_id == -1)
    return 0;
  return hasImage() ? current()->width() : 0 ;
}

size_t GUI::Slides::height() const {
  if (_id == -1)
    return 0;
  return hasImage() ? current()->height() : 0 ;
}

bool GUI::Slides::hasImage() const {
  if (_id == -1)
    return false;
  return current()->hasImage();
}

std::string GUI::Slides::path() const {
  if (_id == -1)
    return "Untitled";
  return current()->path();
}