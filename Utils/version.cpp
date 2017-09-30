#include "version.h"

namespace Utils {

std::string GetVersionInfo() {
  return std::string("Saccade (") + std::string(SACCADE_VERSION.c_str()) + ")";
}

std::string GetBuildInfo() {
  return std::string(SACCADE_BRANCH.c_str()) +
         std::string(" (") +
         std::string(SACCADE_COMMIT_ID.c_str()) +
         std::string(") on ") +
         std::string(SACCADE_COMMIT_DATE.c_str());
}

}  // namespace Utils
