// Copyright (c) 2013 by Julie Kim (kjulee114@gmail.com)
#include <string.h>

#include "defs.h"

void print_proc_list() {
  proc_node* ptr = proc_list;
  syslog(LOG_DEBUG, "current proc_list");
  for(; ptr != NULL; ptr = ptr->next_proc_node) {
    syslog(LOG_DEBUG, "pid:%d", ptr->pid);
  }
}

void print_trace_tree(trace_node* trace_tree) {
  if (trace_tree == NULL) {
    syslog(LOG_DEBUG, "End of trace_tree");
    return;
  }
  trace *t_ptr = trace_tree->trace;
  syslog(LOG_DEBUG, "ptt:pid %ld, fd %ld, fname %s, offset %ld, state %d, rval %ld",
      t_ptr->pid, t_ptr->fd, t_ptr->fname, t_ptr->offset, t_ptr->state, t_ptr->rval);
  print_trace_tree(trace_tree->rchild);
  //print_trace_tree(trace_tree->lchild);
}

int exist_proc_node(int pid) {
  proc_node* ptr;
  for (ptr = proc_list; ptr != NULL; ptr = ptr->next_proc_node) {
    syslog(LOG_DEBUG, "find process:%d", ptr->pid);
    if (ptr->pid == pid) {
      print_proc_list();
      return TRUE;
    }
  }
  syslog(LOG_DEBUG, "pid:%d does not exist", pid);
  print_proc_list();
  return FALSE;
}

int add_proc_node(int pid, proc_node *new_node) {
  // First insert - Create list
  if (proc_list == NULL) {
    proc_list = proc_ptr = new_node;
    syslog(LOG_DEBUG, "The first proc_node insertion. pid:%d", proc_list->pid);
    return proc_ptr->pid;
  }
  proc_ptr->next_proc_node = new_node;
  proc_ptr = new_node;
  syslog(LOG_DEBUG, "add new process node to proc_node list. pid:%d",
      proc_ptr->pid);
  return proc_ptr->pid;
}

proc_node* find_proc_node(int pid) {
  syslog(LOG_DEBUG, "enter find_proc_node with %d", pid);
  proc_node* ptr = proc_list;

  for (; ptr != NULL; ptr = ptr->next_proc_node) {
    if (ptr->pid == pid) {
      syslog(LOG_DEBUG, "Find proc_node(%d)", pid);
      return ptr;
    }
  }
  syslog(LOG_DEBUG, "Cannot find proc_node(%d) from list (@find_proc_node)",
      pid);
  return NULL;
}

int exist_trace_node(trace_node *s_ptr, int fd) {
  while (s_ptr != NULL) {
    if (s_ptr->fd == fd) {
      syslog(LOG_DEBUG, "exist_trace_node:fd %d is exist", fd);
      return TRUE;
    }
    s_ptr = s_ptr->rchild;
  }
  syslog(LOG_DEBUG, "exist_trace_node:Cannot find fd:%d", fd);
  return FALSE;
}

