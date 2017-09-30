#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QtGui>
#include <QFrame>
#include <QAction>

#include "../Utils/histogram_data.h"

// namespace Utils {
// class HistogramData;
// }; // namespace Utils

namespace GUI {
  namespace hist_internal{
    template<typename T>
    inline T scale_func(T i){
      return i;
      // return log(i);
    }
  };
/**
 * @brief Represent color histogram of an image
 */
class Histogram : public QFrame {
  Q_OBJECT
 public:
  Histogram(QWidget *parent);
  ~Histogram();

  QSize sizeHint () const;

  /**
   * @brief Set precomputed histogram data;
   *
   * @param h histogram data
   */
  void setData(Utils::HistogramData *h);
  const Utils::HistogramData* data();

 protected:
  void paintEvent(QPaintEvent *e);

  void contextMenuEvent(QContextMenuEvent *event);

  void mousePressEvent(QMouseEvent * e);
  void mouseMoveEvent(QMouseEvent * e);
  void mouseReleaseEvent(QMouseEvent * e);

  /**
   * @brief Return a description of painting area.
   */
  QRect paintingArea() const;

  /**
 * @brief helper to avoid segfaults
 */
  bool hasHistogram() const;

 signals:
  // histogram-range changed -> request buffer change
  void sigRefreshBuffer();

 public slots:
 // reset range to default of image
  void slotResetRange();

 private:
  // pointer to underlying data structure
  Utils::HistogramData *_histogram;

  // dragging information
  struct dragging_t {
    // drag either left, right of range or shift the range, or nothing
    enum mode_t {
      LEFT, RIGHT, SHIFT, NONE
    };
    // horizontal position of mouse when dragging started
    int start_x;
    // range information when dragging started
    Utils::HistogramData::range_t start_range;
    // current dragging mode
    mode_t mode;

  } _dragging;

  int _expected_width;
  int _margin;

  QAction* _resetRangeAct;

  QPainter::CompositionMode _composition_mode;
};
}; // namespace GUI

#endif // HISTOGRAM_H