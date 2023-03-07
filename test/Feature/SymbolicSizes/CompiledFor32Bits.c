// REQUIRES: z3
// RUN: %clang %s -g -m32 -emit-llvm %O0opt -c -o %t1.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --output-dir=%t.klee-out -solver-backend=z3 --out-of-mem-allocs=false %t1.bc 2>&1 | FileCheck %s

#include <stdlib.h>
#include "klee/klee.h"

int main() {
  int n = klee_int("n");
  char *s = malloc(n);
  // CHECK: CompiledFor32Bits.c:[[@LINE+1]]: memory error: out of bound pointer
  s[1] = 10;
}