#ifndef LAYER_H
#define LAYER_H


#include <memory>
#include <QtGui>
#include <QFileSystemWatcher>
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

typedef std::shared_ptr<Utils::ImageData> ImageData_ptr;
typedef std::shared_ptr<Utils::Mipmap> Mipmap_ptr;
class ImageWindow;

namespace threads {
/**
 * @brief create Mipmap data structure from image file
 */
class MipmapThread : public QThread {
 public:
  MipmapThread();
  void notify( Mipmap_ptr mipmap,  ImageData_ptr img);
  void run();
 private:
  Mipmap_ptr _mipmap;
  ImageData_ptr _img;
};

/**
 * @brief apply a list of operations to image
 */
class OperationThread : public QThread {
 public:
  OperationThread();
  void notify(ImageData_ptr dst, ImageData_ptr src, Utils::Ops::ImgOp *op);
  void run();
 private:
  Utils::Ops::ImgOp *_op;
  ImageData_ptr _dst;
  ImageData_ptr _src;
};

/**
 * @brief triggers loadImage when fileformat is not corrupted
 * @details QFileWatcher is triggered when file changes. But there is no guarantee
 *          that the writing process completed its operation
 */
class ReloadThread : public QThread {
  Q_OBJECT
 public:
  ReloadThread();
  void notify(std::string fn, int attempts = 3);
  void run();
 private:
  std::string _fn;
  int _attempts;
 signals:
  void sigFileIsValid(QString);

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
  void clear();
  bool available() const;
  std::string path() const;


 signals:
  void sigRefresh();
  void sigApplyOpFinished();

 protected:

 public slots:
  void slotRebuildMipmap();
  void slotMipmapFinished();
  // void slotApplyOpFinished();
  // void slotApplyOp(Utils::Ops::ImgOp*);
  void slotFileIsValid(QString);

 protected slots:
  void slotPathChanged(QString);
 private slots:

 private:

  QFileSystemWatcher* _watcher;

  std::string _path;

  // the image itself
  ImageData_ptr _imgdata;
  // any modification to the image (gamma correction, range slider)
  ImageData_ptr _bufdata;
  // mipmap datastructure of _bufdata
  Mipmap_ptr _working_mipmap;
  Mipmap_ptr _current_mipmap;

  bool _available;

  threads::MipmapThread *_thread_mipmapBuilder;
  threads::OperationThread *_thread_opWorker;
  threads::ReloadThread *_thread_Reloader;

};
}; // namespace GUI

#endif // LAYER_H