// Copyright (c) 2013 by Julie Kim (kjulee114@gmail.com)

#ifndef __DEFS_H__
#define __DEFS_H__

#define TRUE 1
#define FALSE 0

#define RIGHT 1
#define LEFT 0

#define CLL_64 64
#define CLL_128 128
#define BLOCK_512 512
#define PAGE_4K 1024*4

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct _trace trace;
typedef struct _trace_node trace_node;
typedef struct _pid_node proc_node;

proc_node* proc_list = NULL;
proc_node* proc_ptr = NULL;

/** global variable to summarize total accessed bytes per units
 * 1/64/128/512/4K/file **/
int p_total[6] = { 0, 0, 0, 0, 0, 0 };

struct _trace {
  int state; // 0:unfinished, 1:finished
  long pid;
  long fd;
  char* fname;
  long rval;
  long int offset;
};

struct _trace_node {
  int fd;
  trace* trace;
  trace_node *rchild;
  trace_node *lchild;
};

struct _pid_node {
  int pid;
  int ppid;
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
trace_node* find_parent_trace_node(trace_node* trace_tree, int fd, trace_node* r_ptr);

// Find trace_node from trace_tree of pid with fd
trace_node* find_trace_node(trace_node* trace_tree, int fd);

// Remove trace_node from trace_tree in proc_node and return pid
int remove_trace_node(long int pid, long int fd);

/** for manipulate string **/
// Replace char s1 to s2 in src string
void char_replace(char s1, char s2, const char* src, char* dest);

/** Printing granularity **/
void print_granularity(char* filepath);

#endif  //  __DEFS_H__
