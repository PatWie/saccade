#include <glog/logging.h>

#include <QtGui>
#include <QMenu>

#include "histogram.h"
#include "../Utils/image_data.h"
#include "../Utils/histogram_data.h"


GUI::Histogram::Histogram(QWidget *parent)
  : _histogram(nullptr) {
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setMouseTracking( true );
  _dragging.mode = dragging_t::NONE;

  _resetRangeAct = new QAction(tr("Reset Range"), this);
  connect(_resetRangeAct, SIGNAL(triggered()),
          this, SLOT(slotResetRange()));
}

GUI::Histogram::~Histogram() {

}

void GUI::Histogram::contextMenuEvent(QContextMenuEvent *event) {
  QMenu menu(this);
  menu.addAction(_resetRangeAct);
  menu.exec(event->globalPos());
}

void GUI::Histogram::setData(Utils::HistogramData *h) {
  LOG(INFO) << "GUI::Histogram::setData";
  _histogram = h;
}

const Utils::HistogramData* GUI::Histogram::data() {
  return _histogram;
}

QSize GUI::Histogram::sizeHint () const {
  return QSize( 300, 30 );
}

void GUI::Histogram::paintEvent( QPaintEvent *e) {
  Q_UNUSED(e);

  const int min_width = 50;
  const QColor selectionColor = QColor(87, 87, 77, 100);

  QPainter p(this);
  QRect paint_rectangle = paintingArea();

  if (paint_rectangle.width() < min_width)
    return;

  // draw histogram
  if (hasHistogram()) {

    // select colors according image format
    std::vector<QColor> channel_colors;
    if (_histogram->channels() > 1) {
      // rgb, rgba
      channel_colors.push_back(QColor(245, 14, 0, 255));
      channel_colors.push_back(QColor(21, 227, 0, 255));
      channel_colors.push_back(QColor(114, 159, 207, 255));
      channel_colors.push_back(QColor(240, 240, 240, 255));
    } else {
      // grayscale
      channel_colors.push_back(QColor(240, 240, 240, 255));
    }

    const double scale = paint_rectangle.height();
    const int offset = 5;
    for (int b = 0; b < _histogram->bins(); ++b) {
      for (int c = _histogram->channels() - 1; c >= 0; --c) {
        const int amount = _histogram->amount(c, b);
        if (amount > 0) {
          const int barSize = (scale * amount) / _histogram->bin_info().max;
          p.setPen(channel_colors[c]);
          p.drawLine(offset + b, paint_rectangle.bottom(),
                     offset + b, paint_rectangle.bottom() - barSize);
        }
      }
    }

    // draw selection
    const int x1 = _histogram->range()->min + paint_rectangle.left();
    const int x2 = _histogram->range()->max + paint_rectangle.left();

    p.fillRect(x1, paint_rectangle.top(),
               x2 - x1,
               paint_rectangle.height(),
               QBrush(selectionColor));

  }



}

QRect GUI::Histogram::paintingArea() const {
  QRect paint_rectangle = frameRect();
  paint_rectangle.setLeft( paint_rectangle.left() + 1 );
  paint_rectangle.setTop( paint_rectangle.top() + 1 );
  paint_rectangle.setBottom( paint_rectangle.bottom() - 1 );
  paint_rectangle.setRight( paint_rectangle.right() - 1 );
  return paint_rectangle;
}

/**
 * @brief helper to avoid segfaults
 */
bool GUI::Histogram::hasHistogram() const {
  if (_histogram != nullptr) {
    if (_histogram->available()) {
      return true;
    }
  }
  return false;
}

void GUI::Histogram::mousePressEvent(QMouseEvent * e) {
  if (hasHistogram()) {
    // user wants to apply some action
    if ( (e->buttons() & Qt::LeftButton) ) {
      // get entire area
      QRect area = paintingArea();
      const int x = e->x();

      // identify which part the user wants to drag
      const int dist_left = abs(x - _histogram->range()->min);
      const int dist_right = abs(x - _histogram->range()->max);

      // user wants to drag the limits of the range
      if ((dist_left < 5) || (dist_right < 5)) {
        if (dist_left < dist_right) {
          _dragging.mode = dragging_t::LEFT;
        } else {
          _dragging.mode = dragging_t::RIGHT;
        }
      } else {
        // user wants to shift entire range
        if (_histogram->range()->min < x && x < _histogram->range()->max) {
          _dragging.mode = dragging_t::SHIFT;
          _dragging.start_x = x;
          _dragging.start_range = *(_histogram->range());
        }
      }
    }
  }
}

void GUI::Histogram::mouseMoveEvent(QMouseEvent * e) {
  const int x = e->x();
  const QRect area = paintingArea();
  if (hasHistogram()) {

    if (_dragging.mode == dragging_t::NONE) {
      const int dist_left = abs(x - _histogram->range()->min);
      const int dist_right = abs(x - _histogram->range()->max);

      if ((dist_left < 5) || (dist_right < 5)) {
        QCursor tmp;
        tmp.setShape(Qt::SplitHCursor);
        this->setCursor(tmp);
      } else {
        if (_histogram->range()->min < x && x < _histogram->range()->max) {
          QCursor tmp;
          tmp.setShape(Qt::PointingHandCursor);
          this->setCursor(tmp);
        } else {
          QCursor tmp;
          tmp.setShape(Qt::ArrowCursor);
          this->setCursor(tmp);
        }

      }
    } else if (_dragging.mode == dragging_t::SHIFT) {
      _histogram->range()->min = std::max((float) area.left(), _dragging.start_range.min + (x - _dragging.start_x));
      _histogram->range()->max = std::min((float) area.right(), _dragging.start_range.max + (x - _dragging.start_x));
      update();
    } else {
      if (_dragging.mode == dragging_t::LEFT) {
        _histogram->range()->min = std::min((float) std::max(e->x(), area.left()), _histogram->range()->max - 1);
      }
      if (_dragging.mode == dragging_t::RIGHT) {
        _histogram->range()->max = std::max((float) std::min(e->x(), area.right()), _histogram->range()->min + 1);
      }
      update();
    }
  }
}

void GUI::Histogram::mouseReleaseEvent(QMouseEvent * e) {
  if (hasHistogram()) {
    QCursor tmp;
    tmp.setShape(Qt::ArrowCursor);
    this->setCursor(tmp);
    if (_dragging.mode != dragging_t::NONE) {
      emit sigRefreshBuffer();
      _dragging.mode = dragging_t::NONE;
    }
  }
}

void GUI::Histogram::slotResetRange(){
  _histogram->range()->min = 0.;
  _histogram->range()->max = _histogram->image()->max();
  emit sigRefreshBuffer();
}