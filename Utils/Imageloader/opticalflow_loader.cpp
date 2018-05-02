
#include "opticalflow_loader.h"
#include <glog/logging.h>
#include <cmath>
#include <string>


namespace Utils {
namespace Loader {
OpticalFlowLoader::OpticalFlowLoader() {

  int k = 0;
  const int RY = 15;
  const int YG = 6;
  const int GC = 4;
  const int CB = 11;
  const int BM = 13;
  const int MR = 6;
  // const int NCOLS = RY + YG + GC + CB + BM + MR;


  for (int i = 0; i < RY; ++i, ++k) {
    // colorWheel[k] = cv::Vec3i(255, 255 * i / RY, 0);
    colorWheel[k * 3 + 0] = 255;
    colorWheel[k * 3 + 1] = 255 * i / RY;
    colorWheel[k * 3 + 2] = 0;
  }

  for (int i = 0; i < YG; ++i, ++k) {
    // colorWheel[k] = cv::Vec3i(255 - 255 * i / YG, 255, 0);
    colorWheel[k * 3 + 0] = 255 - 255 * i / YG;
    colorWheel[k * 3 + 1] = 255;
    colorWheel[k * 3 + 2] = 0;
  }

  for (int i = 0; i < GC; ++i, ++k) {
    // colorWheel[k] = cv::Vec3i(0, 255, 255 * i / GC);
    colorWheel[k * 3 + 0] = 0;
    colorWheel[k * 3 + 1] = 255;
    colorWheel[k * 3 + 2] = 255 * i / GC;
  }

  for (int i = 0; i < CB; ++i, ++k) {
    // colorWheel[k] = cv::Vec3i(0, 255 - 255 * i / CB, 255);
    colorWheel[k * 3 + 0] = 0;
    colorWheel[k * 3 + 1] = 255 - 255 * i / CB;
    colorWheel[k * 3 + 2] = 255;
  }

  for (int i = 0; i < BM; ++i, ++k) {
    // colorWheel[k] = cv::Vec3i(255 * i / BM, 0, 255);
    colorWheel[k * 3 + 0] = 255 * i / BM;
    colorWheel[k * 3 + 1] = 0;
    colorWheel[k * 3 + 2] = 255;
  }

  for (int i = 0; i < MR; ++i, ++k) {
    // colorWheel[k] = cv::Vec3i(255, 0, 255 - 255 * i / MR);
    colorWheel[k * 3 + 0] = 255;
    colorWheel[k * 3 + 1] = 0;
    colorWheel[k * 3 + 2] = 255 - 255 * i / MR;
  }


}
OpticalFlowLoader::~OpticalFlowLoader() {

}

bool OpticalFlowLoader::canLoad(std::string fn) {
  return (fn.substr(fn.length() - 4) == ".flo");
}


float* OpticalFlowLoader::load(std::string fn, int *_height, int *_width, int *_channels, float *_max_value)  {

  FILE *stream = fopen(fn.c_str(), "rb");
  CHECK(stream != 0) << "cannot open flo file";

  int width, height;
  float tag;

  bool ret = (int)fread(&tag,    sizeof(float), 1, stream) != 1 ||
             (int)fread(&width,  sizeof(int),   1, stream) != 1 ||
             (int)fread(&height, sizeof(int),   1, stream) != 1;

  DLOG(INFO) << "height " << height;
  DLOG(INFO) << "width " << width;
  DLOG(INFO) << "tag " << tag;
  CHECK(ret != true) << "cannot read meta from flo file";

  *_width = width;
  *_height = height;
  *_channels = 3;
  *_max_value = 255;

  // read flow file
  // ---------------------------------------------------------------------------------------
  float* _motion_buf = new float[(*_height) * 2 * (*_width)];

  for (int h = 0; h < height; h++) {
    CHECK((int)fread(_motion_buf + h * 2 * width, sizeof(float), 2 * (*_width), stream) == 2 * (*_width));
  }


  const int RY = 15;
  const int YG = 6;
  const int GC = 4;
  const int CB = 11;
  const int BM = 13;
  const int MR = 6;
  const int NCOLS = RY + YG + GC + CB + BM + MR;


  float max_rad = 0;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      float fx = _motion_buf[ h * 2 * width + 2 * w + 0];
      float fy = _motion_buf[ h * 2 * width + 2 * w + 1];

      const float rad = sqrt(fx * fx + fy * fy);

      if (rad > max_rad) {
        max_rad = rad;
      }
    }
  }


  // convert to hsv image space
  // ---------------------------------------------------------------------------------------
  float* _raw_buf = new float[(*_channels) * (*_height) * (*_width)];
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {


      float fx = _motion_buf[ h * 2 * width + 2 * w + 0];
      float fy = _motion_buf[ h * 2 * width + 2 * w + 1];

      const float rad = sqrt(fx * fx + fy * fy);
      const float a = atan2(-fy, -fx) / (float) M_PI;
      const float fk = (a + 1.0f) / 2.0f * (NCOLS - 1);
      const int k0 = static_cast<int>(fk);
      const int k1 = (k0 + 1) % NCOLS;
      const float f = fk - k0;

      for (int c = 0; c < *_channels; ++c) {
        const float col0 = colorWheel[k0 * 3 + c] / 255.0f;
        const float col1 = colorWheel[k1 * 3 + c] / 255.0f;

        float col = (1 - f) * col0 + f * col1;
        col = 1 - (rad / max_rad) * (1 - col);

        _raw_buf[c * (height * width) + h * width + w] = col * 255.;
      }
    }
  }
  fclose(stream);
  return _raw_buf;
}


}; // namespace Loader
}; // namespace Utils