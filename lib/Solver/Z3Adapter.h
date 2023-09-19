#ifndef Z3ADAPTER_H
#define Z3ADAPTER_H

#include "SolverAdapter.h"
#include "SolverTheory.h"

#include "z3++.h"
#include "llvm/ADT/APInt.h"

namespace klee {

template <typename> class ref;

class Z3SolverHandle : public SolverHandle {
public:
  const z3::expr expr;
  Z3SolverHandle(const z3::expr &);
};

class Z3SortSolverHandle : public SortHandle {
public:
  const z3::sort sort;
  Z3SortSolverHandle(const z3::sort &);
};

class Z3Adapter : public SolverAdapter {
private:
  z3::context ctx;

public:
  ref<SortHandle> bvSort(uint64_t width) override;
  ref<SortHandle> arraySort(const ref<SortHandle> &,
                            const ref<SortHandle> &) override;
  ref<SortHandle> liaSort() override;
  ref<SortHandle> boolSort() override;

public:
  ref<SolverHandle> array(const std::string &name,
                          const ref<SortHandle> &) override;
  ref<SolverHandle> read(const ref<SolverHandle> &,
                         const ref<SolverHandle> &) override;
  ref<SolverHandle> write(const ref<SolverHandle> &, const ref<SolverHandle> &,
                          const ref<SolverHandle> &) override;

public:
  ref<SolverHandle> bvConst(const llvm::APInt &val) override;
  ref<SolverHandle> bvAdd(const ref<SolverHandle> &lhs,
                        const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvSub(const ref<SolverHandle> &lhs,
                        const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvMul(const ref<SolverHandle> &lhs,
                        const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvUDiv(const ref<SolverHandle> &lhs,
                         const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvSDiv(const ref<SolverHandle> &lhs,
                         const ref<SolverHandle> &rhs) override;

  ref<SolverHandle> bvAnd(const ref<SolverHandle> &lhs,
                        const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvOr(const ref<SolverHandle> &lhs,
                       const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvXor(const ref<SolverHandle> &lhs,
                        const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvNot(const ref<SolverHandle> &arg) override;

  ref<SolverHandle> bvSle(const ref<SolverHandle> &lhs,
                        const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvSlt(const ref<SolverHandle> &lhs,
                        const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvUle(const ref<SolverHandle> &lhs,
                        const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> bvUlt(const ref<SolverHandle> &lhs,
                        const ref<SolverHandle> &rhs) override;

  ref<SolverHandle> bvSExt(const ref<SolverHandle> &arg,
                         const ref<SolverHandle> &width) override;
  ref<SolverHandle> bvZExt(const ref<SolverHandle> &arg,
                         const ref<SolverHandle> &width) override;

  ref<SolverHandle> bvShl(const ref<SolverHandle> &arg,
                        const ref<SolverHandle> &width) override;
  ref<SolverHandle> bvLShr(const ref<SolverHandle> &arg,
                         const ref<SolverHandle> &width) override;
  ref<SolverHandle> bvAShr(const ref<SolverHandle> &arg,
                         const ref<SolverHandle> &width) override;

  ref<SolverHandle> bvExtract(const ref<SolverHandle> &expr,
                            const ref<SolverHandle> &lb,
                            const ref<SolverHandle> &rb) override;
  ref<SolverHandle> bvConcat(const ref<SolverHandle> &lhs,
                           const ref<SolverHandle> &rhs) override;

public:
  ref<SolverHandle> propAnd(const ref<SolverHandle> &lhs,
                          const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> propOr(const ref<SolverHandle> &lhs,
                         const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> propXor(const ref<SolverHandle> &lhs,
                          const ref<SolverHandle> &rhs) override;
  ref<SolverHandle> propNot(const ref<SolverHandle> &arg) override;
  ref<SolverHandle> propIte(const ref<SolverHandle> &cond,
                          const ref<SolverHandle> &onTrue,
                          const ref<SolverHandle> &onFalse) override;

  ~Z3Adapter() = default;
};

}; // namespace klee

#endif
