#include "image_data.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cmath>
#include <string.h>
#include <glog/logging.h>
#include "misc.h"

/* This file is responsible to load the image data

PNG:
- gray (8)
- gray + alpha (16)
- rgb (24)
- rgb + alpha (40)
JPG:
- rgb (24)
*/

bool Utils::ImageData::validFile(std::string filename) {
  const FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename.c_str(), 0);
  return (format != FIF_UNKNOWN);
}

Utils::ImageData::~ImageData() {}

Utils::ImageData::ImageData(float*d, int h, int w, int c)
  : _raw_buf(d), _height(h), _width(w), _channels(c) {
}

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
  CHECK(format != FIF_UNKNOWN) << "unkown fileformat";
  /*
  FIF_UNKNOWN  Unknown format (returned value only, never use it as input value)
  FIF_BMP      Windows or OS/2 Bitmap File (*.BMP)
  FIF_ICO      Windows Icon (*.ICO)
  FIF_JPEG     Independent JPEG Group (*.JPG, *.JIF, *.JPEG, *.JPE)
  FIF_JNG      JPEG Network Graphics (*.JNG)
  FIF_KOALA    Commodore 64 Koala format (*.KOA)
  FIF_LBM      Amiga IFF (*.IFF, *.LBM)
  FIF_IFF      Amiga IFF (*.IFF, *.LBM)
  FIF_MNG      Multiple Network Graphics (*.MNG)
  FIF_PBM      Portable Bitmap (ASCII) (*.PBM)
  FIF_PBMRAW   Portable Bitmap (BINARY) (*.PBM)
  FIF_PCD      Kodak PhotoCD (*.PCD)
  FIF_PCX      Zsoft Paintbrush PCX bitmap format (*.PCX)
  FIF_PGM      Portable Graymap (ASCII) (*.PGM)
  FIF_PGMRAW   Portable Graymap (BINARY) (*.PGM)
  FIF_PNG      Portable Network Graphics (*.PNG)
  FIF_PPM      Portable Pixelmap (ASCII) (*.PPM)
  FIF_PPMRAW   Portable Pixelmap (BINARY) (*.PPM)
  FIF_RAS      Sun Rasterfile (*.RAS)
  FIF_TARGA    truevision Targa files (*.TGA, *.TARGA)
  FIF_TIFF     Tagged Image File Format (*.TIF, *.TIFF)
  FIF_WBMP     Wireless Bitmap (*.WBMP)
  FIF_PSD      Adobe Photoshop (*.PSD)
  FIF_CUT      Dr. Halo (*.CUT)
  FIF_XBM      X11 Bitmap Format (*.XBM)
  FIF_XPM      X11 Pixmap Format (*.XPM)
  FIF_DDS      DirectDraw Surface (*.DDS)
  FIF_GIF      Graphics Interchange Format (*.GIF)
  FIF_HDR      High Dynamic Range (*.HDR)
  FIF_FAXG3    Raw Fax format CCITT G3 (*.G3)
  FIF_SGI      Silicon Graphics SGI image format (*.SGI)
  FIF_EXR      OpenEXR format (*.EXR)
  FIF_J2K      JPEG-2000 format (*.J2K, *.J2C)
  FIF_JP2      JPEG-2000 format (*.JP2)
  FIF_PFM      Portable FloatMap (*.PFM)
  FIF_PICT     Macintosh PICT (*.PICT)
  FIF_RAW      RAW camera image (*.*)
  */

  _data = FreeImage_Load(format, _filename.c_str());
  CHECK(_data != nullptr) << "cannot load image";
  CHECK_NOTNULL(_data);

  _width = FreeImage_GetWidth(_data);
  _height = FreeImage_GetHeight(_data);

  const int bpp = FreeImage_GetBPP(_data);
  LOG(INFO) << "FreeImage_GetBPP: " << bpp;

  /*
  FIC_MINISWHITE  min value is white
  FIC_MINISBLACK  min value is black
  FIC_RGB         RGB color model
  FIC_PALETTE     color map indexed
  FIC_RGBALPHA    RGB color model with alpha channel
  FIC_CMYK        CMYK color model
  */
  if (FreeImage_GetColorType(_data) == FIC_RGBALPHA) {
    LOG(INFO) << "FreeImage_GetColorType: FIC_RGBALPHA";
    _channels = 4;
  }
  if (FreeImage_GetColorType(_data) == FIC_RGB) {
    LOG(INFO) << "FreeImage_GetColorType: FIC_RGB";
    _channels = 3;
  }
  if (FreeImage_GetColorType(_data) == FIC_MINISBLACK) {
    LOG(INFO) << "FreeImage_GetColorType: FIC_MINISBLACK";
    _channels = 1;
  }

  bool is_rgba = (_channels == 4);
  int off = 0;
  if (is_rgba) {
    LOG(INFO) << "DETECTED RGBA";
    _channels = 3;
    // bgra (skip a)
    off = 1;
  }

  _max_value = std::pow(2, (float) FreeImage_GetBPP(_data) / _channels);
  LOG(INFO) << "max value is " << _max_value;
  LOG(INFO) << "channels:    " << _channels;
  LOG(INFO) << "off:         " << off;

  // handle special case: we ignore the alpha channel
  if (bpp == 32) {
    _channels = 3;
    off = 1;
    _max_value = std::pow(2, (float)24 / _channels);
  }

  CHECK_GE(_channels, 0);

  _raw_buf = new float[_channels * _height * _width];

  double sc;


  FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(_data);

  // https://github.com/patwie-stuff/FreeImage/blob/master/TestAPI/testImageType.cpp
  /*
  FIT_UNKNOWN     unknown type
  FIT_BITMAP      standard image : 1-, 4-, 8-, 16-, 24-, 32-bit
  FIT_UINT16      array of unsigned short : unsigned 16-bit
  FIT_INT16       array of short : signed 16-bit
  FIT_UINT32      array of unsigned long : unsigned 32-bit
  FIT_INT32       array of long : signed 32-bit
  FIT_FLOAT       array of float : 32-bit IEEE floating point
  FIT_DOUBLE      array of double : 64-bit IEEE floating point
  FIT_COMPLEX     array of FICOMPLEX : 2 x 64-bit IEEE floating point
  FIT_RGB16       48-bit RGB image : 3 x 16-bit
  FIT_RGBA16      64-bit RGBA image : 4 x 16-bit
  FIT_RGBF        96-bit RGB float image : 3 x 32-bit IEEE floating point
  FIT_RGBAF       128-bit RGBA float image : 4 x 32-bit IEEE floating point
  */
  switch (image_type) {
  case FIT_BITMAP:
    LOG(INFO) << "case FIT_BITMAP";
    for (int c = 0; c < _channels; ++c) {
      for (int h = 0; h < _height; ++h) {
        const uint8_t* line = FreeImage_GetScanLine(_data, _height - 1 - h);
        for (int w = 0; w < _width; ++w) {
          float val = ((float) line[w * (_channels + off) + _channels - c - 1]);
          _raw_buf[c * (_height * _width) + h * _width + w] = val;
        }
      }
    }
    break;
  case FIT_UINT16:
    LOG(INFO) << "case FIT_UINT16";
    _channels = 1;
    // _max_value = 1.0;
    off = 0;

    for (int c = 0; c < _channels; ++c) {
      for (int h = 0; h < _height; ++h) {
        const unsigned short* line = (unsigned short *)FreeImage_GetScanLine(_data, _height - 1 - h);
        for (int w = 0; w < _width; ++w) {
          float val = ((float) line[w * (_channels + off) + _channels - c - 1]);
          _raw_buf[c * (_height * _width) + h * _width + w] = val;
        }
      }
    }
    break;
  case FIT_INT16:
    LOG(INFO) << "case FIT_INT16";
    break;
  case FIT_UINT32:
    LOG(INFO) << "case FIT_UINT32";
    break;
  case FIT_INT32:
    LOG(INFO) << "case FIT_INT32";
    break;
  case FIT_FLOAT:
    LOG(INFO) << "case FIT_FLOAT";
    for (int c = 0; c < _channels; ++c) {
      for (int h = 0; h < _height; ++h) {
        const float* line = (float *)FreeImage_GetScanLine(_data, _height - 1 - h);
        for (int w = 0; w < _width; ++w) {
          float val = ((float) line[w * (_channels + off) + _channels - c - 1]);
          _raw_buf[c * (_height * _width) + h * _width + w] = val;
        }
      }
    }
    break;
  case FIT_DOUBLE:
    LOG(INFO) << "case FIT_DOUBLE";
    for (int c = 0; c < _channels; ++c) {
      for (int h = 0; h < _height; ++h) {
        const double* line = (double *)FreeImage_GetScanLine(_data, _height - 1 - h);
        for (int w = 0; w < _width; ++w) {
          float val = ((float) line[w * (_channels + off) + _channels - c - 1]);
          _raw_buf[c * (_height * _width) + h * _width + w] = val;
        }
      }
    }
    break;
  case FIT_COMPLEX:
    LOG(INFO) << "case FIT_COMPLEX";
    break;
  case FIT_RGB16:
    LOG(INFO) << "case FIT_RGB16";
    sc = 1.; // std::pow(2, (double) FreeImage_GetBPP(_data) / _channels);
    for (int c = 0; c < _channels; ++c) {
      for (int h = 0; h < _height; ++h) {
        const FIRGB16 *line = (FIRGB16 *) FreeImage_GetScanLine(_data, _height - 1 - h);
        for (int w = 0; w < _width; ++w) {
          _raw_buf[0 * (_height * _width) + h * _width + w] = (float) line[w].red / sc;
          _raw_buf[1 * (_height * _width) + h * _width + w] = (float) line[w].green / sc;
          _raw_buf[2 * (_height * _width) + h * _width + w] = (float) line[w].blue / sc;
        }
      }
    }
    break;
  case FIT_RGBF:
    LOG(INFO) << "case FIT_RGBF";
    break;
  case FIT_RGBA16:
    LOG(INFO) << "case FIT_RGBA16";
    // rescale to [0., 1.]
    sc = std::pow(2, (double) FreeImage_GetBPP(_data) / _channels);
    for (int c = 0; c < _channels; ++c) {
      for (int h = 0; h < _height; ++h) {
        const FIRGBA16 *line = (FIRGBA16 *) FreeImage_GetScanLine(_data, _height - 1 - h);
        for (int w = 0; w < _width; ++w) {
          _raw_buf[0 * (_height * _width) + h * _width + w] = ((double) line[w].red) / sc;
          _raw_buf[1 * (_height * _width) + h * _width + w] = ((double) line[w].green) / sc;
          _raw_buf[2 * (_height * _width) + h * _width + w] = ((double) line[w].blue) / sc;
        }
      }
    }
    _max_value = 1.0;
    break;
  case FIT_RGBAF:
    LOG(INFO) << "case FIT_RGBAF";

    break;
  }

  FreeImage_Unload(_data);
}


