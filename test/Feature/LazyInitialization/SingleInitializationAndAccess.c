// RUN: %clang %s -emit-llvm -g -c -o %t1.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --output-dir=%t.klee-out --skip-not-symbolic-objects %t1.bc 2>&1 | FileCheck %s

#include <assert.h>
#include "klee/klee.h"

int main() {
  int *x;
  klee_make_symbolic(&x, sizeof(x), "*x");
  // CHECK: SingleInitializationAndAccess.c:[[@LINE+1]]: memory error: null pointer exception
  *x = 10;
  // CHECK-NOT: SingleInitializationAndAccess.c:[[@LINE+1]]: memory error: null pointer exception
  if (*x == 10) {
    // CHECK-NOT: SingleInitializationAndAccess.c:[[@LINE+2]]: memory error: null pointer exception
    // CHECK-NOT: SingleInitializationAndAccess.c:[[@LINE+1]]: memory error: out of bound pointer
    *x = 20;
    // CHECK-NOT: SingleInitializationAndAccess.c:[[@LINE+2]]: memory error: null pointer exception
    // CHECK: SingleInitializationAndAccess.c:[[@LINE+1]]: memory error: out of bound pointer
    x[1] = 30;
  } else {
    // CHECK-NOT: ASSERTION FAIL
    assert(0);
  }
}

// CHECK: KLEE: done: completed paths = 0
// CHECK: KLEE: done: generated tests = 2
