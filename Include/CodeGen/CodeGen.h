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

    static void Runtime();

    static llvm::Value *BoolExpr(bool value);
    static llvm::Value *IntExpr(int value);
    static llvm::Value *FloatExpr(double value);
    static llvm::Value *StringExpr(std::string value);
    static llvm::Value *VariableExpr(std::string name, llvm::Value *index = nullptr);
    static llvm::Value *NegateExpr(llvm::Value *value);
    static llvm::Value *TermExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op); // Gonna have to break this up for different types (bool, int, float, string)
    static llvm::Value *FactorExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op);
    static llvm::Value *ArithOpExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op);
    static llvm::Value *ExprExpr(llvm::Value *LHS, llvm::Value *RHS, TOKEN_TYPE op);
    static llvm::Value *ProcedureCall(std::string name, std::vector<llvm::Value *> args);
    static void VariableDeclaration(std::string name, ValueType type, bool hasGlobal, int arraySize = 0);
    static llvm::Value *AssignmentStatement(std::string name, llvm::Value *index, llvm::Value *RHS);
    static llvm::Value *ReturnStatement(llvm::Value *RHS);
    static void IfStatement(llvm::Value *Condition, llvm::BasicBlock *&ThenBBOut, llvm::BasicBlock *&ElseBBOut, llvm::BasicBlock *&MergeBBOut, llvm::Function *&TheFunctionOut);
    static void ElseStatement(llvm::BasicBlock *&ThenBBOut, llvm::BasicBlock *&ElseBBOut, llvm::BasicBlock *&MergeBBOut, llvm::Function *&TheFunctionOut);
    static void EndIfStatement(llvm::BasicBlock *ThenBB, llvm::BasicBlock *ElseBB, llvm::BasicBlock *MergeBB, llvm::Function *TheFunction);
    static void ForStatementHeader(llvm::BasicBlock *&ForCheckBBOut, llvm::BasicBlock *&LoopBBOut, llvm::BasicBlock *&AfterForBBOut, llvm::Function *&TheFunctionOut);
    static void ForStatementCheck(llvm::Value *LoopCondition, llvm::BasicBlock *&ForCheckBBOut, llvm::BasicBlock *&LoopBBOut, llvm::BasicBlock *&AfterForBBOut, llvm::Function *&TheFunctionOut);
    static void EndForStatement(llvm::BasicBlock *ForCheckBB, llvm::BasicBlock *LoopBB, llvm::BasicBlock *AfterForBB, llvm::Function *TheFunction);

    static llvm::Type *BoolType();
    static llvm::Type *IntType();
    static llvm::Type *DoubleType();
    static llvm::Type *StringType();

    static llvm::Function *ProcedureHeader(std::string name, llvm::Type *retType, std::vector<std::string> argNames, std::vector<llvm::Type *> argTypes);
    static llvm::Function *ProcedureDeclaration(llvm::Function *F);
    static llvm::Function *ProcedureEnd(llvm::Function *F);
private:
    static void GetTypeAndInitVal(ValueType type, llvm::Value *&InitValOut, llvm::Type *&TypeOut, int &AlignNum);
    static llvm::Value *CheckIfValueTrue(llvm::Value *ValIn, std::string CondName);
    static llvm::Value *ConvertType(llvm::Value *Destination, llvm::Value *RHS);
    static llvm::Value *ConvertType(llvm::Type *DestinationType, llvm::Value *RHS);
    static llvm::Value *ConvertToDouble(llvm::Value *Val);
};

#endif