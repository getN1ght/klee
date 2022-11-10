// RUN: %clang %s -g -emit-llvm %O0opt -c -o %t1.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --output-dir=%t.klee-out --rewrite-equalities=false --skip-not-lazy-and-symbolic-pointers --max-symsize=128 %t1.bc 2>&1 | FileCheck %s

#include "klee/klee.h"
#include <stdlib.h>
#include <assert.h>

int main() {
  int *ptr;
  klee_make_symbolic(&ptr, sizeof(ptr), "ptr");
  // CHECK: LazyInstantiationOfSymbolicSize.c:[[@LINE+1]]: memory error: null pointer exception
  int x = ptr[0];
  // CHECK: LazyInstantiationOfSymbolicSize.c:[[@LINE+1]]: memory error: out of bound pointer
  int y = ptr[2];
  if (x + y == 16) {
    // CHECK: LazyInstantiationOfSymbolicSize.c:[[@LINE+1]]: ASSERTION FAIL
    assert(0);
  }
}

// CHECK: KLEE: done: completed paths = 1
// CHECK: KLEE: done: partially completed paths = 3
// CHECK: KLEE: done: generated tests = 4