#ifndef IMG_OP_H
#define IMG_OP_H

#include <iostream>

namespace Utils {
namespace Ops {

class ImgOp
{

public:
  virtual float apply(float inp) = 0;
};

}
}

#endif