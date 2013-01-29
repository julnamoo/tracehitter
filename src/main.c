// Copyright (c) 2013 by Julie Kim (kjulee114@gmail.com)

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
  syslog(LOG_DEBUG, "Start parse %s", argv[1]);

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
      } else {
        trace *new_fd = (trace *) malloc(sizeof(trace));
        new_fd->state = 1;

        if (strstr(line, "open") != NULL) {
          syslog(LOG_DEBUG, "enter open parser");
          char* pch = strtok(line, " ");
          if (pch != NULL) {
            new_fd->pid = atol(pch);
            syslog(LOG_DEBUG, "set pid:%ld", new_fd->pid);
          } else {
            fprintf(stderr, "Cannot parse trace log(@open, pid):%s\n", pch);
            exit(EXIT_FAILURE);
          }

          pch = strtok(NULL, " ");
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
            } else {
              fprintf(stderr, "Cannot parse trace log(@open, fname1):%s\n", fpath);
              free(tmp);
              free(new_fd);
              exit(EXIT_FAILURE);
            }
          } else {
              fprintf(stderr, "Cannot parse trace log(@open, fname2):%s\n", pch);
              free(new_fd);
              exit(EXIT_FAILURE);
          }

          syslog(LOG_DEBUG, "current parser pos:%s", pch);
          new_fd->fd = atol(pch);
          syslog(LOG_DEBUG, "set fd:%ld", new_fd->fd);
          new_fd->rval = atol(pch);
          syslog(LOG_DEBUG, "set rval:%ld", new_fd->rval);
          
          /** add new_fd to proc_node **/
          if (exist_proc_node(new_fd->pid) == FALSE) {
            syslog(LOG_DEBUG, "new process:%ld", new_fd->pid);
            proc_node *new_proc = (proc_node*) malloc(sizeof(proc_node));
            new_proc->pid = new_fd->pid;
            new_proc->next_proc_node = NULL;
            new_proc->trace_tree = (trace_node*) malloc(sizeof(trace_node));
            new_proc->trace_tree->fd = new_fd->fd;
            new_proc->trace_tree->trace = new_fd;
            new_proc->trace_tree->rchild = NULL;
            new_proc->trace_tree->lchild = NULL;
            add_proc_node(new_proc->pid, new_proc);
          } else {
            syslog(LOG_DEBUG, "exist process:%ld", new_fd->pid);
            proc_node *cur_proc = find_proc_node(new_fd->pid);
            syslog(LOG_DEBUG, "success to find proc_node:%d", cur_proc->pid);
            trace_node *new_trace = (trace_node*) malloc(sizeof(trace_node));
            new_trace->fd = new_fd->fd;
            new_trace->trace = new_fd;
            new_trace->rchild = NULL;
            new_trace->lchild = NULL;
            add_trace_node(new_fd->pid, new_trace);
            syslog(LOG_DEBUG,
                "complete to add new trace node into pid %ld, fd %ld",
                new_fd->pid, new_fd->fd);
          }
        } else if (strstr(line, "read") != NULL) {
        } else if (strstr(line, "close") != NULL) {
          //TODO(Julie) Find trace_node from proc_node and remove the trace_node
          //from trace_tree in proc_node. If the node is the last, then remove
          //proc_node from proc_list like before.
          syslog(LOG_DEBUG, "enter close parser");
          char* pch = strtok(line, " ");
          if (pch != NULL) {
            new_fd->pid = atol(pch);
            syslog(LOG_DEBUG, "set pid %ld to new_fd to close", new_fd->pid);
          } else {
            fprintf(stderr, "Cannot parse trace log(@close, pid):%s", pch);
            exit(EXIT_FAILURE);
          }

          pch = strtok(NULL, " ");
          pch = strtok(NULL, " ");
          syslog(LOG_DEBUG, "current parser pos %s", pch);
          if (pch != NULL) {
            int len = strlen(pch);
            char* fd_tok = (char*) malloc(sizeof(char) * len);
            memcpy(fd_tok, pch, sizeof(char) * len);
            /** set rval and fd **/
            pch = strtok(NULL, "=");
            pch = strtok(NULL, "=");
            new_fd->rval = atol(pch);
            syslog(LOG_DEBUG, "set rval for closing:%ld", new_fd->rval);
            fd_tok = strtok(fd_tok, "()");
            fd_tok = strtok(NULL, "()");
            new_fd->fd = atol(fd_tok);
            syslog(LOG_DEBUG, "set fd for closing:%ld", new_fd->fd);
          } else {
            fprintf(stderr, "Cannot parse trace log(@close, fd):%s", pch);
            exit(EXIT_FAILURE);
          }

          /** remove trace_node **/
          if(remove_trace_node(new_fd->pid, new_fd->fd) != new_fd->pid) {
            fprintf(stderr, "Fail to remove trace_node at %ld(pid)",
                new_fd->pid);
            exit(EXIT_FAILURE);
          }
        } else if (strstr(line, "lseek") != NULL) {
        } else if (strstr(line, "dup") != NULL) {
        } else if (strstr(line, "dup2") != NULL) {
        }
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
  exit(EXIT_SUCCESS);
}
