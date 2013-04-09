// Copyright (c) 2013 by Julie Kim (kjulee114@gmail.com)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "defs.h"

#define LINE_MAX 4096

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
      // dup, dup2 : copy trace structure and add to trace_tree of the process
      
      /** check unfinished or resumed **/
      if (strstr(line, "unfinished") != NULL) {
        syslog(LOG_DEBUG, "enter unfinished..");
      } else if (strstr(line, "resumed") != NULL) {
      } else {
        trace *new_fd = (trace *) malloc(sizeof(trace));
        new_fd->state = 1;

        if (strstr(line, "open(") != NULL) {
          syslog(LOG_DEBUG, "enter open parser");
          long int ppid;
          char* pch = strtok(line, " ");
          if (pch != NULL) {
            new_fd->pid = atol(pch);
            syslog(LOG_DEBUG, "open:set pid:%ld", new_fd->pid);
            pch = strtok(NULL, " ");
            ppid = atol(pch);
            syslog(LOG_DEBUG, "open:catch ppid:%ld", ppid);
          } else {
            fprintf(stderr, "Cannot parse trace log(@open, pid):%s\n", pch);
            exit(EXIT_FAILURE);
          }

          syslog(LOG_DEBUG, "open:current parser pos:%s", pch);
          pch = strtok(NULL, " ");
          pch = strtok(NULL, " ");
          syslog(LOG_DEBUG, "open:get fname from:%s", pch);
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
              syslog(LOG_DEBUG, "open:set fname:%s", new_fd->fname);
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

          syslog(LOG_DEBUG, "open:current parser pos:%s", pch);
          new_fd->fd = atol(pch);
          syslog(LOG_DEBUG, "set fd:%ld", new_fd->fd);
          new_fd->rval = atol(pch);
          syslog(LOG_DEBUG, "set rval:%ld", new_fd->rval);
          if (new_fd->rval < 0) {
            syslog(LOG_DEBUG, "Fail to open file %s", new_fd->fname);
            free(new_fd);
            continue;
          }
          
          /** add new_fd to proc_node **/
          if (exist_proc_node(new_fd->pid) == FALSE) {
            syslog(LOG_DEBUG, "open:new process..%ld", new_fd->pid);
            proc_node *new_proc = (proc_node*) malloc(sizeof(proc_node));
            new_proc->pid = new_fd->pid;
            new_proc->ppid = ppid;
            new_proc->next_proc_node = NULL;
            new_proc->trace_tree = (trace_node*) malloc(sizeof(trace_node));
            new_proc->trace_tree->fd = new_fd->fd;
            new_proc->trace_tree->trace = new_fd;
            new_proc->trace_tree->rchild = NULL;
            new_proc->trace_tree->lchild = NULL;
            add_proc_node(new_proc->pid, new_proc);
          } else {
            syslog(LOG_DEBUG, "open:exist process..%ld", new_fd->pid);
            proc_node *cur_proc = find_proc_node(new_fd->pid);
            syslog(LOG_DEBUG, "open:success to find proc_node:%d", cur_proc->pid);
            trace_node *new_trace = (trace_node*) malloc(sizeof(trace_node));
            new_trace->fd = new_fd->fd;
            new_trace->trace = new_fd;
            new_trace->trace->offset = 0;
            new_trace->rchild = NULL;
            new_trace->lchild = NULL;
            add_trace_node(new_fd->pid, new_trace);
            syslog(LOG_DEBUG,
                "complete to add new trace node into pid %ld, fd %ld",
                new_fd->pid, new_fd->fd);
          }
        } else if (strstr(line, "read(") != NULL) {
          syslog(LOG_DEBUG, "enter read parser");
          char* pch = strtok(line, "()=, ");
          long int ppid = 0;
          proc_node* cur_proc;
          trace_node* cur_trace;
          int len;

          /** For print byte offset **/
          FILE op_fp = NULL;

          new_fd->pid = atol(pch);
          pch = strtok(NULL, "()=, ");
          cur_proc = find_proc_node(new_fd->pid);
          if (cur_proc == NULL) {
            fprintf(stderr, "read:Cannot operate reading on non-exist process(%ld)",
                new_fd->pid);
            syslog(LOG_WARNING, "read:Cannot operate reading on non-exist process(%ld)",
                new_fd->pid);
            free(new_fd);
            continue;
          }
          ppid = atol(pch);
          syslog(LOG_DEBUG, "read:Set pid:%ld, ppid:%ld", new_fd->pid, ppid);

          pch = strtok(NULL, "()=, ");
          pch = strtok(NULL, "()=, ");
          pch = strtok(NULL, "()=, ");
          new_fd->fd = atol(pch);
          syslog(LOG_DEBUG, "read:Set fd:%ld", new_fd->fd);
          cur_trace = find_trace_node(cur_proc->trace_tree, new_fd->fd);
          if (cur_trace == NULL) {
            fprintf(stderr, "read:There is not opened fd..(pid:%ld, fd:%ld)\n",
                new_fd->pid, new_fd->fd);
            syslog(LOG_WARNING, "read:There is not opened fd..(pid:%ld, fd:%ld)",
                new_fd->pid, new_fd->fd);
            free(new_fd);
            continue;
          }
          free(new_fd);

          pch = strtok(NULL, "()=, ");
          pch = strtok(NULL, "()=, ");
          len = atol(pch);

          pch = strtok(NULL, "()=, ");
          cur_trace->trace->rval = atol(pch);
          syslog(LOG_DEBUG, "read:Set fd:%ld, request %ld read, success %ld"
              cur_trace->trace->fd, len, cur_trace->trace->rval);
          //TODO(Julie) Print 1 from the current offset to rval
          syslog(LOG_DEBUG, "read:Open File by %s");
          op_fp = fopen(cur_trace->trace->fname, "w");

          syslog(LOG_DEBUG, "read:Update fd offset from %ld to %ld",
              cur_trace->trace->offset, cur_trace->trace->offset+new_fd->rval);
          cur_trace->trace->offset += rval;
          
        } else if (strstr(line, "close(") != NULL) {
          //Find trace_node from proc_node and remove the trace_node
          //from trace_tree in proc_node. If the node is the last, then remove
          //proc_node from proc_list like before.
          syslog(LOG_DEBUG, "enter close parser");
          char* pch = strtok(line, " ");
          if (pch != NULL) {
            new_fd->pid = atol(pch);
            syslog(LOG_DEBUG, "set pid %ld to new_fd to close", new_fd->pid);
          } else {
            fprintf(stderr, "Cannot parse trace log(@close, pid):%s", pch);
            free(new_fd);
            exit(EXIT_FAILURE);
          }

          pch = strtok(NULL, " ");
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
        } else if (strstr(line, "lseek(") != NULL) {
          syslog(LOG_DEBUG, "enter lseek parser");
          char* pch = strtok(line, " ");
          int op, fd, rval;
          long int pid, ppid;

          proc_node* cur_proc = NULL;
          trace_node* cur_trace = NULL;
          op = fd = rval = 0;
          pid = ppid = -1;
          free(new_fd);

          if (pch != NULL) {
            pid = atol(pch);
            syslog(LOG_DEBUG, "set pid %ld to new_fd @lseek", pid);
            pch = strtok(NULL, " ");
            ppid = atol(pch);
            syslog(LOG_DEBUG, "set ppid %ld @lseek", ppid);
          } else {
            fprintf(stderr, "Cannot parse trace log (@lseek, pid):%s", pch);
            exit(EXIT_FAILURE);
          }

          pch = strtok(NULL, "(),= ");
          pch = strtok(NULL, " (),=");
          pch = strtok(NULL, "(),= ");
          fd = atoi(pch);
          syslog(LOG_DEBUG, "Current parser pos(@lseek1):%s", pch);
          
          pch = strtok(NULL, " (),=");
          pch = strtok(NULL, "(),= ");
          if (strstr(pch, "END")) {
            op = -1; // SEEK_END
          } else if (strstr(pch, "SET")) {
            op = 1; //SEEK_SET
          } else if (strstr(pch, "CUR")) {
            op = 0; //SEEK_CUR
          }
          syslog(LOG_DEBUG, "Current parser pos(@lseek2):%s", pch);

          pch = strtok(NULL, "(),= ");
          rval = atoi(pch);
          syslog(LOG_DEBUG, "Current parser pos(@lseek3):%s", pch);

          cur_proc = find_proc_node(pid);
          if (cur_proc == NULL) {
            syslog(LOG_WARNING, "lseek:Cannot find the process(pid:%ld)", pid);
            free(new_fd);
            continue;
          }
          cur_trace = find_trace_node(cur_proc->trace_tree, fd);
          if (cur_trace == NULL) {
            syslog(LOG_WARNING, "lseek:Cannot find the trace_node(pid:%ld, fd:%d",
                pid, fd);
            free(new_fd);
            continue;
          }
          new_fd = cur_trace->trace;
          if (new_fd == NULL) {
            syslog(LOG_WARNING, "lseek:Unavailable lseek..(pid:%ld, fd:%d)", pid, fd);
            fprintf(stderr, "lseek:Unavailable lseek..(pid:%ld, fd:%d)\n", pid, fd);
            free(new_fd);
            continue;
          } else {
            switch (op) {
              case -1:
                new_fd->offset = rval;
                break;
              case 0:
                new_fd->offset += rval;
                break;
              case 1:
                new_fd->offset = 0;
                break;
            }
            syslog(LOG_DEBUG, "Set new offset of pid(%ld), fd(%ld) to %ld",
                new_fd->pid, new_fd->fd, new_fd->offset);
          }
        } else if (strstr(line, "dup2") != NULL) {
          /** copy a trace_node and insert in to trace_tree of the proc_node
           * from exist trace_node **/
          syslog(LOG_DEBUG, "enter dup2 parser");
          char* pch = strtok(line, " ");
          long int ppid = -1;
          if (pch != NULL) {
            new_fd->pid = atol(pch);
            syslog(LOG_DEBUG, "set pid %ld to new_fd to dup", new_fd->pid);
            pch = strtok(NULL, " ");
            ppid = atol(pch);
            syslog(LOG_DEBUG, "find ppid:%ld", ppid);
          } else {
            fprintf(stderr, "Cannot parse trace log(@dup2, pid):%s", pch);
            exit(EXIT_FAILURE);
          }

          pch = strtok(NULL, " ");
          pch = strtok(NULL, " ");
          syslog(LOG_DEBUG, "current parser pos (@dup2):%s", pch);
          if (pch != NULL) {
            long int len = strlen(pch);
            long int old = -1;
            long int new = -1;
            char* dup2 = (char*) malloc(sizeof(char) * len);
            memcpy(dup2, pch, sizeof(char) * len);
            pch = strtok(NULL, " ");
            syslog(LOG_DEBUG, "current parser pos (@dup2):%s", pch);
            new = atol(pch);
            pch = strtok(NULL, "=");
            pch = strtok(NULL, "=");
            new_fd->rval = atol(pch);
            syslog(LOG_DEBUG, "set rval(@dup2):%ld", new_fd->rval);
            dup2 = strtok(dup2, "(,");
            dup2 = strtok(NULL, "(,");
            old = atol(dup2);
            new_fd->fd = old;
            if (new != new_fd->rval) {
              syslog(LOG_DEBUG, "Fail dup2(%ld, %ld)", new, old);
              free(new_fd);
              continue;
            }
            syslog(LOG_DEBUG, "Extract fds from dup2>>old:%ld, new:%ld",
                old, new);
          } else {
            fprintf(stderr, "Cannot parse trace log(@dup2, pid):%s", pch);
            exit(EXIT_FAILURE);
          }
          proc_node* cur_proc = find_proc_node(new_fd->pid);
          trace_node* new_trace = NULL;
          
          if (cur_proc == NULL) {
            /** copy trace_node from parent process node and construct
             * new process node with pid and link it to the parent **/
            proc_node *parent_proc = find_proc_node(ppid);
            if (parent_proc == NULL) {
              syslog(LOG_DEBUG, "dup2:Unavailable trace value...(ppid:%ld, pid:%ld, fd:%ld)",
                  ppid, new_fd->pid, new_fd->fd);
              free(new_fd);
              continue;
            }
            trace_node* old_trace = find_trace_node(parent_proc->trace_tree,
                new_fd->fd);
            if (old_trace == NULL) {
              syslog(LOG_DEBUG, "dup2:Unavailable trace value...(ppid:%ld, pid:%ld, fd:%ld)",
                  ppid, new_fd->pid, new_fd->fd);
              free(new_fd);
              continue;
            }
            proc_node *new_proc = (proc_node*) malloc(sizeof(proc_node));
            new_proc->pid = new_fd->pid;
            new_proc->ppid = ppid;
            new_proc->next_proc_node = NULL;

            new_trace = (trace_node*) malloc(sizeof(trace_node));
            memcpy(new_trace, find_trace_node(parent_proc->trace_tree, new_fd->fd),
                sizeof(trace_node));
            syslog(LOG_DEBUG,
                "copy trace_ from (pid:%ld, fd:%ld) to (pid:%ld, fd:%d)",
                ppid, new_fd->fd, new_trace->trace->pid, new_trace->fd);
            new_trace->fd = new_fd->rval;
            new_fd->fd = new_fd->rval;
            new_trace->trace = new_fd;
            new_trace->rchild = NULL;
            new_trace->lchild = NULL;
            new_proc->trace_tree = new_trace;
            add_proc_node(new_proc->pid, new_proc);
            syslog(LOG_DEBUG, "Add a new child process (pid:%d) to (pid:%d)",
                new_proc->ppid, new_proc->pid);
          } else {
            /** copy trace_node from the current process node **/
            trace_node* old_trace = find_trace_node(cur_proc->trace_tree,
                new_fd->fd);
            if (old_trace == NULL) {
              syslog(LOG_DEBUG, "Start to find the trace node from parent");
              proc_node* pp_node = find_proc_node(cur_proc->ppid);
              if (pp_node == NULL) {
                syslog(LOG_WARNING,
                    "Cannot find parent proc_node...(ppid:%d, pid:%d, fd:%d)",
                    cur_proc->ppid, cur_proc->pid, old_trace->fd);
                free(new_fd);
                continue;
              } else {
                old_trace = find_trace_node(pp_node->trace_tree, new_fd->fd);
                if (old_trace == NULL) {
                  syslog(LOG_WARNING,
                      "Cannot find original trace_node..(ppid:%d, pid:%d, fd:%d)",
                      pp_node->pid, cur_proc->pid, old_trace->fd);
                  free(new_fd);
                  continue;
                } else {
                  syslog(LOG_DEBUG, "Find old fd(%ld) from parent process(%ld)",
                    new_fd->fd, ppid);
                }
              }
            } else {
              syslog(LOG_DEBUG, "Start copy trace node");
              new_trace = (trace_node*) malloc(sizeof(trace_node));
              memcpy(new_trace, old_trace, sizeof(trace_node));
              syslog(LOG_DEBUG,
                "copy trace_ from (pid:%ld, fd:%d) to (pid:%ld, fd:%ld)",
                old_trace->trace->pid, old_trace->fd, new_trace->trace->pid, new_fd->rval);
              new_trace->fd = new_fd->rval;
              new_fd->fd = new_fd->rval;
              new_trace->trace = new_fd;
              new_trace->trace->offset = 0;
              new_trace->rchild = NULL;
              new_trace->lchild = NULL;
              add_trace_node(new_trace->trace->pid, new_trace);
            }
          }
        } else if (strstr(line, "dup(") != NULL) {
        } else if (strstr(line, "fcntl(") != NULL) {
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
