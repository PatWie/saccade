#ifndef MISC_H
#define MISC_H

#include <memory>
#include <QColor>

typedef unsigned int uint;

template< typename T >
struct always_false {
    enum { value = false };
};


enum class HistogramRefreshTarget {CURRENT, ENTIRE_CANVAS};

// color channels or gray channel
const static QColor misc_theme_red(241, 79, 76, 255);
const static QColor misc_theme_green(105, 213, 107, 255);
const static QColor misc_theme_blue(113, 158, 248, 255);
const static QColor misc_theme_gray(240, 240, 240, 255);

// theming
const static QColor misc_theme_yellow(251, 199, 99);
const static QColor misc_theme_gray_selected(39, 40, 34, 100);

#endif // MISC_H
