// Copyright (c) 2012 Julie Kim (julie114.kim@gmail.com)
// CUnit Test code for tracehitter
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
  if ((NULL == CU_add_test(pSuite, "successful_exist_proc_node",
          test_exist_proc_node)) ||
      (NULL == CU_add_test(pSuite, "successful_add_proc_node",
                           test_add_proc_node)))
  {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* run all tests using the automated interface */
  //CU_automated_run_tests();
  //CU_list_tests_to_file();

  /* run all tests usint the console interface */
  CU_console_run_tests();

  CU_cleanup_registry();
  return CU_get_error();
}
