#pragma once

typedef struct {
  long DATA[3];
  void (*evfn)(const long, const long, const long);
} ev_t;

