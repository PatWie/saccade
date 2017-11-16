#include "marker.h"

GUI::Marker::Marker()
: active(false){

}

std::string GUI::Marker::textLocation() const{
  std::string markerText = "";
  if (active) {
    markerText = std::to_string((int)y) + ", " + std::to_string((int)x);
  }
  return markerText;
}
