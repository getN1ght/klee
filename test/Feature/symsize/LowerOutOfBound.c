// RUN: %clang %s -g -emit-llvm %O0opt -c -o %t1.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --output-dir=%t.klee-out --rewrite-equalities=false %t1.bc 2>&1 | FileCheck %s
// CHECK: KLEE: done: generated tests = 3

#include "klee/klee.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main() {
  int n;
  klee_make_symbolic(&n, sizeof(n), "n");
  if (n >= 5) {
    char *c1 = malloc(n);
    char *c2 = malloc(9 - n);
    c2[9 - n - 1] = 20;
  }
}