#ifndef OPTICALFLOW_LOADER_H
#define OPTICALFLOW_LOADER_H

#include "image_loader.h"

namespace Utils
{
  namespace Loader
  {
    /**
     * @brief loading all filetypes that OpticalFlow library can handle
     */
    class OpticalFlowLoader : public ImageLoader
    {

      uint8_t colorWheel[55 * 3];
    public:
      OpticalFlowLoader();
      ~OpticalFlowLoader();

      /**
       * @brief test if OpticalFlow knows this format
       */
      bool canLoad(std::string fn);
      float* load(std::string fn, int *h, int *w, int *_channels, float *_max_value) ;

    };
  }; // namespace Loader
}; // namespace Utils

#endif // OPTICALFLOW_LOADER_H