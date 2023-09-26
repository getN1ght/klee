// #include "SolverTheory.h"

// #include "SolverAdapter.h"
// #include "klee/ADT/Ref.h"

// #include "klee/Expr/ExprHashMap.h"

// using namespace klee;

// SolverTheory::SolverTheory(Sort theorySort,
//                            const ref<SolverAdapter> &solverAdapter)
//     : theorySort(theorySort), solverAdapter(solverAdapter) {
//   // castMapping[ARRAYS] = &SolverTheory::castToArray;
//   // castMapping[BV] = &SolverTheory::castToBV;
//   // castMapping[BOOL] = &SolverTheory::castToBool;
//   // castMapping[FPBV] = &SolverTheory::castToFPBV;
//   // castMapping[LIA] = &SolverTheory::castToLIA;
// }

// // ref<TheoryHandle> SolverTheory::castToArray(const ref<TheoryHandle> &arg) {
// //   return nullptr;
// // }
// // ref<TheoryHandle> SolverTheory::castToBV(const ref<TheoryHandle> &arg) {
// //   return nullptr;
// // }
// // ref<TheoryHandle> SolverTheory::castToBool(const ref<TheoryHandle> &arg) {
// //   return nullptr;
// // }
// // ref<TheoryHandle> SolverTheory::castToFPBV(const ref<TheoryHandle> &arg) {
// //   return nullptr;
// // }
// // ref<TheoryHandle> SolverTheory::castToLIA(const ref<TheoryHandle> &arg) {
// //   return nullptr;
// // }

// // ref<TheoryHandle> SolverTheory::castTo(SolverTheory::Sort sort,
// //                                        const ref<TheoryHandle> &arg) {
// //   if (castMapping.count(sort) == 0) {
// //     return nullptr;
// //   }
// //   const cast_function_t castFunction = castMapping.at(sort);
// //   return (this->*castFunction)(arg);
// // }

// ref<SolverHandle> CompleteTheoryHandle::expr() const { return handle; }

// ref<CompleteTheoryHandle>
// IncompleteResponse::complete(const TheoryHandleProvider &required) {
//   for (const ref<Expr> &expr : toBuild) {
//     if (required.count(expr) == 0) {
//       // return this;
//       llvm::errs() << "Incomplete response error\n";
//       std::abort();
//     }
//   }

//   ref<CompleteTheoryHandle> completedTheoryHandle =
//       new CompleteTheoryHandle(completer(required), parent);
//   notifyAll({source, completedTheoryHandle});
//   return completedTheoryHandle;
// }
