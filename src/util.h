#pragma once

#define LEN(X) (sizeof X / sizeof X[0])

typedef struct {
  unsigned x;
  unsigned y;
} pair_t;
