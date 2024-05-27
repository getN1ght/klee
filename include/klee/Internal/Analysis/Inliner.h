#ifndef INLINER_H
#define INLINER_H

#include <string>
#include <vector>

#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

#include "ReachabilityAnalysis.h"

class Inliner {
public:
  Inliner(llvm::Module *module, ReachabilityAnalysis *ra,
          const std::vector<std::string> &targets,
          const std::vector<std::string> &functions, llvm::raw_ostream &debugs)
      : module(module), ra(*ra), targets(targets), functions(functions),
        debugs(debugs) {}

  ~Inliner() = default;

  void run();

private:
  void inlineCalls(llvm::Function *f,
                   const std::vector<std::string> &functions);

  llvm::Module *module;
  ReachabilityAnalysis &ra;
  std::vector<std::string> targets;
  std::vector<std::string> functions;
  llvm::raw_ostream &debugs;
};

#endif /* INLINER_H */
