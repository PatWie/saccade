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
  /**
   * @brief Get precomputed underlying histogram data
   * @return underlying histogram data
   */
  const Utils::HistogramData* data();

 protected:
  void paintEvent(QPaintEvent *e);

  /**
   * @brief create menu with range and scaling settings
   */
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
  /**
   * @brief set limits of range
   * @param min left delimiter
   * @param max right delimiter
   */
  void slotSetRange(float, float);

  /**
     * @brief reset limits of range to cover the full spectrum
     */
  void slotResetRange();

  /**
   * @brief change mapping mode (linear/log)
   * @param  mode (0=log, 1=linear)
   */
  void slotSetMappingMode(int);

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

  // total width of the painting area
  int _expected_width;
  // margin on left and margin on right (not the sum)
  int _margin;
  // minimal expected width, otherwise we do not paint anything
  const int _min_width;

  QAction* _resetRangeResetAct;
  QAction* _resetRangeLdrAct;
  QAction* _resetRange01Act;

  QAction* _setMappingLinearAct;
  QAction* _setMappingLogAct;

  QPainter::CompositionMode _composition_mode;


  float scale_func(float) const;
};
}; // namespace GUI

#endif // HISTOGRAM_H