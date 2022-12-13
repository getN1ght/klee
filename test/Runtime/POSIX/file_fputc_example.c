// RUN: %clang %s -emit-llvm -g %O0opt -c -o %t.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --output-dir=%t.klee-out --posix-runtime %t.bc --sym-stdin 64 --sym-files 1 64
// RUN: test -f %t.klee-out/test000001.ktest
// RUN: test -f %t.klee-out/test000002.ktest
// RUN: test -f %t.klee-out/test000003.ktest
// RUN: test -f %t.klee-out/test000004.ktest
// RUN: test -f %t.klee-out/test000005.ktest
// RUN: test -f %t.klee-out/test000006.ktest
// RUN: not test -f %t.klee-out/test000007.ktest

#include "klee/klee.h"
#include <stdio.h>

char simple_fputc(int x, int y) {
  FILE *fA = fopen("A", "w");
  if (x < y) {
    fputc('<', fA);
    return '<';
  } else if (x > y) {
    fputc('>', fA);
    return '>';
  } else {
    fputc('=', fA);
    return '=';
  }
}

int main() {
  int x, y;
  klee_make_symbolic(&x, sizeof(int), "x");
  klee_make_symbolic(&y, sizeof(int), "y");
  char c = simple_fputc(x, y);
  return 0;
}
