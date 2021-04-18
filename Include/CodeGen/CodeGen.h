#ifndef _INCL_CODE_GEN
#define _INCL_CODE_GEN

#include <string>

#include "Utilities/Token.h"

#include "llvm/IR/Value.h"

class CodeGen
{
public:
    //CodeGen();

    static llvm::Value *BoolExpr(bool value);
    static llvm::Value *IntExpr(int value);
    static llvm::Value *FloatExpr(double value);
    static llvm::Value *VariableExpr(std::string name);
    static llvm::Value *NegateExpr(llvm::Value *value);
    static llvm::Value *TermExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op); // Gonna have to break this up for different types (bool, int, float, string)
    static llvm::Value *FactorExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op);
    static llvm::Value *ArithOpExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op);
    static llvm::Value *ExprExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op);
    static llvm::Value *ProcedureCall(std::string name, std::vector<llvm::Value *> args);

    static llvm::Type *BoolType();
    static llvm::Type *IntType();
    static llvm::Type *DoubleType();
    static llvm::Type *StringType();

    static llvm::Function *ProcedureHeader(std::string name, llvm::Type *retType, std::vector<std::string> argNames, std::vector<llvm::Type *> argTypes);
    static llvm::Function *ProcedureDeclaration(llvm::Function *F);
private:
    
};

#endif