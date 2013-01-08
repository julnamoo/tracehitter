// (c) 2012 by Julie Kim. All rights reserved.
// Julie Kim (kjulee114@gmail.com)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "defs.h"

#define LINE_MAX 1024

int main(int argc, char* argv[]) {

  FILE* tracef;
  char ch;
  char* line;
  int l_pos = 0;
  int cur_max = LINE_MAX;

  /** open syslog **/
  openlog("trace_hitter", LOG_CONS|LOG_ODELAY|LOG_PID, LOG_USER);

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
      
      /** check unfinished or resumed **/
      if (strstr(line, "unfinished") != NULL) {
      } else if (strstr(line, "resumed") != NULL) {
      } else if (strstr(line, "open") != NULL) {
        syslog(LOG_DEBUG, "enter open parser");
        trace *new_fd = (trace *) malloc(sizeof(trace));
        char* pch = strtok(line, " ");
        if (pch != NULL) {
          new_fd->pid = atol(pch);
          syslog(LOG_DEBUG, "set pid:%ld", new_fd->pid);
        } else {
          fprintf(stderr, "Cannot parse trace log:%s\n", pch);
          exit(EXIT_FAILURE);
        }

        pch = strtok(NULL, " ");
        syslog(LOG_DEBUG, "get fname from:%s", pch);
        if (pch != NULL) {
          int len = strlen(pch);
          char* tmp = (char*) malloc(sizeof(char) * len);
          memcpy(tmp, pch, sizeof(char) * len);
          /** prevent from losing rval **/
          pch = strtok(NULL, "=");
          pch = strtok(NULL, "=");

          char* fpath = strtok(tmp, "\"");
          fpath = strtok(NULL, "\"");
          if (fpath != NULL) {
            new_fd->fname = fpath;
            syslog(LOG_DEBUG, "set fname:%s", new_fd->fname);
            free(tmp);
          } else {
            fprintf(stderr, "Cannot parse trace log:%s\n", fpath);
            free(tmp);
            exit(EXIT_FAILURE);
          }
        } else {
            fprintf(stderr, "Cannot parse trace log:%s\n", pch);
            exit(EXIT_FAILURE);
        }

        syslog(LOG_DEBUG, "current pos:%s", pch);
        
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

  closelog();
}
