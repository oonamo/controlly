#ifndef _MATRIX_H
#define _MATRIX_H
#include "arena.h"

typedef struct {
  float* coeffs;
  size_t size;
  size_t capacity;
} vector_t;

typedef struct
{
    float *rows;
    float *cols;
    size_t n_rows;
    size_t n_cols;
} matrix_t;

#endif
