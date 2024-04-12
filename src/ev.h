#pragma once

#include <atoms.h>

enum event {
  NOOP,
  MAPNOTIFY,
  UNMAPNOTIFY,
  //CLIENTMESSAGE,
  CONFIGUREROOT,
  CONFIGURENOTIFY,
  MAPREQUEST,
  CONFIGUREREQUEST,
  MOTIONNOTIFY,
  KEYPRESS,
  BTNPRESS,
  ENTERNOTIFY,
  PROPERTYNOTIFY,
  EXPOSE
};
  
typedef union {
	char B[20];
	short S[10];
	long L[5];
} data_t;

typedef struct {
  long DATA[4];
  void* ptr[2];
  //data_t data; 
  //void (*evfn)(const data_t);
  void (*evfn)(const long, const long, const long);
  union {
    const enum event EVENT;
    const enum prop PROP;
  };
} ev_t;

void init_event(ev_t*);
void init_msgevent(ev_t*);
void init_events(Display*);
ev_t* event();