int add_trace_node(int pid, trace_node *new_node) {
  syslog(LOG_DEBUG, "enter add_trace_node");
  int depth = 0;
  if (exist_proc_node(pid) == FALSE) {
    syslog(LOG_DEBUG, "There are no process with %d. Returned", pid);
    return -1;
  }
  proc_node* p_ptr = find_proc_node(pid);
  trace_node* ttree_ptr = p_ptr->trace_tree;
  if (ttree_ptr == NULL) {
    p_ptr->trace_tree = new_node;
    syslog(LOG_DEBUG, "add_trace_node:First trace node of %d", pid);
    p_ptr->trace_tree->rchild = NULL;
    p_ptr->trace_tree->lchild = NULL;
    syslog(LOG_DEBUG, "add_trace_node:print trace tree");
    print_trace_tree(p_ptr->trace_tree);
    return p_ptr->trace_tree->fd;
  }
  trace_node* ttree_p_ptr = ttree_ptr;
  /** find position of new_node **/
  syslog(LOG_DEBUG, "add_trace_node:Start traversal from fd:%d", ttree_ptr->fd);
  for(; ttree_ptr != NULL; ttree_p_ptr = ttree_ptr, ttree_ptr = ttree_ptr->rchild) {
    if (ttree_ptr->fd == new_node->fd) {
      if (depth == 0) {
        syslog(LOG_DEBUG, "add_trace_node:It needs to chante the root..");
        p_ptr->trace_tree = new_node;
      } else {
        syslog(LOG_DEBUG, "add_trace_node:Attempt to insert already exist trace node...pid:%d, fd:%d(current depth:%d)", pid, new_node->fd, depth);
        ttree_p_ptr->rchild = new_node;
      }
      free(ttree_ptr);
      syslog(LOG_DEBUG, "add_trace_node:Update older trace_node to new");
      return new_node->fd;
    }
    ++depth;
  }
  ttree_p_ptr->rchild = new_node;
  syslog(LOG_DEBUG, "add_trace_node_node:new node pid:%d, fd:%d is located in %dth level",
      pid, new_node->fd, depth);
  syslog(LOG_DEBUG, "add_trace_node_node:print trace tree");
  print_trace_tree(p_ptr->trace_tree);
  return new_node->fd;
}

/** TODO(Julie) NWM to be linked trace node list **/
trace_node* find_parent_trace_node(trace_node* trace_tree, int fd, trace_node* r_ptr) {
  trace_node* ptr = trace_tree;
  while (ptr != NULL) {
    if (ptr->fd == fd) {
      syslog(LOG_DEBUG, "Parent of (pid %ld, fd %d) is itself",
          trace_tree->trace->pid, trace_tree->fd);
      return ptr;
    } else if (ptr->fd > fd) {
      if (ptr->lchild == NULL) {
        fprintf(stderr, "Tree Fault:The %d is not in the tree of pid:%ld(l)\n",
            fd, trace_tree->trace->pid);
        return NULL;
      }
      if (ptr->lchild->fd == fd) {
        syslog(LOG_DEBUG, "Find the %d, parent fd is %d", fd, ptr->fd);
        return ptr;
      } else {
        ptr = ptr->lchild;
        syslog(LOG_DEBUG,
            "Lchild of the parent is not matched. Move to lchild");
      }
    } else if (ptr->fd < fd) {
      if (ptr->rchild == NULL) {
        fprintf(stderr, "Tree Fault:The %d is not in the tree of pid:%ld(r)\n",
            fd, trace_tree->trace->pid);
        return NULL;
      }
      if (ptr->rchild->fd == fd) {
        syslog(LOG_DEBUG, "Find the %d, parent fd is %d", fd, ptr->fd);
        return ptr;
      } else {
        ptr = ptr->rchild;
        syslog(LOG_DEBUG,
            "Rchild of the parent is not matched. Move to rchild");
      }
    }
  }
  syslog(LOG_DEBUG, "Cannot find %d from trace_tree", fd);
  return NULL;
}

trace_node* find_trace_node(trace_node* trace_tree, int fd) {
  trace_node* t_ptr = trace_tree;
  int depth = 0;
  syslog(LOG_DEBUG, "enter find_trace_node with fd:%d", fd);
  if (trace_tree == NULL) {
    syslog(LOG_DEBUG, "find_trace_node:trace_tree is empty. return NULL");
    return NULL;
  }

  syslog(LOG_DEBUG, "find_trace_node:trace tree status");
  print_trace_tree(trace_tree);
  for(; t_ptr != NULL; t_ptr = t_ptr->rchild, ++depth) {
    if (t_ptr->fd == fd) {
      syslog(LOG_DEBUG, "find_trace_node:Find trace_node(%ld) in pid %ld",
          t_ptr->trace->fd, t_ptr->trace->pid);
      return t_ptr;
    }
    syslog(LOG_DEBUG, "find_trace_node:current level:%d", depth);
  }
  syslog(LOG_DEBUG, "find_trace_node:Cannot find the trace_node(%d) \
      from pid %ld", fd, trace_tree->trace->pid);
  return NULL;
}

