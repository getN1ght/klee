#include "gtest/gtest.h"

#include "klee/Expr/ExprBuilder.h"
#include "klee/Expr/Parser/Parser.h"
#include "klee/Expr/SymbolicSource.h"

#include "llvm/Support/MemoryBuffer.h"

#include <cassert>
#include <fstream>
#include <functional>
#include <vector>

using namespace klee;
using namespace expr;

namespace {

class ParsingTest : public ::testing::Test {
private:
  std::vector<Decl *> decls;

  std::unique_ptr<llvm::MemoryBuffer>
  readFile(const std::filesystem::path &path) {
    auto file = llvm::MemoryBuffer::getFile(path.string());
    assert(file.getError().value() == 0);
    return std::move(file.get());
  }

  std::vector<std::optional<Decl *>>
  parseFromFile(const std::filesystem::path &path) {
    llvm::errs() << path << '\n';
    assert(std::filesystem::exists(path));

    // Set up expression builder
    ExprBuilder *Builder = createDefaultExprBuilder();

    // Read the file
    auto fileContent = readFile(path);

    // Set up parser
    Parser *P = Parser::Create(path, fileContent.get(), Builder, true);

    // Read all declaration from file
    std::vector<std::optional<Decl *>> Decls;
    while (Decl *D = P->ParseTopLevelDecl()) {
      // Put `Some` if parsing succeeded, and `None` otherwise.
      // NB: do not forget to  validate that all declarations are valid in
      // test suite.
      Decls.push_back(P->GetNumErrors() == 0 ? std::optional(D) : std::nullopt);
    }

    // Clean up parser and builder
    delete P;
    delete Builder;

    return Decls;
  }

protected:
  void SetUp(const std::filesystem::path &path) {
    unsigned declNum = 0;
    for (const auto &wrappedDecl : parseFromFile(path)) {
      ++declNum;
      ASSERT_TRUE(wrappedDecl.has_value())
          << "Parsing for declaration with number " << declNum << " failed";
      decls.push_back(std::move(wrappedDecl.value()));
    }
  }

  virtual void TearDown() {
    for (auto decl : decls) {
      delete decl;
    }
  }

  const std::vector<Decl *> &declarations() const { return decls; }
};

TEST_F(ParsingTest, SymbolicSizeConstantAddressSourceFromInstruction) {
  SetUp(std::filesystem::current_path() /
        "unittests/Parsing/resources/"
        "SimpleSymbolicSizeConstantAddressSource.kquery");
  // File contains only one array source
  ASSERT_EQ(declarations().size(), 1) << "Expected only 1 declaration in file";
  ASSERT_TRUE(isa<ArrayDecl>(declarations().front()))
      << "Expected array declaration";

  auto arrayDecl = cast<ArrayDecl>(declarations().front());
  auto arraySource = arrayDecl->Root->source;
  // Check source
  ASSERT_TRUE(isa<SymbolicSizeConstantAddressSource>(arraySource))
      << "Expected array source with symbolic size and constant address";

  auto symbolicSizeConstantAddressSource =
      cast<SymbolicSizeConstantAddressSource>(arraySource);
}

} // namespace
