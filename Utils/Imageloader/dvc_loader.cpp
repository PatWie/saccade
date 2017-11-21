
#include "dvc_loader.h"
#include <glog/logging.h>
#include <cmath>
#include <string>

namespace Utils {
namespace Loader {

DVCLoader::DVCLoader() {}
DVCLoader::~DVCLoader() {}

namespace {
inline bool ends_with(std::string const & value, std::string const & ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
}; // namespace

bool DVCLoader::canLoad(std::string fn) {
  return (ends_with(fn, ".dvc") || ends_with(fn, ".dvc.gz"));
}


float* DVCLoader::load(std::string fn, int *_height, int *_width, int *_channels, float *_max_value)  {}
  // TODO

}; // namespace Loader
}; // namespace Utils