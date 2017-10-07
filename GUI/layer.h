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
class HistogramData;
class GlManager;


namespace Ops {
class ImgOp;
} // namespace Ops
}; // namespace Utils

namespace GUI {

typedef std::shared_ptr<Utils::ImageData> ImageData_ptr;
typedef std::shared_ptr<Utils::HistogramData> HistogramData_ptr;
typedef std::shared_ptr<Utils::Mipmap> Mipmap_ptr;
class ImageWindow;

namespace threads {
/**
 * @brief create Mipmap data structure from image file
 */
class MipmapThread : public QThread {
 public:
  MipmapThread();
  void notify(Mipmap_ptr mipmap,  ImageData_ptr img);
  void run();
 private:
  Mipmap_ptr _mipmap;
  ImageData_ptr _img;
};

/**
 * @brief Compute image histogram.
 */
class HistogramThread : public QThread {
 public:
  HistogramThread();
  void notify(const ImageData_ptr img,  HistogramData_ptr hist);
  void run();
 private:
  ImageData_ptr _img;
  HistogramData_ptr _hist;
};

/**
 * @brief Apply an operations to image.
 * @details Can be scaling and clipping according to histogram limits.
 */
class OperationThread : public QThread {
 public:
  OperationThread();
  void notify(const ImageData_ptr dst, ImageData_ptr src, Utils::Ops::ImgOp *op);
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
 * @todo this is currently an ugly workaround
 *          
 *          see: https://codereview.qt-project.org/#/c/19274/
 *          see: https://bugreports.qt.io/browse/QTBUG-2264
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

  /**
   * @brief width of image
   */
  size_t width() const;

  /**
   * @brief height of image
   */
  size_t height() const;

  /**
   * @brief pointer to underlying image data-struct
   */
  const Utils::ImageData* img() const;

  /**
   * @brief Draw image content with OpenGL
   * @details leverages the mip-mapping datastructure
   * 
   * @param gl wrapper for OpenGL
   * @param top coordinate top of visible area
   * @param left coordinate left of visible area
   * @param bottom coordinate bottom of visible area
   * @param right coordinate right of visible area
   * @param zoom pixel size
   */
  void draw(Utils::GlManager *gl,
            uint top, uint left,
            uint bottom, uint right,
            double zoom);

  /**
   * @brief override image data with image from given file
   * @details might destroy old data
   * 
   * @param fn path to new image
   */
  void loadImage(std::string fn);

  /**
   * @brief delete all data
   * @details cleares image data, buffer data and mipmap
   */
  void clear();

  /**
   * @brief indicates whether there are pending operations on the buffer
   * @details might be false during scaling due to histogram-widget or reloading
   * @return operation is pending and it is unsafe to use the data pointer
   */
  bool available() const;

  /**
   * @brief path to file where image was read from
   * @return image path
   */
  std::string path() const;

  /**
   * @brief Pointer to underlying histogram information
   * @return pointer to histogram data struct
   */
  Utils::HistogramData* histogram() const;


 signals:
  void sigRefresh();
  void sigApplyOpFinished();
  void sigHistogramFinished();

 protected:

 public slots:
  void slotRebuildMipmap();
  void slotRebuildHistogram();
  void slotMipmapFinished();
  void slotHistogramFinished();
  void slotApplyOpFinished();
  void slotApplyOp(Utils::Ops::ImgOp*);
  void slotFileIsValid(QString);
  void slotRefresh(float, float);

 protected slots:
  void slotPathChanged(QString);

 private slots:

 private:

  QFileSystemWatcher* _watcher;

  std::string _path;

  Utils::Ops::ImgOp* _op;

  // the image itself
  ImageData_ptr _imgdata;
  // and its histogram
  HistogramData_ptr _histdata;
  // any modification to the image (gamma correction, range slider)
  ImageData_ptr _bufdata;
  // mipmap datastructure of _bufdata
  Mipmap_ptr _working_mipmap;
  Mipmap_ptr _current_mipmap;

  bool _available;

  threads::MipmapThread *_thread_mipmapBuilder;
  threads::OperationThread *_thread_opWorker;
  threads::HistogramThread *_thread_histogram;
  threads::ReloadThread *_thread_Reloader;

};
}; // namespace GUI

#endif // LAYER_H