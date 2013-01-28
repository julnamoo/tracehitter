// Copyright (c) 2013 by Julie Kim (kjulee114@gmail.com)

#ifndef __DEFS_H__
#define __DEFS_H__

#define TRUE 1
#define FALSE 0

#include <stdlib.h>
#include <syslog.h>

typedef struct _trace trace;
typedef struct _trace_node trace_node;
typedef struct _pid_node proc_node;

proc_node* proc_list = NULL;
proc_node* proc_ptr = NULL;

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
};

// Attach new proc_node to proc_list.
// Return pid of new_node
int add_proc_node(int pid, proc_node *new_node);

// Return existence of proc_node with pid.
int exist_proc_node(int pid);

// Return ptr for proc_node of pid
proc_node* find_proc_node(int pid);

// Remove proc_node from global proc_list and return removed pid
int remove_proc_node(long int pid);

// Attach new trace_node to trace_tree of pid.
// Return fd of new_node
int add_trace_node(int pid, trace_node *new_node);

// Return existence of trace_node with fd
int exist_trace_node(trace_node *trace_tree, int fd);

// Find parent trace_node from proc_list of pid with fd
trace_node* find_parent_trace_node(trace_node* trace_tree, int fd);

// Remove trace_node from trace_tree in proc_node and return pid
int remove_trace_node(long int pid, long int fd);

#endif  //  __DEFS_H__
