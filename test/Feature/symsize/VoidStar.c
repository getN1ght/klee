// RUN: %clang %s -emit-llvm %O0opt -c -o %t1.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --max-symsize=128 --skip-not-lazy-and-symbolic-pointers --output-dir=%t.klee-out %t1.bc

#include "klee/klee.h"
#include <stdlib.h>
#include <stdio.h>

struct Node {
  int x;
  struct Node *next;
};

void foo(void *s) {
  // CHECK: VoidStar.c:[[@LINE+1]]: memory error: out of bound pointer
  ((int *) s)[1] = 10;
  // CHECK-NOT: VoidStar.c:[[@LINE+1]]: memory error: out of bound pointer
  ((char *) s) [1] = 'a';
  // CHECK: VoidStar.c:[[@LINE+1]]: memory error: out of bound pointer
  struct Node *node = ((struct Node *) s)->next;
  // CHECK-NOT: VoidStar.c:[[@LINE+1]]: memory error: out of bound pointer
  node->x = 20;
}

int main() {
  int n = klee_int("n");
  void *s = malloc(n);
  klee_make_symbolic(s, n, "s");
  foo(s);
}

// CHECK: KLEE: done: completed paths = 1
// CHECK: KLEE: done: partially completed paths = 3
// CHECK: KLEE: done: generated tests = 4
