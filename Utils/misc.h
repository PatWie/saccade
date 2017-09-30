#ifndef MISC_H
#define MISC_H

#include <memory>
#include <QColor>

typedef unsigned int uint;

template< typename T >
struct always_false { 
    enum { value = false };  
};

const static QColor misc_theme_red(241, 79, 76, 255);
const static QColor misc_theme_green(105, 213, 107, 255);
const static QColor misc_theme_blue(113, 158, 248, 255);
const static QColor misc_theme_gray(240, 240, 240, 255);

#endif // MISC_H