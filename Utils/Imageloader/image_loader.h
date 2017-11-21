#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <string>

namespace Utils
{
  namespace Loader
  {
    class ImageLoader
    {
    public:
      /**
       * @brief should return wether this file can be loaded by this loading-class
       * @details inspecting of the image can be loaded by this particular loader
       * 
       * @param fn path to image file
       * @return true/false
       */
      virtual bool canLoad(std::string fn) = 0;
      /**
       * @brief should load image from file
       * @details image data is stored unscaled in a float array [C,H,W]
       * 
       * @param fn path to image file
       * @param h height of image
       * @param w width of image
       * @param _channels channels of image
       * @param _max_value maximum possible intensity value (used for rescaled during OpenGL rendering)
       * @return float-array containing the image data
       */
      virtual float* load(std::string fn, int *h, int *w, int *_channels, float *_max_value) = 0;
      
    };
  }; // namespace Loader
}; // namespace Utils

#endif // IMAGE_LOADER_H