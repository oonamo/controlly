#ifndef _VEC_H
#define _VEC_H

#include "arena.h"

typedef struct {
  float* coeffs;
  size_t size;
  size_t capacity;
} Vec;

#endif
