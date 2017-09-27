#ifndef MIPMAP_LEVEL_H
#define MIPMAP_LEVEL_H

#include <vector>
#include "misc.h"

namespace Utils  {
class MipmapTile;
class GlManager;

class MipmapLevel {
 public:
  MipmapLevel();
  ~MipmapLevel();

  void setData(float* ptr,
               uint height, uint width, uint channels,
               uint tileSize = 512);

  void bindBuffer();
  void draw(Utils::GlManager *gl,
            int top, int left,
            int bottom, int right,
            double zoom);

  void clear();

 protected:
  float* getTileData(const float* ptr,
                     uint height, uint width,
                     uint minH, uint minW, uint maxH, uint maxW,
                     uint channels) const;

  std::vector< std::vector<MipmapTile*> > _tiles;
 private:

  uint _tileSize;
  uint _gridHeight;
  uint _gridWidth;

};

}; // namespace Utils

#endif // MIPMAP_LEVEL_H