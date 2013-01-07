// (c) 2012 by Julie Kim. All rights reserved.
// Julie Kim (kjulee114@gmail.com)

#include <stdio.h>

#include "defs.h"

#define LINE_MAX 1024

int main(int argc, char* argv[]) {

  FILE* tracef;
  char ch;
  char* line;
  int l_pos = 0;
  int cur_max = LINE_MAX;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s TRACE_FILE", argv[0]);
    exit(1);
  }

  tracef = fopen(argv[1], "r");
  
  if (tracef == NULL) {
    fprintf(stderr, "Cannot open trace file %s\n", argv[1]);
    exit(1);
  }

  line = (char*) malloc(sizeof(char) * cur_max);
  ch = getc(tracef);
  while (ch != EOF) {
    if (ch == '\n') {
      // parse trace line and do proper actions
      // open : allocate new trace struct and add to struct list
      // and write '0's as much as size of the file into rb_{filename}.txt
      // read : find proper offset in reading file and convert '0' to '1'
      // at read bit
      // close : free the trace struct and closing read bit file
      // TODO(Julie)

      continue;
    }

    if (l_pos == LINE_MAX) {
      cur_max *= 2;
      line = realloc(line, cur_max);
    }

    line[l_pos] = ch;
    l_pos++;
  }
}
