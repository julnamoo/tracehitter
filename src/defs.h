// (c) 2012 by Julie Kim. All rights reserved.
// Julie Kim (kjulee114@gmail.com)

#ifndef __DEFS_H__
#define __DEFS_H__

typedef struct _trace trace;
typedef struct _trace_node trace_node;
typedef struct _pid_node proc_node;

proc_node* proc_list;

struct _trace {
  int state; // 0:unfinished, 1:finished
  long pid;
  long fd;
  char* fname;
  long rval;
};

struct _trace_node {
  int fd;
  trace* trace;
  trace_node *rchild;
  trace_node *lchild;
};

struct _pid_node {
  int pid;
  trace_node *trace_tree;
  proc_node *next_proc_node;
}

// Attach new proc_node to proc_list.
// Return pid of new_node
int add_proc_node(int pid; proc_node* new_node);

// Attach new trace_node to trace_tree of pid.
// Return fd of new_node
int add_trace_node(int pid, trace_node* new_node);

// Find trace_node from proc_list of pid with fd
trace_node* find_trace_node(int pid, int fd);

#endif  //  __DEFS_H__
