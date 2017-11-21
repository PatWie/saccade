#include "gamma_op.h"
#include <cmath>

namespace Utils {
namespace Ops {


GammaOp::GammaOp() {
  _gamma = 2.2;
}


void GammaOp::apply_cpu(const float* src, float* dst,
                     size_t H, size_t W, size_t C) {
  #pragma omp parallel for
  for (size_t i = 0; i < H * W * C; ++i)
    dst[i] = pow(src[i], _gamma);

}

void GammaOp::apply_gpu(const float* src, float* dst,
                     size_t H, size_t W, size_t C) {
  // TODO
  apply_cpu(src, dst, H, W, C);
}

}
}
