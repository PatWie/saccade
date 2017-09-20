#ifndef LAYER_H
#define LAYER_H

#include <QtGui>
#include <string>

namespace Utils {
class Mipmap;
class ImageData;
class GlManager;
}; // namespace Utils

namespace GUI {
class ImageWindow;


class Layer{
  // Q_OBJECT

 public:
  Layer();

  size_t width() const;
  size_t height() const;

  void draw(Utils::GlManager *gl,
            uint top, uint left,
            uint bottom, uint right,
            double zoom);

  void loadImage(std::string fn);
  bool hasImage() const;

  std::string path() const;

 protected:

 private slots:

 private:

  std::string _path;

  Utils::ImageData *_imgdata;
  Utils::Mipmap *_mipmap;

};
}; // namespace GUI

#endif // LAYER_H