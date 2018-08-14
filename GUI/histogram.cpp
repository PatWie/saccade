#include <glog/logging.h>

#include <QtGui>
#include <QMenu>

#include "histogram.h"
#include "../Utils/misc.h"
#include "../Utils/image_data.h"
#include "../Utils/histogram_data.h"


GUI::Histogram::Histogram(QWidget *parent)
  : _histogram(nullptr), _min_width(50) {

  Q_UNUSED(parent);

  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setMouseTracking( true );

  _dragging.mode = dragging_t::NONE;

  _resetRangeResetAct = new QAction(tr("Reset Range"), this);
  _resetRangeLdrAct = new QAction(tr("Fit LDR"), this);
  _resetRange01Act = new QAction(tr("Fit [0,1]"), this);
  _resetRangeMinMaxAct = new QAction(tr("Fit [0,max]"), this);
  connect(_resetRangeResetAct, &QAction::triggered, this, [this] { slotSetRange(0, 0, HistogramRefreshTarget::CURRENT); });
  connect(_resetRangeLdrAct, &QAction::triggered, this, [this] { slotSetRange(0, 255, HistogramRefreshTarget::CURRENT); });
  connect(_resetRange01Act, &QAction::triggered, this, [this] { slotSetRange(0, 1, HistogramRefreshTarget::CURRENT); });
  connect(_resetRangeMinMaxAct, &QAction::triggered, this, [this] {
    slotSetRange(_histogram->range_used()->min, _histogram->range_used()->max, HistogramRefreshTarget::CURRENT);
  });

  _setMappingLinearAct = new QAction(tr("Set linear mapping"), this);
  _setMappingLogAct = new QAction(tr("Set log mapping"), this);
  connect(_setMappingLinearAct,  &QAction::triggered, this, [this] { slotSetMappingMode(0); });
  connect(_setMappingLogAct,  &QAction::triggered, this, [this] { slotSetMappingMode(1); });


  // we want to have a nice additive blending of the colors
  _composition_mode = QPainter::CompositionMode_Plus;
  // width of the histogram widget
  _expected_width = 256;
  // allow some margin on left and right (this is the value foreach / not the sum)
  _margin = 4;

}

GUI::Histogram::~Histogram() {

}

void GUI::Histogram::contextMenuEvent(QContextMenuEvent *event) {
  if (hasHistogram()) {
    QMenu menu(this);
    menu.addAction(_resetRangeResetAct);
    menu.addAction(_resetRangeLdrAct);
    menu.addAction(_resetRange01Act);
    menu.addAction(_resetRangeMinMaxAct);
    menu.addSeparator();
    menu.addAction(_setMappingLinearAct);
    menu.addAction(_setMappingLogAct);
    menu.exec(event->globalPos());
  }
}

void GUI::Histogram::setData(Utils::HistogramData *h) {
  DLOG(INFO) << "GUI::Histogram::setData";
  _histogram = h;
  if (_histogram != nullptr)
    _expected_width = _histogram->bins();
  update();
}

const Utils::HistogramData* GUI::Histogram::data() {
  return _histogram;
}

QSize GUI::Histogram::sizeHint () const {
  return QSize(_expected_width + 2 * _margin, 30);
}

