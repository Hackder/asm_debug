#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
  public:
    explicit MyASTVisitor(ASTContext *Context, Rewriter &R)
        : Context(Context), TheRewriter(R) {}

    bool VisitStmt(Stmt *S) {
        if (auto *Asm = dyn_cast<MSAsmStmt>(S)) {
            // Get the assembly string
            StringRef AsmString = Asm->getAsmString();

            SourceLocation StartLoc = Asm->getBeginLoc();
            SourceLocation EndLoc = Asm->getEndLoc();

            // This will segfault
            bool result =
                TheRewriter.ReplaceText(SourceRange(StartLoc, EndLoc), "asdf");
            llvm::errs() << "Replace result: " << result << "\n";
        }
        return true;
    }

  private:
    ASTContext *Context;
    Rewriter &TheRewriter;
};

class MyASTConsumer : public ASTConsumer {
  public:
    explicit MyASTConsumer(ASTContext *Context)
        : TheRewriter(Context->getSourceManager(), Context->getLangOpts()),
          Visitor(Context, TheRewriter) {}

    virtual void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

  private:
    Rewriter TheRewriter;
    MyASTVisitor Visitor;
};

class MyPluginAction : public PluginASTAction {
  protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   llvm::StringRef) override {
        return std::make_unique<MyASTConsumer>(&CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance &CI,
                   const std::vector<std::string> &args) override {
        return true;
    }
};

} // namespace

static FrontendPluginRegistry::Add<MyPluginAction>
    X("asm_debug", "Inject debug steps into inline assembly");
