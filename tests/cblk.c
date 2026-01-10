#include <stdio.h>

#include "../src/cblk.h"

static cblk_t cblk;

static void print_by_prev_val(void* org) {
  fprintf(stdout, "Running over prev values from arg: ");
  void *dev = org;
  do {
    fprintf(stdout, "%d ", *(int32_t*) dev);
    dev = cblk_prev(&cblk, dev);
  } while (dev != org);
  
  fprintf(stdout, "\n");
}

static void print_by_next_val(void) {
  void* dev = cblk.front;
  fprintf(stdout, "Running over next values from front: ");
  do {
    fprintf(stdout, "%d ", *(int32_t*) dev);
    dev = cblk_next(&cblk, dev);
  } while (dev != cblk.front);
  
  fprintf(stdout, "\n");
}

static void print_by_itr(void) {
  fprintf(stdout, "Iterating by index: ");

  for (int32_t i = 0; i < cblk.size; i++) {
    void* itr = cblk_itr(&cblk, i);
    fprintf(stdout, "%d ", *((int32_t*) itr));
  }

  fprintf(stdout, "\n");
}

int main(int const argc, char const* argv[]) {
  cblk = cblk_init(sizeof(int32_t), 10);
  for (int32_t i = 0; i < 10; i++) {
    cblk_map(&cblk, &i);
  }

  fprintf(stdout, "After initial mapping\n");
  fprintf(stdout, "Front %d Back %d\n", 
    *(int32_t*) cblk.front, *(int32_t*) cblk.back);

  print_by_next_val();
  print_by_itr();
  
  {
    /* Remove 1st */
    void* const rm = cblk_itr(&cblk, 0);
    cblk_unmap(&cblk, rm);
    fprintf(stdout, "After removal of 1st\n");
    fprintf(stdout, "Front %d Back %d\n", 
      *(int32_t*) cblk.front, *(int32_t*) cblk.back);
    print_by_next_val();
    print_by_itr();
  }
  
  {
    /* Add another */
    int32_t const i = 37;
    cblk_map(&cblk, &i);
    fprintf(stdout, "After mapping another\n");
    fprintf(stdout, "Front %d Back %d\n", 
      *(int32_t*) cblk.front, *(int32_t*) cblk.back);
    print_by_next_val();
    print_by_itr();
  }

  {
    /* Add another */
    int32_t const i = 111;
    cblk_map(&cblk, &i);
    fprintf(stdout, "After mapping another\n");
    fprintf(stdout, "Front %d Back %d\n", 
      *(int32_t*) cblk.front, *(int32_t*) cblk.back);
    print_by_next_val();
    print_by_itr();
  }
  
  {
    /* Add a few more */
    for (int32_t i = 500; i < 510; i++) {
      cblk_map(&cblk, &i);
    }

    fprintf(stdout, "Front %d Back %d\n", 
      *(int32_t*) cblk.front, *(int32_t*) cblk.back);
    print_by_next_val();
    print_by_itr();
  }

  {
    /* Remove a few */
    for (size_t i = 15; i < 18; i++) {
      void* const rm = cblk_itr(&cblk, i);
      size_t d = cblk_dist(&cblk, rm);
      fprintf(stdout, "Dist from root %lu\n", d);
      cblk_unmap(&cblk, rm);
      fprintf(stdout, "Front %d Back %d\n", 
        *(int32_t*) cblk.front, *(int32_t*) cblk.back);
      print_by_next_val();
      print_by_itr();
    }
  }

  {
    void* const dev = cblk_itr(&cblk, 15);
    fprintf(stdout, "Val %d\n", *(int32_t*) dev);
    print_by_prev_val(dev);
    size_t d = cblk_dist(&cblk, dev);
    fprintf(stdout, "Dist from root %lu\n", d);
  }

  return 0;
}
