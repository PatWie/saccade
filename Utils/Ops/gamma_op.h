#ifndef GAMMA_OP_H
#define GAMMA_OP_H

#include "img_op.h"

/* Just a simple example on how to add operation.
There won't be probably more operations as this should be an image viewer only.
*/

namespace Utils {
namespace Ops {

/**
 * @brief Gamma correction
 * @details [long description]
 * @return [description]
 */
class GammaOp : public ImgOp {
 public:
  GammaOp();
  void apply_cpu(const float* src, float* dst, size_t H, size_t W, size_t C);
  void apply_gpu(const float* src, float* dst, size_t H, size_t W, size_t C);
  float _gamma;
};

}
}

#endif