// RUN: %clang %s -g -emit-llvm %O0opt -c -o %t.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --output-dir=%t.klee-out %t.bc 2>&1 | FileCheck %s

#pragma clang attribute push(__attribute__((optnone)), apply_to = function)

int main() {
  int length1 = __VERIFIER_nondet_int();
  int length2 = __VERIFIER_nondet_int();
  if (length1 < 1) {
    length1 = 1;
  }
  if (length2 < 1) {
    length2 = 1;
  }
  char *nondetString1 = (char *)__builtin_alloca(length1 * sizeof(char));
  char *nondetString2 = (char *)__builtin_alloca(length2 * sizeof(char));
  nondetString1[length1 - 1] = '\0';
  // CHECK-NOT: memory error: out of bound pointer
  nondetString2[length2 - 1] = '\0';
}
#pragma clang attribute pop
