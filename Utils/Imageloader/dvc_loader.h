#ifndef DVC_LOADER_H
#define DVC_LOADER_H

#include "image_loader.h"

namespace Utils
{
  namespace Loader
  {
    /**
     * @brief loading all filetypes that FreeImage library can handle
     */
    class DVCLoader : public ImageLoader
    {
    public:
      DVCLoader();
      ~DVCLoader();

      /**
       * @brief test if FreeImage knows this format
       */
      bool canLoad(std::string fn);
      float* load(std::string fn, int *h, int *w, int *_channels, float *_max_value) ;
      
    };
  }; // namespace Loader
}; // namespace Utils

#endif // DVC_LOADER_H