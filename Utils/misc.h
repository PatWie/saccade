#ifndef MISC_H
#define MISC_H

#include <memory>

typedef unsigned int uint;

template< typename T >
struct always_false { 
    enum { value = false };  
};

#endif // MISC_H