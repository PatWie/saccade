#include "histogram_op.h"
#include <cmath>

#include <glog/logging.h>

namespace Utils {
namespace Ops {


HistogramOp::HistogramOp() {}

float HistogramOp::apply(float inp) {
  // map from [0, img_max] _> [0, 1]
  float scaled = inp / _scaling.scale;
  scaled -= _scaling.min / _scaling.scale;
  scaled /= (_scaling.max - _scaling.min) / _scaling.scale;
  // apply clipping
  scaled = std::min(scaled, 1.f);
  scaled = std::max(scaled, 0.f);
  return scaled;
}

}
}
