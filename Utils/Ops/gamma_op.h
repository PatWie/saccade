#ifndef GAMMA_OP_H
#define GAMMA_OP_H

#include "img_op.h"

namespace Utils {
namespace Ops {

/**
 * @brief Gamma correction
 * @details [long description]
 * @return [description]
 */
class GammaOp : public ImgOp
{
public:
  GammaOp();
  float apply(float inp);
  float _gamma;
};

}
}

#endif