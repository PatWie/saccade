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
  int area() const;
  // only 3 or 4 is supported
  int channels() const;
  void clear(bool remove = true);

  float max() const;

  float value(int h, int w, int c) const;
  float value(int t, int c) const;
  float operator()(int h, int w, int c) const;

  void copyTo(ImageData *dst) const;

  static bool validFile(std::string filename);


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

};


}; // namespace Utils

#endif // IMAGE_DATA_H