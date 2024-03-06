#pragma once

enum event {
  NOOP,
  MAPNOTIFY,
  UNMAPNOTIFY,
  CLIENTMESSAGE,
  CONFIGURENOTIFY,
  MAPREQUEST,
  CONFIGUREREQUEST,
  MOTIONNOTIFY,
  KEYPRESS,
  BTNPRESS,
  ENTERNOTIFY,
  PROPERTYNOTIFY
};

typedef struct {
  long DATA[4];
  void* ptr[2];
  void (*evfn)(const long, const long, const long);
  enum event name;
  int pad;
} ev_t;

