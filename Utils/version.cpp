#include "version.h"

namespace Utils {

std::string GetVersionInfo() {
  return std::string("EagleEye (") + std::string(EAGLEEYE_VERSION.c_str()) + ")";
}

std::string GetBuildInfo() {
  return std::string(EAGLEEYE_BRANCH.c_str()) +
         std::string(" (") +
         std::string(EAGLEEYE_COMMIT_ID.c_str()) +
         std::string(") on ") +
         std::string(EAGLEEYE_COMMIT_DATE.c_str());
}

}  // namespace Utils
