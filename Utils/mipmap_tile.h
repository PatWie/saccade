#ifndef MIPMAP_TILE_H
#define MIPMAP_TILE_H

#include <vector>
#include "misc.h"

namespace Utils  {

template<typename T>
class GlObject;
class GlManager;

class MipmapTile {
 public:
  MipmapTile(float* ptr, uint height, uint width, uint channels);
  ~MipmapTile();

  void draw(Utils::GlManager *gl, double posH, double posW);

  const Utils::GlObject<float> *obj() const;

 private:

  Utils::GlObject<float> *_obj;

};

}; // namespace Utils

#endif // MIPMAP_TILE_H