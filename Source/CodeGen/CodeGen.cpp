#include "CodeGen/CodeGen.h"

#include <map>
#include <memory>

#include "Utilities/Error.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Scalar/Reassociate.h"

using namespace Error;
using namespace llvm;

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::map<std::string, Value *> NamedValues;

const int NUM_BITS = 32;

Value *CodeGen::BoolExpr(bool value)
{
    return ConstantInt::get(TheContext, APInt(1, value));
}

Value *CodeGen::IntExpr(int value)
{
    return ConstantInt::get(TheContext, APInt(NUM_BITS, value, true));
}

Value *CodeGen::FloatExpr(double value)
{
    return ConstantFP::get(TheContext, APFloat(value));
}

Value *CodeGen::VariableExpr(std::string name)
{
    Value *V = NamedValues[name];
    if (!V)
    {
        ReportError(ERROR_SYMBOL_DOESNT_EXIST);
    }
    return V;
}

Value *CodeGen::NegateExpr(Value *value)
{
    return Builder.CreateNeg(value, "negtmp");
}

Value *CodeGen::TermExpr(Value *LHS, Value *RHS, TOKEN_TYPE op)
{
    if (!LHS || !RHS)
    {
        return nullptr; // TODO: Maybe throw error
    }

    if (op == T_MULTIPLY)
    {
        return Builder.CreateFMul(LHS, RHS, "multmp");
    }
    else if (op == T_DIVIDE)
    {
        return Builder.CreateFDiv(LHS, RHS, "divtmp");
    }
    else
    {
        return nullptr; // TODO: Throw error
    }
}

Value *CodeGen::FactorExpr(Value *LHS, Value *RHS, TOKEN_TYPE op)
{
    if (!LHS || !RHS)
    {
        return nullptr; // TODO: Maybe throw error
    }

    switch (op)
    {
    case T_LESSTHAN:
        return Builder.CreateFCmpULT(LHS, RHS, "cmptmp");
    case T_GREATERTHANEQUALTO:
        return Builder.CreateFCmpUGE(LHS, RHS, "cmptmp");
    case T_LESSTHANEQUALTO:
        return Builder.CreateFCmpULE(LHS, RHS, "cmptmp");
    case T_GREATERTHAN:
        return Builder.CreateFCmpUGT(LHS, RHS, "cmptmp");
    case T_EQUALS:
        return Builder.CreateFCmpUEQ(LHS, RHS, "cmptmp");
    case T_NOTEQUALS:
        return Builder.CreateFCmpUNE(LHS, RHS, "cmptmp");
    default:
        return nullptr; // TODO: Throw error
    }
}

Value *CodeGen::ArithOpExpr(Value *LHS, Value *RHS, TOKEN_TYPE op)
{
    if (!LHS || !RHS)
    {
        return nullptr; // TODO: Maybe throw error
    }

    switch (op)
    {
    case T_ADD:
        return Builder.CreateFAdd(LHS, RHS, "addtmp");
    case T_SUBTRACT:
        return Builder.CreateFSub(LHS, RHS, "subtmp");
    default:
        return nullptr; // TODO: Throw error
    }
}

Value *CodeGen::ExprExpr(Value *LHS, Value *RHS, TOKEN_TYPE op)
{
    if (!LHS || !RHS)
    {
        return nullptr; // TODO: Maybe throw error
    }

    switch (op)
    {
    case T_AND:
        return nullptr; // TODO: Implement
    case T_OR:
        return nullptr;
    default:
        return nullptr; // TODO: Throw error
    }
}

Value *CodeGen::ProcedureCall(std::string name, std::vector<Value *> args)
{
    Function *F = TheModule->getFunction(name);
    if (!F)
    {
        // TODO: Throw error
        return nullptr;
    }

    if (F->arg_size() != args.size())
    {
        // TODO: Throw error
        return nullptr;
    }

    return Builder.CreateCall(F, args, "calltmp");
}



Type *CodeGen::BoolType()
{
    return Type::getInt1Ty(TheContext);
}

Type *CodeGen::IntType()
{
    return Type::getInt32Ty(TheContext);
}

Type *CodeGen::DoubleType()
{
    return Type::getDoubleTy(TheContext);
}

Type *CodeGen::StringType()
{
    return nullptr;
}



Function *CodeGen::ProcedureHeader(std::string name, Type *retType, std::vector<std::string> argNames, std::vector<Type *> argTypes)
{
    FunctionType *FT = FunctionType::get(retType, argTypes, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, name, TheModule.get()); // TODO: Change ExternalLinkage for scoping

    // Set names of arguments
    int i = 0;
    for (auto &Arg : F->args())
    {
        Arg.setName(argNames[i++]);
    }

    return F;
}

Function *CodeGen::ProcedureDeclaration(Function *F)
{
    // Create basic block to start insertion into
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(BB);

    // Insert function arguments into NamedValues map, might need to clear
    for (auto &Arg : F->args())
    {
        NamedValues[std::string(Arg.getName())] = &Arg;
    }

    // TODO: Return logic, probably split this out into procedure body gen

    verifyFunction(*F);

    return F;
}