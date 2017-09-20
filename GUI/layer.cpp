#include "../Utils/image_data.h"
#include "../Utils/mipmap.h"
#include "../Utils/gl_manager.h"
#include "layer.h"
#include <iostream>
#include <string>



GUI::Layer::Layer() {
  _path = "";
  std::cout << "GUI::Layer::Layer()" << std::endl;
}

void GUI::Layer::draw(Utils::GlManager *gl,
                      uint top, uint left,
                      uint bottom, uint right,
                      double zoom) {

  _mipmap->draw(gl, top, left, bottom, right, zoom);

}

size_t GUI::Layer::width() const {
  return hasImage() ? _imgdata->width() : 0 ;
}
size_t GUI::Layer::height() const {
  return hasImage() ? _imgdata->height() : 0 ;
}

bool GUI::Layer::hasImage() const {
  return _path != "";
}
void GUI::Layer::loadImage(std::string fn) {
  _path = fn;
  _imgdata = new Utils::ImageData(fn);
  _mipmap = new Utils::Mipmap();
  _mipmap->setData(_imgdata->data(),
                   _imgdata->height(), _imgdata->width(), _imgdata->channels());
}

std::string GUI::Layer::path() const {
  return _path;
}