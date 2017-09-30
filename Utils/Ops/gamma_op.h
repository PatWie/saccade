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