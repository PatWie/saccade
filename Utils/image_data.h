#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <FreeImage.h>
#include <memory>
#include <string>
#include <vector>



namespace Utils {

namespace Ops{
 class ImgOp;
}

class ImageData {

 public:
  ImageData(std::string filename);
  ImageData(float*d, int h, int w, int c);
  ImageData(ImageData* i);
  ~ImageData();

  float* data() const;
  size_t elements() const;
  int width() const;
  int height() const;
  // only 3 or 4 is supported
  int channels() const;

  RGBQUAD operator()(int h, int w) const;

  void copyTo(ImageData *dst) const;


 private:

  std::string _filename;
  // typedef std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> FIBitmapPtr;
  typedef FIBITMAP* FIBitmapPtr;

  float *_raw_buf;
  FIBitmapPtr _data;
  int _height;
  int _width;
  int _channels;

  // rgb times height*width

  static bool is_ptr_Grey(FIBITMAP* data);
  static bool is_ptr_RGB(FIBITMAP* data);
};
}; // namespace Image

#endif // IMAGE_DATA_H