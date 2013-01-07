// (c) 2012 by Julie Kim. All rights reserved.
// Julie Kim (kjulee114@gmail.com)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"

#define LINE_MAX 1024

int main(int argc, char* argv[]) {

  FILE* tracef;
  char ch;
  char* line;
  int l_pos = 0;
  int cur_max = LINE_MAX;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s TRACE_FILE\n", argv[0]);
    exit(1);
  }

  tracef = fopen(argv[1], "r");
  
  if (tracef == NULL) {
    fprintf(stderr, "Cannot open trace file %s\n", argv[1]);
    exit(1);
  }

  line = (char*) calloc(cur_max, sizeof(char));
  ch = getc(tracef);
  while (ch != EOF) {
    // parse trace line and do proper actions
    if (ch == '\n') {
      // open : allocate new trace struct and add to struct list
      // and write '0's as much as size of the file into rb_{filename}.txt
      // read : find proper offset in reading file and convert '0' to '1'
      // at read bit
      // close : free the trace struct and closing read bit file
      // dup, dup2 : TODO(Julie)
      if (strstr(line, "open") != NULL) {
        trace *new_fd = (trace *) malloc(sizeof(trace));
        char* pch = strtok(line, " ");
        if (pch != NULL) {
          new_fd->pid = atol(pch);
        }

        pch = strtok(NULL, " ");
        // TODO(Julie) parse again for getting fname and fd
      } else if (strstr(line, "read") != NULL) {
      } else if (strstr(line, "close") != NULL) {
      } else if (strstr(line, "lseek") != NULL) {
      } else if (strstr(line, "dup") != NULL) {
      } else if (strstr(line, "dup2") != NULL) {
      }
      l_pos = 0;
      cur_max = LINE_MAX;
      free(line);
      line = (char*) calloc(cur_max, sizeof(char));
      ch = getc(tracef);
      continue;
    }

    if (l_pos == cur_max) {
      cur_max *= 2;
      line = (char*) realloc(line, cur_max);
    }

    line[l_pos] = ch;
    l_pos++;
    ch = getc(tracef);
  }
}
