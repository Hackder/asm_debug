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

            const SourceManager &SM = Context->getSourceManager();
            SourceLocation Loc = Asm->getBeginLoc();
            PresumedLoc PLoc = SM.getPresumedLoc(Loc);
            if (!PLoc.isValid()) {
                llvm::errs()
                    << "Failed to obtain filename from asm block" << "\n";
            }

            // Print the current assembly for debugging
            llvm::errs() << "Original Assembly: " << AsmString << "\n";

            // Modify the assembly (example: append a comment)
            std::string ModifiedAsm = "__asm {\n";
            ModifiedAsm += ".file 1 ";
            ModifiedAsm += "\"";
            ModifiedAsm += PLoc.getFilename();
            ModifiedAsm += "\"";
            ModifiedAsm += "\n.loc 1 7\n";
            ModifiedAsm += AsmString;

            char *buffer = new char[ModifiedAsm.length() + 1];
            memset(buffer, 0, ModifiedAsm.length() + 1);
            memcpy(buffer, ModifiedAsm.c_str(), ModifiedAsm.length());

            llvm::errs() << "New Assembly: " << ModifiedAsm << "\n";

            SourceLocation StartLoc = Asm->getBeginLoc();
            // StartLoc = TheRewriter.getSourceMgr().getFileLoc(StartLoc);
            SourceLocation EndLoc = Asm->getEndLoc();
            // EndLoc = TheRewriter.getSourceMgr().getFileLoc(EndLoc);
            //
            StartLoc.dump(SM);
            EndLoc.dump(SM);

            StartLoc = SM.getSpellingLoc(StartLoc);
            EndLoc = SM.getSpellingLoc(EndLoc);

            StartLoc.dump(SM);
            EndLoc.dump(SM);

            if (StartLoc.isInvalid() || EndLoc.isInvalid()) {
                llvm::errs() << "Invalid source location.\n";
                return true;
            }

            if (!SM.isBeforeInTranslationUnit(StartLoc, EndLoc)) {
                llvm::errs()
                    << "Invalid range: StartLoc is not before EndLoc.\n";
                return true; // Skip this asm statement
            }

            if (SM.getFileID(StartLoc) != SM.getFileID(EndLoc)) {
                llvm::errs() << "Invalid range: StartLoc and EndLoc are in "
                                "different files.\n";
                return true; // Skip this asm statement
            }

            llvm::errs() << StartLoc.isMacroID() << "\n";
            llvm::errs() << StartLoc.isMacroID() << "\n";

            llvm::errs() << "StartLoc: ";
            int size = TheRewriter.getRangeSize(SourceRange(StartLoc, EndLoc));
            llvm::errs() << size << "\n";

            if (SM.isBeforeInTranslationUnit(StartLoc, EndLoc)) {
                llvm::errs() << "Valid range.\n";
            } else {
                llvm::errs() << "Invalid range.\n";
            }

            bool result =
                TheRewriter.ReplaceText(SourceRange(StartLoc, EndLoc), buffer);
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
    explicit MyASTConsumer(ASTContext *Context, Rewriter R)
        : Visitor(Context, R) {}

    virtual void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

  private:
    MyASTVisitor Visitor;
};

class MyPluginAction : public PluginASTAction {
  protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   llvm::StringRef) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<MyASTConsumer>(&CI.getASTContext(),
                                               TheRewriter);
    }

    bool ParseArgs(const CompilerInstance &CI,
                   const std::vector<std::string> &args) override {
        return true;
    }

  private:
    Rewriter TheRewriter;
};

} // namespace

static FrontendPluginRegistry::Add<MyPluginAction>
    X("asm_debug", "Inject debug steps into inline assembly");
