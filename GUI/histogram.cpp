#include <glog/logging.h>

#include <QtGui>
#include <QMenu>

#include "histogram.h"
#include "../Utils/misc.h"
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

  _composition_mode = QPainter::CompositionMode_Plus;
  _expected_width = 256;
  _margin = 4;
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
  _expected_width = _histogram->bins();
}

const Utils::HistogramData* GUI::Histogram::data() {
  return _histogram;
}

QSize GUI::Histogram::sizeHint () const {
  return QSize(_expected_width, 30);
}

void GUI::Histogram::paintEvent( QPaintEvent *e) {
  Q_UNUSED(e);

  const int min_width = 50;
  // const QColor selectionColor = QColor(87, 87, 77, 100);
  const QColor selectionColor = QColor(39, 40, 34, 100);
  const QColor delimiterColor = QColor(224, 186, 10, 170);

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.setCompositionMode(_composition_mode);
  QRect paint_rectangle = paintingArea();

  // area is too small (makes no sense to draw widget)
  if (paint_rectangle.width() < min_width)
    return;

  // draw histogram
  if (hasHistogram()) {

    // select colors according image format
    std::vector<QColor> channel_colors;
    if (_histogram->channels() > 1) {
      // rgb
      channel_colors.push_back(misc_theme_red);
      channel_colors.push_back(misc_theme_green);
      channel_colors.push_back(misc_theme_blue);
    } else {
      // grayscale
      channel_colors.push_back(misc_theme_gray);
    }

    const double scale = paint_rectangle.height();


    CHECK_LT(_histogram->channels(), 4) << "current histogram support is for 1 or 3 channels";

    // draw the actual bars
    for (int b = 0; b < _histogram->bins(); ++b) {
      for (int c = 0; c < _histogram->channels(); ++c) {
        const int amount = _histogram->amount(c, b);
        if (amount > 0) {
          const int barSize = scale * hist_internal::scale_func((double)amount) / hist_internal::scale_func((double)_histogram->bin_info().max);
          p.setPen(channel_colors[c]);
          p.drawLine(_margin + b, paint_rectangle.bottom(),
                     _margin + b, paint_rectangle.bottom() - barSize);
        }
      }
    }

    // draw range-selection
    const int x1 = _histogram->range()->min + paint_rectangle.left();
    const int x2 = _histogram->range()->max + paint_rectangle.left();

    p.fillRect(x1, paint_rectangle.top(),
               x2 - x1,
               paint_rectangle.height(),
               QBrush(selectionColor));

    // draw range delimiter
    // p.setPen(delimiterColor);
    // p.drawLine(x1, paint_rectangle.bottom(),
    //            x1, paint_rectangle.bottom() - scale);

    p.fillRect(x1 - 1, paint_rectangle.top(), 1, scale, QBrush(delimiterColor));
    p.fillRect(x2 - 1, paint_rectangle.top(), 1, scale, QBrush(delimiterColor));
    // p.drawLine(x2 - 1, paint_rectangle.bottom(),
    //            x2 + 1, paint_rectangle.bottom() - scale);
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
  if (hasHistogram()) {
    // if there is a histogram, we are interested in this event
    const int x = e->x();
    const QRect area = paintingArea();

    // mouse moves only (prepare for action and just change cursor style)
    if (_dragging.mode == dragging_t::NONE) {
      const int dist_left = abs(x - _histogram->range()->min);
      const int dist_right = abs(x - _histogram->range()->max);

      if ((dist_left < 5) || (dist_right < 5)) {
        // mouse hovers the range limits
        QCursor tmp;
        tmp.setShape(Qt::SplitHCursor);
        this->setCursor(tmp);
      } else {
        // is mouse somewhere between limits
        if (_histogram->range()->min < x && x < _histogram->range()->max) {
          QCursor tmp;
          tmp.setShape(Qt::PointingHandCursor);
          this->setCursor(tmp);
        } else {
          // reset cursor if none of the above cases occured
          QCursor tmp;
          tmp.setShape(Qt::ArrowCursor);
          this->setCursor(tmp);
        }

      }
    } else if (_dragging.mode == dragging_t::SHIFT) {
      // there is already an action in progress which moves the entire range
      _histogram->range()->min = std::max((float) area.left(), _dragging.start_range.min + (x - _dragging.start_x));
      _histogram->range()->max = std::min((float) area.right(), _dragging.start_range.max + (x - _dragging.start_x));
      update();
    } else {
      // only shift a range-delimiter
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
    // stop clicking means reset cursor styl
    QCursor tmp;
    tmp.setShape(Qt::ArrowCursor);
    this->setCursor(tmp);
    // if there was an action, this action will now take effect
    if (_dragging.mode != dragging_t::NONE) {
      emit sigRefreshBuffer();
      _dragging.mode = dragging_t::NONE;
    }
  }
}

void GUI::Histogram::slotResetRange() {
  if (hasHistogram()) {
    _histogram->range()->min = 0.;
    _histogram->range()->max = _histogram->image()->max();
    emit sigRefreshBuffer();
    update();
  }
}