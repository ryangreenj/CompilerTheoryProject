#ifndef _INCL_CODE_GEN
#define _INCL_CODE_GEN

#include <string>

#include "Utilities/Token.h"
#include "Utilities/SymbolTable.h"

#include "llvm/IR/Value.h"

class CodeGen
{
public:
    //CodeGen();
    static void InitCodeGen();
    static void EndCodeGen();
    static void Print();
    static void Out(std::string outFileName);

    static void InitPutFloat();

    static llvm::Value *BoolExpr(bool value);
    static llvm::Value *IntExpr(int value);
    static llvm::Value *FloatExpr(double value);
    static llvm::Value *StringExpr(std::string value);
    static llvm::Value *VariableExpr(std::string name);
    static llvm::Value *NegateExpr(llvm::Value *value);
    static llvm::Value *TermExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op); // Gonna have to break this up for different types (bool, int, float, string)
    static llvm::Value *FactorExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op);
    static llvm::Value *ArithOpExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op);
    static llvm::Value *ExprExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op);
    static llvm::Value *ProcedureCall(std::string name, std::vector<llvm::Value *> args);
    static llvm::Value *VariableDeclaration(std::string name, ValueType type, bool hasGlobal = false);
    static llvm::Value *AssignmentStatement(std::string name, llvm::Value *RHS);
    static llvm::Value *ReturnStatement(llvm::Value *RHS);
    static void IfStatement(llvm::Value *Condition, llvm::BasicBlock *&ThenBBOut, llvm::BasicBlock *&ElseBBOut, llvm::BasicBlock *&MergeBBOut, llvm::Function *&TheFunctionOut);
    static void ElseStatement(llvm::BasicBlock *&ThenBBOut, llvm::BasicBlock *&ElseBBOut, llvm::BasicBlock *&MergeBBOut, llvm::Function *&TheFunctionOut);
    static void EndIfStatement(llvm::BasicBlock *ThenBB, llvm::BasicBlock *ElseBB, llvm::BasicBlock *MergeBB, llvm::Function *TheFunction);

    static llvm::Type *BoolType();
    static llvm::Type *IntType();
    static llvm::Type *DoubleType();
    static llvm::Type *StringType();

    static llvm::Function *ProcedureHeader(std::string name, llvm::Type *retType, std::vector<std::string> argNames, std::vector<llvm::Type *> argTypes);
    static llvm::Function *ProcedureDeclaration(llvm::Function *F, ValueType retType);
    static llvm::Function *ProcedureEnd(llvm::Function *F);
private:
    static void GetTypeAndInitVal(ValueType type, llvm::Value *&InitValOut, llvm::Type *&TypeOut);
};

#endif