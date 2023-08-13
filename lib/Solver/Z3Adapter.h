#ifndef Z3ADAPTER_H
#define Z3ADAPTER_H

#include "SolverTheory.h"

class ExprHandle;

class Z3Adapter {
public:
  class Arrays : SolverTheory {
  public:
    static ExprHandle read();
    static ExprHandle write();
  };

  class LIA : SolverTheory {
  public:
    static ExprHandle add(const ExprHandle &lhs, const ExprHandle &rhs);
    static ExprHandle sub(const ExprHandle &lhs, const ExprHandle &rhs);
    static ExprHandle zext();
    static ExprHandle sext();

  };
  
  class BV : SolverTheory {
  public:
    
  };

  
};

#endif