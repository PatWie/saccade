#ifndef HISTOGRAM_OP_H
#define HISTOGRAM_OP_H

#include "img_op.h"

namespace Utils {
namespace Ops {

/**
 * @brief Apply histogram operation with information from GUI
 * @details [long description]
 * @return [description]
 */
class HistogramOp : public ImgOp {
 public:
  HistogramOp();
  float apply(float inp);

  // scaling from histogram
  struct scaling_t {
    float scale;
    float min;
    float max;
  } _scaling;

};

}
}

#endif // HISTOGRAM_OP_H