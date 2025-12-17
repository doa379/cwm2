#pragma once

typedef struct {
  Cursor ptr;
  Cursor move;
  Cursor resize;
  Cursor h;
  Cursor v;
} font_crs_t;

int font_init(void);
void font_deinit(void);
int font_cursor_init(void);
void font_cursor_deinit(void);