int remove_trace_node(long int pid, long int fd) {
  syslog(LOG_DEBUG, "enter remove_trace_node with pid %ld, fd %ld", pid, fd);
  proc_node* cur_proc = find_proc_node(pid);
  trace_node* cur_trace = NULL;
  trace_node* p_trace = NULL;
  int depth = 0;

  if (cur_proc == NULL) {
    syslog(LOG_WARNING, "remove_trace_node:This proc(%ld) is not added to \
        proc_list yet", pid);
    return pid;
  }
  syslog(LOG_DEBUG, "remove_trace_node:Before delete trace_node(pid:%ld, fd:%ld)", pid, fd);
  print_trace_tree(cur_proc->trace_tree);
  //p_trace = find_parent_trace_node(cur_proc->trace_tree, fd);

  p_trace = cur_trace = cur_proc->trace_tree;
  if (p_trace == NULL) {
    syslog(LOG_WARNING, "This trace(pid:%d, fd:%ld) is from another op",
        cur_proc->pid, fd);
    return cur_proc->pid;
  }

  for(; cur_trace != NULL; p_trace = cur_trace, cur_trace = cur_trace->rchild) {
    if (cur_trace->fd == fd) {
      p_trace->rchild = cur_trace->rchild;
      syslog(LOG_DEBUG, "remove_trace_node:Delete the trace(fd:%ld)", fd);
      if (depth == 0) {
        if (cur_trace->rchild != NULL) {
          cur_proc->trace_tree = cur_trace->rchild;
          syslog(LOG_DEBUG, "remove_trace_node:Root is changed from fd:%d to fd:%d",
              cur_trace->fd, p_trace->rchild->fd);
        } else {
          cur_proc->trace_tree = NULL;
          syslog(LOG_DEBUG, "remove_trace_node:There is only root..\
              trace_tree is NULL");
        }
      }
      break;
    }
    ++depth;
  }
  free(cur_trace);
  syslog(LOG_DEBUG, "remove_trace_node:print trace tree after delete");
  print_trace_tree(cur_proc->trace_tree);
  return cur_proc->pid;
}

void char_replace(char s1, char s2, const char* src, char* dest) {
  int i = 0;
  int len = strlen(src);
  strncpy(dest, src, len);
  for (; i < len; i++) {
    if (src[i] == s1) {
      dest[i] = s2;
    }
  }
}