void GUI::Histogram::paintEvent( QPaintEvent *e) {
  Q_UNUSED(e);

  const QColor selectionColor = misc_theme_gray_selected;
  const QColor delimiterColor = misc_theme_yellow;

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.setCompositionMode(_composition_mode);
  QRect paint_rectangle = paintingArea();

  // area is too small (makes no sense to draw widget)
  if (paint_rectangle.width() < _min_width)
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


    CHECK(_histogram->channels() == 1 || _histogram->channels() == 3)
        << "current histogram support is for 1 or 3 channels";

    // draw the actual bars
    for (int b = 0; b < _histogram->bins(); ++b) {
      for (int c = 0; c < _histogram->channels(); ++c) {
        const int amount = _histogram->amount(c, b);
        if (amount > 0) {
          const int barSize = scale * scale_func((double)amount) / scale_func((double)_histogram->bin_info().max);
          p.setPen(channel_colors[c]);
          p.drawLine(b + _margin, paint_rectangle.bottom(),
                     b + _margin, paint_rectangle.bottom() - barSize);
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
    p.fillRect(x1, paint_rectangle.top(), 1, scale, QBrush(delimiterColor));
    p.fillRect(x2, paint_rectangle.top(), 1, scale, QBrush(delimiterColor));

  }
}

QRect GUI::Histogram::paintingArea() const {
  QRect paint_rectangle = frameRect();
  paint_rectangle.setLeft( paint_rectangle.left() + _margin);
  paint_rectangle.setTop( paint_rectangle.top() + 1 );
  paint_rectangle.setBottom( paint_rectangle.bottom() - 1 );
  paint_rectangle.setRight( paint_rectangle.right() - _margin);
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
  const int delimiter_sensibility = 5;
  if (hasHistogram()) {
    // user wants to apply some action
    if ( (e->buttons() & Qt::LeftButton) ) {
      // get entire area
      // QRect area = paintingArea();
      const int x = e->x();

      // identify which part the user wants to drag
      const int dist_left = fabs(x - _margin - _histogram->range()->min);
      const int dist_right = fabs(x - _margin - _histogram->range()->max);

      // user wants to drag the limits of the range
      if ((dist_left < delimiter_sensibility) || (dist_right < delimiter_sensibility)) {
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
      const int dist_left = fabs(x - _margin - _histogram->range()->min);
      const int dist_right = fabs(x - _margin - _histogram->range()->max);

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
      _histogram->range()->min = std::max((float) area.left(), _dragging.start_range.min + (x - _dragging.start_x)) - _margin;
      _histogram->range()->max = std::min((float) area.right(), _dragging.start_range.max + (x - _dragging.start_x)) - _margin + 1;
      update();
    } else {
      // only shift a range-delimiter
      if (_dragging.mode == dragging_t::LEFT) {
        _histogram->range()->min = std::min((float) std::max(e->x(), area.left()), _histogram->range()->max - 1) - _margin;
      }
      if (_dragging.mode == dragging_t::RIGHT) {
        _histogram->range()->max = std::max((float) std::min(e->x(), area.right()), _histogram->range()->min + 1) - _margin + 1;
      }
      update();
    }
  }
}

void GUI::Histogram::mouseReleaseEvent(QMouseEvent * e) {
  Q_UNUSED(e);
  if (hasHistogram()) {
    // stop clicking means reset cursor style
    QCursor tmp;
    tmp.setShape(Qt::ArrowCursor);
    this->setCursor(tmp);
    // if there was an action, this action will now take effect
    if (_dragging.mode != dragging_t::NONE) {

      Qt::KeyboardModifiers keymod = QGuiApplication::keyboardModifiers();
      if (keymod == Qt::ShiftModifier) {
        emit sigRefreshBuffer(HistogramRefreshTarget::ENTIRE_CANVAS);
      } else {
        emit sigRefreshBuffer(HistogramRefreshTarget::CURRENT);
      }

      _dragging.mode = dragging_t::NONE;
    }
  }
}

void GUI::Histogram::slotResetRange(HistogramRefreshTarget t) {
  slotSetRange(0, 0, t);
}

void GUI::Histogram::slotSetRange(float min, float max, HistogramRefreshTarget t) {
  if (hasHistogram()) {
    if (min == 0 && max == 0) {
      // by 0,0 we encode reset
      _histogram->range()->min = 0.;
      _histogram->range()->max = _histogram->image()->max();
    } else {
      _histogram->range()->min = min;
      _histogram->range()->max = max;
    }
    DLOG(INFO) << "_histogram->range()->min " << _histogram->range()->min;
    DLOG(INFO) << "_histogram->range()->max " << _histogram->range()->max;
    emit sigRefreshBuffer(t);
    update();
  }
}

float GUI::Histogram::scale_func(float k) const {
  return _histogram->scale_func(k);
}

void GUI::Histogram::slotSetMappingMode(int i) {
  _histogram->setScale(i);
}