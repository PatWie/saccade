#ifndef PYTHON_API_H
#define PYTHON_API_H

namespace Python {

class API {
 public:
  static API& getInstance();

  API(API const&)             = delete;
  void operator=(API const&)  = delete;
 private:
  API();


};
}; // namespace Python

#endif // PYTHON_API_H