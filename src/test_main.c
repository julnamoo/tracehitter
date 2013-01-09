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
  if ((NULL == CU_add_test(pSuite, "successful_test_1", test_exist_proc_node)))
  {
  CU_cleanup_registry();
  return CU_get_error();
  }

  /* run all tests using the automated interface */
  CU_automated_run_tests();
  CU_list_tests_to_file();

  /* run all tests usint the console interface */
//  CU_console_run_tests();

  /* run all tests using the curses interface */
  /* (only on system having curses) */

  CU_cleanup_registry();
  return CU_get_error();
}
