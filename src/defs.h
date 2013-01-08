// (c) 2012 by Julie Kim. All rights reserved.
// Julie Kim (kjulee114@gmail.com)

#ifndef __DEFS_H__
#define __DEFS_H__

typedef struct _trace trace;

struct _trace {
  int state; // 0:unfinished, 1:finished
  long pid;
  long fd;
  char* fname;
  long rval;
};

#endif  //  __DEFS_H__
