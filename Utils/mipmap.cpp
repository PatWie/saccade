#include <iostream>
#include <math.h>
#include "image_data.h"
#include "mipmap.h"
#include "mipmap_level.h"
#include "gl_manager.h"


void Utils::Mipmap::clear() {
  _empty = true;
  for(auto &&level : _levels){
    level->clear();
    delete level;
  }
  _levels.clear();
}
bool Utils::Mipmap::empty() {
  return _empty;
}

Utils::Mipmap::Mipmap() {
  _empty = true;
  std::cout << "Utils::Mipmap::Mipmap" << std::endl;

}

void Utils::Mipmap::setData(float *ptr,
                            uint height, uint width, uint channels,
                            uint tileSize) {

  std::cout << "Utils::Mipmap::set_image START" << std::endl;


  // unload old textures

  // get new dimensions

  // find next greater power of 2, e.g. 512 -> 2**10
  // data should fit powers of 2**x
  uint depth = 1.0;
  if ( width < height ) {
    depth = 1.0 + log(width) / log(2.0);
  } else {
    depth = 1.0 + log(height) / log(2.0);
  }

  uint working_height = height;
  uint working_width = width;
  float *working_ptr = ptr;

  MipmapLevel* level = new MipmapLevel();
  _levels.push_back(level);
  _levels[0]->setData(working_ptr,
                      working_height, working_width, channels,
                      tileSize);

  std::cout << "create level " << 0
            << " " << working_height
            << " " << working_width
            << std::endl;

  for (uint d = 1; d < depth; ++d) {

    float* old_ptr = working_ptr;
    working_ptr = downsample(old_ptr,
                             &working_height, &working_width, channels);
    delete[] old_ptr;


    std::cout << "create level " << d
              << " " << working_height
              << " " << working_width
              << std::endl;
    MipmapLevel* level = new MipmapLevel();
    _levels.push_back(level);
    _levels[d]->setData(working_ptr,
                        working_height, working_width, channels,
                        tileSize);
  }
  delete [] working_ptr;


  std::cout << "Utils::Mipmap::set_image END" << std::endl;
  _empty = false;
}

float* Utils::Mipmap::downsample(const float* ptr,
                                 uint* ptr_height, uint* ptr_width, uint channels) const {

  uint height = *ptr_height;
  uint width = *ptr_width;

  //Downsampling by using "space" aspect for each pixel
  uint nwidth = width / 2;
  uint nheight = height / 2;

  // downsample the width
  float* working_Data = new float[nwidth * height * channels ];
  // downsample the height
  float* d = new float[nwidth * nheight * channels ];

  // sampling factors
  double widthRatio = (double)nwidth / (double)width;
  double heightRatio = (double)nheight / (double)height;

  // ==============================================================
  // downsample H,W,C --> H,W/2,C
  // ==============================================================

  for ( uint c = 0; c < channels; c++ ) {
    // initialize to zero
    for ( uint h = 0; h < height; h++ ) {
      for ( uint w = 0; w < nwidth; w++ ) {
        working_Data[(w + h * nwidth)*channels + c] = 0.0;
      }
    }

    // load pixel from bigger (original image) to halfsize image
    for (uint h = 0; h < height; h++ ) {
      for ( uint w = 0; w < width; w++ ) {

        // left and right pixel
        uint b1 = int(w * widthRatio);
        uint b2 = int((w + 1) * widthRatio);

        // double span = ((w + 1) * widthRatio) - (w * widthRatio);
        if ( b1 == b2 ) {
          // perfect match
          working_Data[(b1 + h * nwidth)*channels + c] +=
            widthRatio * ptr[(w + h * width) * channels + c];
        } else {
          // get weight (linear interpolation)
          double lFactor = (b2 - (w * widthRatio) ) / widthRatio;
          double rFactor = 1.0 - lFactor;
          if ( b1 < nwidth )
            working_Data[(b1 + h * nwidth)*channels + c] +=
              widthRatio * lFactor * ptr[(w + h * width) * channels + c];
          if ( b2 < nwidth ) {
            working_Data[(b2 + h * nwidth)*channels + c] +=
              widthRatio * rFactor * ptr[(w + h * width) * channels + c];
          }
        }
      }
    }
  }

  // ==============================================================
  // downsample H,W/2,C -> H/2,W/2,C
  // ==============================================================

  for ( uint c = 0; c < channels; c++ ) {
    for ( uint h = 0; h < nheight; h++ ) {
      for ( uint w = 0; w < nwidth; w++ ) {
        d[(w + h * nwidth)*channels + c] = 0.0;
      }
    }
    for ( uint w = 0; w < nwidth; w++ ) {
      for (uint h = 0; h < height; h++ ) {
        uint b1 = int(h * heightRatio);
        uint b2 = int((h + 1) * heightRatio);
        if ( b1 == b2 ) {
          d[(w + b1 * nwidth)*channels + c] += heightRatio * working_Data[(w + h * nwidth) * channels + c];
        } else {
          double lFactor = (b2 - (h * heightRatio) ) / heightRatio;
          double rFactor = 1.0 - lFactor;
          if ( b1 < nheight )
            d[(w + b1 * nwidth)*channels + c] += heightRatio * lFactor * working_Data[(w + h * nwidth) * channels + c];
          if ( b2 < nheight ) {
            d[(w + b2 * nwidth)*channels + c] += heightRatio * rFactor * working_Data[(w + h * nwidth) * channels + c];
          }
        }
      }
    }
  }
  delete [] working_Data;
  *ptr_width = nwidth;
  *ptr_height = nheight;
  return d;
}

void Utils::Mipmap::draw(Utils::GlManager *gl,
                         int top, int left,
                         int bottom, int right,
                         double zoom) {
  // std::cout << "Utils::Mipmap::draw START" << std::endl;
  // std::cout << "top " << top
  //           << "left " << left
  //           << "bottom " << bottom
  //           << "right " << right
  //           << std::endl;

  // find best level for given zoom_level
  int currentLevel = 0;
  if ( zoom < 1.0 )
    currentLevel = (unsigned int)(-(log(zoom) / log(2.0)) + 0.01);
  /*
  zoom_level --> current_level
  1/2 -> 1.01 -> 1
  1/4 -> 2.01 -> 2
  1/8 -> 3.01 -> 3
  */
  // clip values to [0, num_levels]
  currentLevel = std::max(currentLevel, 0);
  currentLevel = std::min(currentLevel, (int)_levels.size() - 1);

  /*
  1 -> 0.50
  2 -> 0.25
  3 -> 0.125
  4 -> 0.0625
  */
  zoom = pow( 2.0, -(double)currentLevel );

  // std::cout << "Utils::Mipmap::draw LEVEL " << currentLevel << std::endl;


  _levels[currentLevel]->draw(gl, top, left, bottom, right, zoom);
  // std::cout << "Utils::Mipmap::draw END" << std::endl;
  // https://doc-snapshots.qt.io/qt5-dev/qopenglfunctions.html

}