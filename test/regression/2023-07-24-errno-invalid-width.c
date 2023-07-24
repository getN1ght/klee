// REQUIRES: not-darwin
// RUN: %clang %s -g -emit-llvm %O0opt -c -m32 -o %t.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --allocate-determ --allocate-determ-start-address=0x00030000000 --output-dir=%t.klee-out %t.bc 2>&1 | FileCheck %s

#include <stdio.h>

int main() {
  printf("kleenok");
  // CHECK-NOT: KLEE: ERROR
}