void print_granularity(char* filepath) {
  struct stat c_stat;
  
  syslog(LOG_DEBUG, "enter print_granularity with %s", filepath);
  if (stat(filepath, &c_stat) < 0) {
    syslog(LOG_DEBUG, "Cannot get status of %s", filepath);
    fprintf(stderr, "Cannot get status of %s\n", filepath);
    return;
  } else if (S_ISDIR(c_stat.st_mode)) {
    syslog(LOG_DEBUG, "print_granularity:This is directory>>%s", filepath);
    DIR *d_ptr;
    struct dirent *cur_dirent;

    if ((d_ptr = opendir(filepath)) == NULL) {
      syslog(LOG_DEBUG, "Cannot open %s..", filepath);
      fprintf(stderr, "Cannot open %s..\n", filepath);
      return;
    }

    char *cwd = getcwd(NULL, 0);
    syslog(LOG_DEBUG, "print_granularity:Old CWD is %s", cwd);
    chdir(filepath);
    free(cwd);
    cwd = getcwd(NULL, 0);
    syslog(LOG_DEBUG, "print_granularity:New CWD is %s", cwd);
    while ((cur_dirent = readdir(d_ptr)) != NULL) {
      if (strcmp(cur_dirent->d_name, ".") == 0
          || strcmp(cur_dirent->d_name, "..") == 0)
        continue;
      syslog(LOG_DEBUG, "print_granularity:current entry is %s",
          cur_dirent->d_name);
      char* tmp = (char*) malloc(sizeof(char) *
          (strlen(cwd) + strlen("/") + strlen(cur_dirent->d_name) + 1));
      sprintf(tmp, "%s/%s", cwd, cur_dirent->d_name);
      syslog(LOG_DEBUG, "print_granularity:Next file path is %s", tmp);
      print_granularity(tmp);
    }
    closedir(d_ptr);
  } else {
    char *fname = strstr(filepath, "tmp");
    fname = strstr(fname, "/");
    fprintf(stdout, "%s\n", fname);
    FILE* t_file = fopen(filepath, "r");
    fname[strlen(fname)-3] = '\0';
    FILE* o_file = fopen(fname, "r"); // For the coarest granularity
    syslog(LOG_DEBUG, "print_granularity:open footprint file %s", filepath);
    char *tmp = (char*) malloc(sizeof(char));
    unsigned int flag = 0;
    unsigned int i = 0;
    unsigned long long int total = 0;
    unsigned long long int g_total = 0;

    if (t_file == NULL) {
      fprintf(stderr, "cannot opn the trace file..%s", fname);
      fprintf(stdout, "cannot opn the trace file..%s", fname);
      return;
    }

    /** For 64 bytes and 1 bytes (Pure amount of read request) **/
    while (!feof(t_file)) {
      fscanf(t_file, "%c", tmp);
      ++i;
      flag += atoi(tmp);
      if (i % CLL_64 == 0 && flag > 0) {
        g_total += CLL_64;
        total += flag;
        i = flag = 0;
      }
    }
    if (flag > 0) {
      g_total += CLL_64;
      total += flag;
    }
    fprintf(stdout, "%10d\t%10llu\n", 1, total);
    fprintf(stdout, "%10d\t%10llu\n", CLL_64, g_total);
    p_total[0] += total;
    p_total[1] += g_total;

    /** For 128 bytes **/
    i = 0;
    g_total = 0;
    flag = 0;
    rewind(t_file);
    while (!feof(t_file)) {
      fscanf(t_file, "%c", tmp);
      ++i;
      flag += atoi(tmp);
      if (i % CLL_128 == 0 && flag > 0) {
        g_total += CLL_128;
        i = flag = 0;
      }
    }
    if (flag > 0) {
      g_total += CLL_128;
    }
    fprintf(stdout, "%10d\t%10llu\n", CLL_128, g_total);
    p_total[2] += g_total;

    /** For 512 Bytes **/
    i = 0;
    g_total = 0;
    flag = 0;
    rewind(t_file);
    while (!feof(t_file)) {
      fscanf(t_file, "%c", tmp);
      ++i;
      flag += atoi(tmp);
      if (i % BLOCK_512 == 0 && flag > 0) {
        g_total += BLOCK_512;
        i = flag = 0;
      }
    }
    if (flag > 0) {
      g_total += BLOCK_512;
    }
    fprintf(stdout, "%10d\t%10llu\n", BLOCK_512, g_total);
    p_total[3] += g_total;

    /** For 4K Bytes **/
    i = 0;
    g_total = 0;
    flag = 0;
    rewind(t_file);
    while (!feof(t_file)) {
      fscanf(t_file, "%c", tmp);
      ++i;
      flag += atoi(tmp);
      if (i % PAGE_4K == 0 && flag > 0) {
        g_total += PAGE_4K;
        i = flag = 0;
      }
    }
    if (flag > 0) {
      g_total += PAGE_4K;
    }
    fprintf(stdout, "%10d\t%10llu\n", PAGE_4K, g_total);
    fclose(t_file);
    p_total[4] += g_total;

    /** For whole file **/
    if (o_file == NULL) {
      syslog(LOG_DEBUG, "print_granularity:Cannot open origin file '%s'", fname);
      g_total = total;
    } else {
      g_total = ftell(o_file);
      fclose(o_file);
      if (g_total == 0)
        g_total = total;
    }
    fprintf(stdout, "%10llu\t%10llu\n", g_total, g_total);
    p_total[5] += g_total;
    fprintf(stdout, "Origin File size: %llubytes, Requested: %llubytes\n", g_total, total);

    fprintf(stdout, "\n\n");
  }
  
  syslog(LOG_DEBUG, "print_granularity:Finish printing");
}
