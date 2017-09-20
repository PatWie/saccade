#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <FreeImage.h>
#include <memory>
#include <string>
#include <vector>



namespace Utils {
class ImageData {

 public:
  ImageData(std::string filename);
  ~ImageData();

  float* data() const;
  int width() const;
  int height() const;
  // only 3 or 4 is supported
  int channels() const;

  RGBQUAD operator()(int h, int w) const;


 private:

  std::string _filename;
  // typedef std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> FIBitmapPtr;
  typedef FIBITMAP* FIBitmapPtr;

  float *_raw_buf;
  FIBitmapPtr _data;
  int _width;
  int _height;
  int _channels;

  // rgb times height*width

  static bool is_ptr_Grey(FIBITMAP* data);
  static bool is_ptr_RGB(FIBITMAP* data);
};
}; // namespace Image

#endif // IMAGE_DATA_H