float Utils::ImageData::operator()(int h, int w, int c) const {
  return value(h, w, c);
}
float Utils::ImageData::value(int h, int w, int c) const {
  return _raw_buf[c * (_height * _width) + h * _width + w];
}

float Utils::ImageData::value(int t, int c) const {
  return _raw_buf[c * (_height * _width) + t];
}

float* Utils::ImageData::data() const {return _raw_buf;}
size_t Utils::ImageData::elements() const {return _height * _width * _channels;}
int Utils::ImageData::width() const {return _width;}
int Utils::ImageData::height() const {return _height;}
int Utils::ImageData::channels() const {return _channels;}
int Utils::ImageData::area() const {return _height * _width;}
float Utils::ImageData::max() const {return _max_value;}

std::string Utils::ImageData::color(int h, int w) const {
  std::stringstream stream;
  stream << std::setprecision(3);

  const int ch = height();
  const int cw = width();

  CHECK(channels() == 3 || channels() == 1) << "color string only for 1 or 3 channels";

  if (0 <= w && w < cw)
    if (0 <= h && h < ch) {
      // within image
      if (channels() == 1) {
        stream << "<font color=" << misc_theme_gray.name().toStdString() << ">"  << value(h, w, 0) << "</font>" << " ";
      }
      if (channels() == 3) {
        stream << "<font color=" << misc_theme_red.name().toStdString() << ">"  << value(h, w, 0) << "</font>" << " ";
        stream << "<font color=" << misc_theme_green.name().toStdString() << ">"  << value(h, w, 1) << "</font>" << " ";
        stream << "<font color=" << misc_theme_blue.name().toStdString() << ">"  << value(h, w, 2) << "</font>" << " ";
      }

    }

  return stream.str();
}


void Utils::ImageData::clear(bool remove) {
  LOG(INFO) << "Utils::ImageData::clear";
  // the _bud_data is already delete (so dont do it here again)
  if (remove)
    if (_raw_buf != nullptr)
      delete[] _raw_buf;
  _height = 0;
  _width = 0;
  _channels = 0;
}

