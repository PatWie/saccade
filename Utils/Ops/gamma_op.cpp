#include "gamma_op.h"
#include <cmath>

namespace Utils {
namespace Ops {


GammaOp::GammaOp() {
  _gamma = 2.2;
}

float GammaOp::apply(float inp) {
  return pow(inp, _gamma);
}

}
}
