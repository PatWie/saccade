
#include "freeimage_loader.h"
#include <FreeImage.h>
#include <glog/logging.h>
#include <cmath>
#include <string>

namespace Utils {
namespace Loader {
FreeImageLoader::FreeImageLoader() {

}
FreeImageLoader::~FreeImageLoader() {

}

bool FreeImageLoader::canLoad(std::string fn) {
  const FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fn.c_str(), 0);
  return (format != FIF_UNKNOWN);
}


float* FreeImageLoader::load(std::string fn, int *_height, int *_width, int *_channels, float *_max_value)  {
  typedef FIBITMAP* FIBitmapPtr;
  FIBitmapPtr _data;

  const FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fn.c_str(), 0);
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

  _data = FreeImage_Load(format, fn.c_str());
  CHECK(_data != nullptr) << "cannot load image";
  CHECK_NOTNULL(_data);

  *_width = FreeImage_GetWidth(_data);
  *_height = FreeImage_GetHeight(_data);
  DLOG(INFO) << "FreeImage_GetWidth: " << *_width;
  DLOG(INFO) << "FreeImage_GetHeight: " << *_height;

  const int bpp = FreeImage_GetBPP(_data);
  DLOG(INFO) << "FreeImage_GetBPP: " << bpp;

  /*

  FI_ENUM(FREE_IMAGE_COLOR_TYPE) {
    FIC_MINISWHITE = 0,   //! min value is white
      FIC_MINISBLACK = 1,   //! min value is black
      FIC_RGB        = 2,   //! RGB color model
      FIC_PALETTE    = 3,   //! color map indexed
    FIC_RGBALPHA   = 4,   //! RGB color model with alpha channel
    FIC_CMYK       = 5    //! CMYK color model
  };
  */
  if (FreeImage_GetColorType(_data) == FIC_RGBALPHA) {
    DLOG(INFO) << "FreeImage_GetColorType: FIC_RGBALPHA";
    (*_channels) = 4;
  }
  if (FreeImage_GetColorType(_data) == FIC_RGB) {
    DLOG(INFO) << "FreeImage_GetColorType: FIC_RGB";
    (*_channels) = 3;
  }
  if (FreeImage_GetColorType(_data) == FIC_MINISBLACK) {
    DLOG(INFO) << "FreeImage_GetColorType: FIC_MINISBLACK";
    (*_channels) = 1;
  }

  bool is_rgba = ((*_channels) == 4);
  int off = 0;
  if (is_rgba) {
    DLOG(INFO) << "DETECTED RGBA";
    (*_channels) = 3;
    // bgra (skip a)
    off = 1;
  }

  *_max_value = std::pow(2, (float) FreeImage_GetBPP(_data) / (*_channels));
  DLOG(INFO) << "max value is " << *_max_value;
  DLOG(INFO) << "channels:    " << (*_channels);
  DLOG(INFO) << "off:         " << off;

  // handle special case: we ignore the alpha channel
  if (bpp == 32) {
    (*_channels) = 3;
    off = 1;
    *_max_value = std::pow(2, (float)24 / (*_channels));
  }

  CHECK_GE((*_channels), 0);

  float* _raw_buf = new float[(*_channels) * (*_height) * (*_width)];

  double sc;


  FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(_data);

