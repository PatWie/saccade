#ifndef LAYER_H
#define LAYER_H

#include <QtGui>
// #include <QObject>
#include <string>

namespace Utils {
class Mipmap;
class ImageData;
class GlManager;
namespace Ops {
class ImgOp;
} // namespace Ops
}; // namespace Utils

namespace GUI {
class ImageWindow;

namespace threads {
/**
 * @brief create Mipmap data structure from image file
 */
class MipmapThread : public QThread {
 public:
  MipmapThread();
  void notify( Utils::Mipmap* mipmap,  Utils::ImageData *img);
  void run();
 private:
  Utils::Mipmap *_mipmap;
  Utils::ImageData *_img;
};

/**
 * @brief apply a list of operations to image
 */
class OperationThread : public QThread {
 public:
  OperationThread();
  void notify(Utils::ImageData *dst, Utils::ImageData *src, Utils::Ops::ImgOp *op);
  void run();
 private:
  Utils::Ops::ImgOp *_op;
  Utils::ImageData *_dst;
  Utils::ImageData *_src;
};

} // namespace threads


class Layer  : public QObject {
  Q_OBJECT

 public:
  Layer();
  ~Layer();

  size_t width() const;
  size_t height() const;

  void draw(Utils::GlManager *gl,
            uint top, uint left,
            uint bottom, uint right,
            double zoom);

  void loadImage(std::string fn);
  bool available() const;
  std::string path() const;


 signals:
  void sigRefresh();
  void sigApplyOpFinished();

 protected:

 public slots:
  void slotRebuildMipmap();
  void slotLoadFinished();
  void slotApplyOpFinished();
  void slotApplyOp(Utils::Ops::ImgOp*);
 private slots:

 private:

  std::string _path;

  // the image itself
  Utils::ImageData *_imgdata;
  // any modification to the image (gamma correction, range slider)
  Utils::ImageData *_bufdata;
  // mipmap datastructure of _bufdata
  Utils::Mipmap *_mipmap;

  bool _available;

  threads::MipmapThread *_thread_mipmapBuilder;
  threads::OperationThread *_thread_opWorker;

};
}; // namespace GUI

#endif // LAYER_H