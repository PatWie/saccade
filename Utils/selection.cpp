#include "selection.h"

const QRect Utils::selection_t::rectangle() const{
  const int left_most = std::min(_start.x(), _stop.x());
  const int top_most = std::min(_start.y(), _stop.y());
  const int bottom_most = std::max(_start.y(), _stop.y());
  const int right_most = std::max(_start.x(), _stop.x());
  return QRect(QPoint(left_most, top_most), QPoint(right_most, bottom_most));
}

void Utils::selection_t::setStart(QPoint p){
  _start = p;
  _stop = p;
}

void Utils::selection_t::setStop(QPoint p){
  _stop = p;
}

void Utils::selection_t::setActive(bool p){
  _active = p;
}

void Utils::selection_t::toggleActive(){
  _active = !_active;
}

void Utils::selection_t::reset(){
  _start = QPoint(0, 0);
  _stop = QPoint(0, 0);
  _active = false;
}

QPoint Utils::selection_t::start() const{
  return _start;
}

QPoint Utils::selection_t::stop() const{
  return _stop;
}

bool Utils::selection_t::active() const{
  return _active;
}