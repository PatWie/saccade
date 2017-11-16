#ifndef MARKER_H
#define MARKER_H

#include <string>

namespace GUI {
class Marker {
 public:
  Marker();
  double x, y;
  bool active;

  std::string textLocation() const;
};
}; // namespace GUI

#endif // MARKER_H