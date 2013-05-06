// Copyright (c) 2012 Julie Kim (julie114.kim@gmail.com)
// CUnit Test code for tracehitter
#include <string.h>

#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "defs.h"

int init_suite_success(void) { return 0; }
int init_suite_failure(void) { return -1; }
int clean_suite_success(void) { return 0; }
int clean_suite_failure(void) { return -1; }

void test_exist_proc_node(void)
{
  CU_ASSERT_FALSE(exist_proc_node(1));
}

void test_add_proc_node(void) {
  proc_node* test_node = (proc_node*) malloc(sizeof(proc_node));
  test_node->pid = 3444;
  test_node->next_proc_node = NULL;
  test_node->trace_tree = (trace_node*) malloc(sizeof(trace_node));
  test_node->trace_tree->fd = 34;
  test_node->trace_tree->trace = (trace*) malloc(sizeof(trace));
  test_node->trace_tree->trace->state = 1;
  test_node->trace_tree->trace->pid = 3444;
  test_node->trace_tree->trace->fd = 34;
  test_node->trace_tree->trace->fname = "hello";
  test_node->trace_tree->trace->rval = 0;
  CU_ASSERT_EQUAL(add_proc_node(test_node->pid, test_node), 3444);
  
  CU_ASSERT_PTR_NOT_NULL(proc_list);
  CU_ASSERT_PTR_EQUAL(proc_list, test_node);
  CU_ASSERT_PTR_NULL(proc_list->next_proc_node);
}

void test_add_trace_node(void) {
  trace_node* new_node;
  /* set up process list containing one proc_node */
  /*
  proc_node* test_node = (proc_node*) malloc(sizeof(proc_node));
  test_node->pid = 3444;
  test_node->next_proc_node = NULL;
  test_node->trace_tree = (trace_node*) malloc(sizeof(trace_node));
  test_node->trace_tree->fd = 34;
  test_node->trace_tree->trace = (trace*) malloc(sizeof(trace));
  test_node->trace_tree->trace->state = 1;
  test_node->trace_tree->trace->pid = 3444;
  test_node->trace_tree->trace->fd = 34;
  test_node->trace_tree->trace->fname = "hello";
  test_node->trace_tree->trace->rval = 0;
  test_node->trace_tree->rchild = NULL;
  test_node->trace_tree->lchild = NULL;
  CU_ASSERT_EQUAL(add_proc_node(test_node->pid, test_node), 3444);*/

  /* set up new trace_node */
  new_node = (trace_node*) malloc(sizeof(trace_node));
  new_node->fd = 100;
  new_node->trace = (trace*) malloc(sizeof(trace));
  new_node->trace->state = 1;
  new_node->trace->pid = 3444;
  new_node->trace->fd = 100;
  new_node->trace->fname = "hello_My_fd_is_100";
  new_node->trace->rval = 1;
  new_node->rchild = NULL;
  new_node->lchild = NULL;
  CU_ASSERT_EQUAL(add_trace_node(new_node->trace->pid, new_node), 100);

}

void test_char_replace(void) {
  char s1 = '/';
  char s2 = '_';
  char* src = "/usr/lib/test/file";
  char* dest = (char*) malloc(sizeof(char) * strlen(src));
  char* ans = "_usr_lib_test_file";
  char_replace(s1, s2, src, dest);
  CU_ASSERT_STRING_EQUAL(dest, ans);
}

int main() {
  CU_pSuite pSuite = NULL;

  /* initialize the CUnit test registry */
  if (CUE_SUCCESS != CU_initialize_registry())
  return CU_get_error();
  /* add a suite to the registry */
  pSuite = CU_add_suite("Suite_success", init_suite_success, clean_suite_success);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  
  /* add the tests to the suite */
  /* if ((NULL == CU_add_test(pSuite, "successful_exist_proc_node",
          test_exist_proc_node)) ||
      (NULL == CU_add_test(pSuite, "successful_add_proc_node",
                           test_add_proc_node)) ||
      (NULL == CU_add_test(pSuite, "successful_add_trace_node",
                           test_add_trace_node)) || */
  if((NULL == CU_add_test(pSuite, "successful_char_replace",
                           test_char_replace)))
  {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* run all tests using the automated interface */
  CU_automated_run_tests();
  //CU_list_tests_to_file();

  /* run all tests usint the console interface */
  //CU_console_run_tests();
  CU_list_tests_to_file();

  CU_cleanup_registry();
  return CU_get_error();
}
