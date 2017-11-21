#ifndef IMG_OP_H
#define IMG_OP_H

#include <iostream>

namespace Utils {
namespace Ops {

class ImgOp {

 public:
  virtual void apply_cpu(const float* src, float* dst, size_t H, size_t W, size_t C) = 0;
  virtual void apply_gpu(const float* src, float* dst, size_t H, size_t W, size_t C) = 0;
};

}
}

#endif