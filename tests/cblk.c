#include <stdio.h>

#include "../src/cblk.h"

static cblk_t cblk;

int main(int const argc, char const* argv[]) {
  cblk = cblk_init(sizeof(int), 10);
  for (int i = 0; i < 10; i++) {
    cblk_map(&cblk, &i);
  }

  for (int i = 0; i < cblk.size; i++) {
    void* itr = cblk_itr(&cblk, i);
    fprintf(stdout, "%d ", *((int*) itr));
  }

  fprintf(stdout, "\n");

  return 0;
}
