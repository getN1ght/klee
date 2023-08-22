#ifndef Z3ADAPTER_H
#define Z3ADAPTER_H

#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "llvm/ADT/APInt.h"
#include "z3++.h"

namespace klee {

template <typename> class ref;

class Z3ExprHandle : public ExprHandle {
public:
  const z3::expr expr;
  Z3ExprHandle(const z3::expr &);
};

class Z3Adapter : public SolverAdapter {
private:
  const z3::context ctx;

public:
  ref<ExprHandle> bvConst(const llvm::APInt &val) override;
  ref<ExprHandle> bvAdd(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvSub(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvMul(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvUDiv(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvSDiv(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) override;

  ref<ExprHandle> bvAnd(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvOr(const ref<ExprHandle> &lhs,
                       const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvXor(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvNot(const ref<ExprHandle> &arg) override;

  ref<ExprHandle> bvSle(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvSlt(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvUle(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> bvUlt(const ref<ExprHandle> &lhs,
                        const ref<ExprHandle> &rhs) override;

  ref<ExprHandle> bvSExt(const ref<ExprHandle> &arg,
                         const ref<ExprHandle> &width) override;
  ref<ExprHandle> bvZExt(const ref<ExprHandle> &arg,
                         const ref<ExprHandle> &width) override;

  ref<ExprHandle> bvShl(const ref<ExprHandle> &arg,
                        const ref<ExprHandle> &width) override;
  ref<ExprHandle> bvLShr(const ref<ExprHandle> &arg,
                         const ref<ExprHandle> &width) override;
  ref<ExprHandle> bvAShr(const ref<ExprHandle> &arg,
                         const ref<ExprHandle> &width) override;

  ref<ExprHandle> bvExtract(const ref<ExprHandle> &expr,
                            const ref<ExprHandle> &lb,
                            const ref<ExprHandle> &rb) override;
  ref<ExprHandle> bvConcat(const ref<ExprHandle> &lhs,
                           const ref<ExprHandle> &rhs) override;

public:
  ref<ExprHandle> propAnd(const ref<ExprHandle> &lhs,
                          const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> propOr(const ref<ExprHandle> &lhs,
                         const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> propXor(const ref<ExprHandle> &lhs,
                          const ref<ExprHandle> &rhs) override;
  ref<ExprHandle> propNot(const ref<ExprHandle> &arg) override;
  ref<ExprHandle> propIte(const ref<ExprHandle> &cond,
                          const ref<ExprHandle> &onTrue,
                          const ref<ExprHandle> &onFalse) override;

  Z3Adapter();

  ~Z3Adapter() = default;
};

}; // namespace klee

#endif