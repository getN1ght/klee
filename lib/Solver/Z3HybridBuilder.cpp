#include "Z3HybridBuilder.h"

#include "Z3Builder.h"
#include "Z3LIABuilder.h"

using namespace klee;

Z3HybridBuilder::Z3HybridBuilder(bool autoClearConstructCache,
                                 const char *z3LogInteractionFile)
    : Z3Builder(autoClearConstructCache, z3LogInteractionFile) {
  // pass
}

Z3ASTHandle Z3HybridBuilder::construct(ref<Expr> e, int *width_out) {
  if (!Z3HashConfig::UseConstructHashZ3 || isa<ConstantExpr>(e)) {
    return constructActual(e, width_out);
  } else {
    auto liaIterCache = _liaCache.find(e);
    auto bvIterCache = _bvCache.find(e);

    if (liaIterCache != _liaCache.end()) {
      if (width_out) {
        *width_out = liaIterCache->second.second;
      }
      return liaIterCache->second.first;
    }

    if (bvIterCache != _bvCache.end()) {
      if (width_out) {
        *width_out = bvIterCache->second.second;
      }
      return bvIterCache->second.first;
    }

    int width;
    if (!width_out) {
      width_out = &width;
    }

    // TODO: construct 2 versions.
    Z3ASTHandle res = constructActual(e, width_out);
    // TODO:
    // constructed.emplace(e, std::make_pair(res, *width_out));
    return res;
  }
}

Z3ASTHandle Z3HybridBuilder::constructActual(ref<Expr> e, int *width_out) {
  // pass
}

Z3ASTHandle Z3HybridBuilder::constructActualBV(ref<Expr> e, int *width_out) {
  // pass
}

Z3ASTHandle Z3HybridBuilder::constructActualLIA(ref<Expr> e, int *width_out) {
  // pass
}

Z3HybridBuilder::~Z3HybridBuilder() {}
