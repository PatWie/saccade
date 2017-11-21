#ifndef FREEIMAGE_LOADER_H
#define FREEIMAGE_LOADER_H

#include "image_loader.h"

namespace Utils
{
  namespace Loader
  {
    /**
     * @brief loading all filetypes that FreeImage library can handle
     */
    class FreeImageLoader : public ImageLoader
    {
    public:
      FreeImageLoader();
      ~FreeImageLoader();

      /**
       * @brief test if FreeImage knows this format
       */
      bool canLoad(std::string fn);
      float* load(std::string fn, int *h, int *w, int *_channels, float *_max_value) ;
      
    };
  }; // namespace Loader
}; // namespace Utils

#endif // FREEIMAGE_LOADER_H