// RUN: %clang %s -emit-llvm %O0opt -c -o %t.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --output-dir=%t.klee-out 2>&1 | FileCheck %s

#include "klee/klee.h"

int main() {
  int x = klee_int("x");
  int f[x];
  // CHECK: SymbolicArrayOnStack.c:[[@LINE+2]]: memory error: out of bound pointer
  // CHECK-NOT: SymbolicArrayOnStack.c:[[@LINE+1]]: memory error: null pointer exception
  f[0] = 10;
}