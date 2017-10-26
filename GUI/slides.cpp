#include <iostream>

#include <glog/logging.h>

#include "slides.h"
#include "layer.h"


GUI::Slides::Slides() {
  _id = -1;
}

const GUI::Layer* GUI::Slides::current() const {
  return _slides[_id];
}

GUI::Layer* GUI::Slides::current() {
  if (_slides.size() > 0)
    return _slides[_id];
  return nullptr;
}

const GUI::Layer* GUI::Slides::operator[](int i) const {
  return _slides[i];
}

GUI::Layer* GUI::Slides::operator[](int i) {
  return _slides[i];
}

unsigned int GUI::Slides::num() const {
  return _slides.size();
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

void GUI::Slides::remove() {
  if (_slides.size() > 0) {
    current()->clear();
    DLOG(INFO) << "_id " << _id;
    int tid = _id;
    _id = _id - 1;
    _slides.erase(_slides.begin() + tid);
  }
  // case only one left --> jump to first
  if (_slides.size() == 1) {
    _id = 0;
  }
  // case no layer left --> disable
  if (_slides.size() == 0) {
    _id = -1;
  }
  // case: there are at least two layers --> automatically jumps to next

}

void GUI::Slides::forward() {
  if (_slides.size() > 0) {
    _id++;
    if (_id >= (int) _slides.size())
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
  return available() ? current()->width() : 0 ;
}

size_t GUI::Slides::height() const {
  if (_id == -1)
    return 0;
  return available() ? current()->height() : 0 ;
}

bool GUI::Slides::available() const {
  if (_id == -1)
    return false;
  return current()->available();
}

std::string GUI::Slides::path() const {
  if (_id == -1)
    return "Untitled";
  return current()->path();
}