#include "image_data.h"
#include <iostream>
#include <string>
#include <string.h>
#include <glog/logging.h>

/* This file is responsible to load the image data

PNG:
- gray (8)
- gray + alpha (16)
- rgb (24)
- rgb + alpha (40)
JPG:
- rgb (24)
*/


Utils::ImageData::~ImageData() {}
Utils::ImageData::ImageData(float*d, int h, int w, int c)
  : _raw_buf(d), _height(h), _width(w), _channels(c) {}
Utils::ImageData::ImageData(Utils::ImageData *i) {
  _height = i->height();
  _width = i->width();
  _channels = i->channels();
  _raw_buf = new float[i->elements()];
  memcpy( _raw_buf, i->data(), sizeof(float) * i->elements() );

}

Utils::ImageData::ImageData(std::string filename) {
  LOG(INFO) << "Utils::ImageData::ImageData()";
  _filename = filename;

  const FREE_IMAGE_FORMAT format = FreeImage_GetFileType(_filename.c_str(), 0);
  _data = FreeImage_Load(format, _filename.c_str());

  CHECK_NOTNULL(_data);

  _width = FreeImage_GetWidth(_data);
  _height = FreeImage_GetHeight(_data);

  LOG(INFO) << "FreeImage_GetBPP: " << (int) FreeImage_GetBPP(_data);

  if (FreeImage_GetColorType(_data) == FIC_RGBALPHA && FreeImage_GetBPP(_data) == 32) {
    LOG(INFO) << "FreeImage_GetColorType: FIC_RGBALPHA";
    _channels = 4;
  }
  if (FreeImage_GetColorType(_data) == FIC_RGB && FreeImage_GetBPP(_data) == 24) {
    LOG(INFO) << "FreeImage_GetColorType: FIC_RGB";
    _channels = 3;
  }
  if (FreeImage_GetColorType(_data) == FIC_MINISBLACK && FreeImage_GetBPP(_data) == 8) {
    LOG(INFO) << "FreeImage_GetColorType: FIC_MINISBLACK";
    _channels = 1;
  }

  bool is_rgba = (_channels == 4);
  int off = 0;
  if(is_rgba){
    LOG(INFO) << "DETECTED RGBA";
    _channels = 3;
    // bgra (skip a)
    off = 1;
  }

  CHECK_GE(_channels, 0);

  _raw_buf = new float[_channels * _height * _width];
  for (int c = 0; c < _channels; ++c) {
    for (int h = 0; h < _height; ++h) {
      // bgr
      const uint8_t* line = FreeImage_GetScanLine(_data, _height - 1 - h);
      for (int w = 0; w < _width; ++w) {
        float val = ((float) line[w * (_channels + off) + _channels - c - 1]) / 255.0f;
        _raw_buf[c * (_height * _width) + h * _width + w] = val;
      }
    }
  }

  FreeImage_Unload(_data);

}


/**
 * @brief Get RGB value
 * @details [long description]
 *
 *   RGBQUAD color = (*this)(0, 0);
*    std::cout << (int)color.rgbRed << std::endl;
*    std::cout << (int)color.rgbGreen << std::endl;
*    std::cout << (int)color.rgbBlue << std::endl;
 *
 * @param h vertical position (from top)
 * @param w horizontal position (from left)
 *
 * @return struct with (r, b, g)
 */
RGBQUAD Utils::ImageData::operator()(int h, int w) const {
  RGBQUAD color;
  // FIXME: use _raw_buf instead;
  // FreeImage_GetPixelColor(_data, w, _height - 1 - h, &color);
  return color;
}

float* Utils::ImageData::data() const {return _raw_buf;}
size_t Utils::ImageData::elements() const {return _height * _width * _channels;}
int Utils::ImageData::width() const {return _width;}
int Utils::ImageData::height() const {return _height;}
int Utils::ImageData::channels() const {return _channels;}



void Utils::ImageData::clear(bool remove) {
  LOG(INFO) << "Utils::ImageData::clear";
  // the _bud_data is already delete (so dont do it here again)
  if (remove)
    delete[] _raw_buf;
  _height = 0;
  _width = 0;
  _channels = 0;
}

/*
// calculate the number of bytes per pixel
unsigned bytespp = FreeImage_GetLine(src) / FreeImage_GetWidth(src);
// calculate the number of samples per pixel
unsigned samples = bytespp / sizeof(T);
where 'T' is 'BYTE' for FIT_BITMAP, 'WORD' for FIT_UINT16, FIT_RGB16,
FIT_RGBA16 and 'float' for FIT_FLOAT, FIT_RGBF, FIT_RGBAF.


  // convert to float
  int bpp = FreeImage_GetBPP(_data);
*/