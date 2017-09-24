#include "image_data.h"
#include <iostream>
#include <string>
#include <string.h>


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
  std::cout << "Utils::ImageData::ImageData()" << std::endl;
  _filename = filename;

  const FREE_IMAGE_FORMAT format = FreeImage_GetFileType(_filename.c_str(), 0);
  _data = FreeImage_Load(format, _filename.c_str());

  _width = FreeImage_GetWidth(_data);
  _height = FreeImage_GetHeight(_data);
  _channels = is_ptr_Grey(_data) ? 1 : 3;

  _raw_buf = new float[_channels * _height * _width];
  for (int c = 0; c < _channels; ++c)
  {
    for (int h = 0; h < _height; ++h)
    {
      // bgr
      const uint8_t* line = FreeImage_GetScanLine(_data, _height - 1 - h);
      for (int w = 0; w < _width; ++w)
      {
        _raw_buf[c * (_height * _width) + h * _width + w] = ((float) line[w * _channels + _channels - c - 1]) / 255.0f;
      }
    }
  }

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
  FreeImage_GetPixelColor(_data, w, _height - 1 - h, &color);
  return color;
}

float* Utils::ImageData::data() const {return _raw_buf;}
size_t Utils::ImageData::elements() const {return _height * _width * _channels;}
int Utils::ImageData::width() const {return _width;}
int Utils::ImageData::height() const {return _height;}
int Utils::ImageData::channels() const {return _channels;}


bool Utils::ImageData::is_ptr_RGB(FIBITMAP* data) {
  return FreeImage_GetColorType(data) == FIC_RGB &&
         FreeImage_GetBPP(data) == 24;
}

bool Utils::ImageData::is_ptr_Grey(FIBITMAP* data) {
  return FreeImage_GetColorType(data) == FIC_MINISBLACK &&
         FreeImage_GetBPP(data) == 8;
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