  // https://github.com/patwie-stuff/FreeImage/blob/master/TestAPI/testImageType.cpp
  /*
  FI_ENUM(FREE_IMAGE_TYPE) {
    FIT_UNKNOWN = 0,  //! unknown type
    FIT_BITMAP  = 1,  //! standard image        : 1-, 4-, 8-, 16-, 24-, 32-bit
    FIT_UINT16  = 2,  //! array of unsigned short   : unsigned 16-bit
    FIT_INT16 = 3,  //! array of short        : signed 16-bit
    FIT_UINT32  = 4,  //! array of unsigned long    : unsigned 32-bit
    FIT_INT32 = 5,  //! array of long       : signed 32-bit
    FIT_FLOAT = 6,  //! array of float        : 32-bit IEEE floating point
    FIT_DOUBLE  = 7,  //! array of double       : 64-bit IEEE floating point
    FIT_COMPLEX = 8,  //! array of FICOMPLEX      : 2 x 64-bit IEEE floating point
    FIT_RGB16 = 9,  //! 48-bit RGB image      : 3 x 16-bit
    FIT_RGBA16  = 10, //! 64-bit RGBA image     : 4 x 16-bit
    FIT_RGBF  = 11, //! 96-bit RGB float image    : 3 x 32-bit IEEE floating point
    FIT_RGBAF = 12  //! 128-bit RGBA float image  : 4 x 32-bit IEEE floating point
  };
  */
  switch (image_type) {
  case FIT_UNKNOWN:
    CHECK_NE(image_type, FIT_UNKNOWN);
    break;
  case FIT_BITMAP:
    DLOG(INFO) << "case FIT_BITMAP";
    for (int c = 0; c < (*_channels); ++c) {
      for (int h = 0; h < (*_height); ++h) {
        const uint8_t* line = FreeImage_GetScanLine(_data, (*_height) - 1 - h);
        for (int w = 0; w < (*_width); ++w) {
          float val = ((float) line[w * ((*_channels) + off) + (*_channels) - c - 1]);
          _raw_buf[c * ((*_height) * (*_width)) + h * (*_width) + w] = val;
        }
      }
    }
    break;
  case FIT_UINT16:
    DLOG(INFO) << "case FIT_UINT16";
    (*_channels) = 1;
    // _max_value = 1.0;
    off = 0;

    for (int c = 0; c < (*_channels); ++c) {
      for (int h = 0; h < (*_height); ++h) {
        const unsigned short* line = (unsigned short *)FreeImage_GetScanLine(_data, (*_height) - 1 - h);
        for (int w = 0; w < (*_width); ++w) {
          float val = ((float) line[w * ((*_channels) + off) + (*_channels) - c - 1]);
          _raw_buf[c * ((*_height) * (*_width)) + h * (*_width) + w] = val;
        }
      }
    }
    break;
  case FIT_INT16:
    DLOG(INFO) << "case FIT_INT16";
    break;
  case FIT_UINT32:
    DLOG(INFO) << "case FIT_UINT32";
    break;
  case FIT_INT32:
    DLOG(INFO) << "case FIT_INT32";
    break;
  case FIT_FLOAT:
    DLOG(INFO) << "case FIT_FLOAT";
    for (int c = 0; c < (*_channels); ++c) {
      for (int h = 0; h < (*_height); ++h) {
        const float* line = (float *)FreeImage_GetScanLine(_data, (*_height) - 1 - h);
        for (int w = 0; w < (*_width); ++w) {
          float val = ((float) line[w * ((*_channels) + off) + (*_channels) - c - 1]);
          _raw_buf[c * ((*_height) * (*_width)) + h * (*_width) + w] = val;
        }
      }
    }
    break;
  case FIT_DOUBLE:
    DLOG(INFO) << "case FIT_DOUBLE";
    for (int c = 0; c < (*_channels); ++c) {
      for (int h = 0; h < (*_height); ++h) {
        const double* line = (double *)FreeImage_GetScanLine(_data, (*_height) - 1 - h);
        for (int w = 0; w < (*_width); ++w) {
          float val = ((float) line[w * ((*_channels) + off) + (*_channels) - c - 1]);
          _raw_buf[c * ((*_height) * (*_width)) + h * (*_width) + w] = val;
        }
      }
    }
    break;
  case FIT_COMPLEX:
    DLOG(INFO) << "case FIT_COMPLEX";
    break;
  case FIT_RGB16:
    DLOG(INFO) << "case FIT_RGB16";
    sc = 1.; // std::pow(2, (double) FreeImage_GetBPP(_data) / (*_channels));
    for (int c = 0; c < (*_channels); ++c) {
      for (int h = 0; h < (*_height); ++h) {
        const FIRGB16 *line = (FIRGB16 *) FreeImage_GetScanLine(_data, (*_height) - 1 - h);
        for (int w = 0; w < (*_width); ++w) {
          _raw_buf[0 * ((*_height) * (*_width)) + h * (*_width) + w] = (float) line[w].red / sc;
          _raw_buf[1 * ((*_height) * (*_width)) + h * (*_width) + w] = (float) line[w].green / sc;
          _raw_buf[2 * ((*_height) * (*_width)) + h * (*_width) + w] = (float) line[w].blue / sc;
        }
      }
    }
    break;
  case FIT_RGBF:
    DLOG(INFO) << "case FIT_RGBF";
    break;
  case FIT_RGBA16:
    DLOG(INFO) << "case FIT_RGBA16";
    // rescale to [0., 1.]
    sc = std::pow(2, (double) FreeImage_GetBPP(_data) / (*_channels));
    for (int c = 0; c < (*_channels); ++c) {
      for (int h = 0; h < (*_height); ++h) {
        const FIRGBA16 *line = (FIRGBA16 *) FreeImage_GetScanLine(_data, (*_height) - 1 - h);
        for (int w = 0; w < (*_width); ++w) {
          _raw_buf[0 * ((*_height) * (*_width)) + h * (*_width) + w] = ((double) line[w].red) / sc;
          _raw_buf[1 * ((*_height) * (*_width)) + h * (*_width) + w] = ((double) line[w].green) / sc;
          _raw_buf[2 * ((*_height) * (*_width)) + h * (*_width) + w] = ((double) line[w].blue) / sc;
        }
      }
    }
    *_max_value = 1.0;
    break;
  case FIT_RGBAF:
    DLOG(INFO) << "case FIT_RGBAF";

    break;
  }

  FreeImage_Unload(_data);
  return _raw_buf;
}


}; // namespace Loader
}; // namespace Utils