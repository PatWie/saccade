#include <iostream>
#include <algorithm>

#include "misc.h"
#include "mipmap_tile.h"
#include "mipmap_level.h"
#include "gl_manager.h"



Utils::MipmapLevel::MipmapLevel() {}
Utils::MipmapLevel::~MipmapLevel() {}
void Utils::MipmapLevel::clear() {
  for(auto &&tile_line : _tiles){
    for(auto &&tile : tile_line){
      tile->clear();
      delete tile;
    }
    tile_line.clear();
  }
  _tiles.clear();
}


void Utils::MipmapLevel::setData(float* ptr,
                                 uint height, uint width, uint channels,
                                 uint tileSize) {
  // LOG(INFO) << "Utils::MipmapLevel::setData " << height << " " << width << " " << tileSize;
  _tileSize = tileSize;

  // generate enough tiles (like block and grid)
  uint tileNumH = width / tileSize;
  uint tileNumV = height / tileSize;
  uint borderRight = width % tileSize;
  uint borderLower = height % tileSize;

  _gridWidth = tileNumH + std::min(borderRight, 1u);
  _gridHeight = tileNumV + std::min(borderLower, 1u);

  for (uint h = 0; h < _gridHeight; ++h) {
    _tiles.push_back(std::vector<MipmapTile*>(_gridWidth) );
  }

  // n = h * width + w
  #pragma omp parallel for
  for(uint n = 0; n < _gridHeight * _gridWidth; n++ ){
      uint h = n / _gridWidth;
      uint w = n % _gridWidth;
  // for (uint h = 0; h < _gridHeight; ++h) {
  //   for (uint w = 0; w < _gridWidth; ++w) {

      const uint minH = h * tileSize;
      const uint minW = w * tileSize;

      const uint maxH = std::min(((h + 1) * tileSize), height);
      const uint maxW = std::min(((w + 1) * tileSize), width);

      const uint diffH = maxH - minH;
      const uint diffW = maxW - minW;

      float* d = getTileData(ptr, height, width,
                             minH, minW, maxH, maxW,
                             channels);

      MipmapTile *tile = new MipmapTile(d, diffH, diffW, channels);
      _tiles[h][w] = tile;

    }
  // }
}


float* Utils::MipmapLevel::getTileData(const float* ptr,
                                       uint height, uint width,
                                       uint minH, uint minW,
                                       uint maxH, uint maxW,
                                       uint channels) const {

  const uint diffW = maxW - minW;
  const uint diffH = maxH - minH;

  float *d = new float[diffH * diffW * channels];

  for (uint c = 0; c < channels; ++c) {
    for (uint h = 0; h < diffH; ++h) {
      for (uint w = 0; w < diffW; ++w) {
        d[h * diffW * channels + w * channels + c] =
          ptr[c * (height * width) + (h + minH) * width + (w + minW)];
      }
    }
  }

  return d;
}

void Utils::MipmapLevel::draw(Utils::GlManager *gl,
                              int top, int left,
                              int bottom, int right,
                              double zoom) {

  // a visual fix ?
  right++;
  bottom++;
  if ( left == 0 ) left--;
  if ( top == 0 ) top--;
  if ( right == 0 ) right++;
  if ( bottom == 0 ) bottom++;

  // set zoom factor
  if ( zoom < 1.0)
    glScalef( 1.0 / zoom, 1.0 / zoom, 1.0 );

  for (uint h = 0; h < _gridHeight; ++h) {
    for (uint w = 0; w < _gridWidth; ++w) {
      const double posW = (double) _tileSize * w;
      const double posH = (double) _tileSize * h;

      const double checkPosW = posW / zoom;
      const double checkPosH = posH / zoom;

      const double checkPosW2 = (checkPosW + (_tiles[h][w]->obj()->width / zoom));
      const double checkPosH2 = (checkPosH + (_tiles[h][w]->obj()->height / zoom));

      if ( right > checkPosW && left < checkPosW2 &&
           bottom > checkPosH && top < checkPosH2 ) {
        _tiles[h][w]->draw(gl, posH, posW);
      }
    }
  }

}
