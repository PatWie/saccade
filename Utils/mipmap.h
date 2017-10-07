#ifndef MIPMAP_H
#define MIPMAP_H

#include <memory>
#include <vector>
#include "misc.h"

namespace Utils  {

class ImageData;
class MipmapLevel;
class GlManager;

class Mipmap {
 public:
  Mipmap();
  ~Mipmap();

  void setData(float* ptr,
               uint height, uint width, uint channels,
               uint tileSize = 512);

  float* downsample(const float* ptr, uint* height, uint* width, uint channels) const;
  void bindBuffer();
  void draw(Utils::GlManager *gl,
            int top, int left,
            int bottom, int right,
            double zoom);

  std::vector<MipmapLevel*> _levels;

  void clear();
  bool empty();

 private:
  ImageData *_img;

  bool _empty;

};

}; // namespace Utils

#endif // MIPMAP_H
