#ifndef HYPERTHYMESIA_ALLOC_FRAME_H
#define HYPERTHYMESIA_ALLOC_FRAME_H

#include "ht_bt.h"

typedef struct {
    ht_backtrace_t alloc_bt;
} ht_alloc_header_t;

#endif // HYPERTHYMESIA_ALLOC_FRAME_H
