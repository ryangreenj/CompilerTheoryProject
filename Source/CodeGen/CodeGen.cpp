#include "CodeGen/CodeGen.h"

#include <fstream>
#include <map>
#include <memory>

#include "Utilities/Error.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Transforms/Scalar/Reassociate.h"

using namespace Error;
using namespace llvm;

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;

//static std::map<std::string, Value *> NamedValues;

const int NUM_BITS = 32;

// Create an alloca in global entry block
static AllocaInst *CreateEntryBlockAlloca(const std::string &VarName, Type *VarType)
{
    return Builder->CreateAlloca(VarType, 0, VarName.c_str());
}

// Create an alloca instruction in entry block of a function.
static AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const std::string &VarName, Type *VarType)
{
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(VarType, 0, VarName.c_str());
}

void CodeGen::InitCodeGen()
{
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("prog", *TheContext);

    Builder = std::make_unique<IRBuilder<>>(*TheContext);

    FunctionType *FT = FunctionType::get(IntType(), false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, "main", TheModule.get());

    BasicBlock *BB = BasicBlock::Create(*TheContext, "program", F);
    Builder->SetInsertPoint(BB);

    InitPutFloat();
}

void CodeGen::EndCodeGen()
{
    Builder->CreateRet(IntExpr(0)); // Main returns 0
}

void CodeGen::Print()
{
    TheModule->print(errs(), nullptr);
}

void CodeGen::Out()
{
    std::ofstream StdOutputFile("test.ll");
    raw_os_ostream OutputFile(StdOutputFile);
    TheModule->print(OutputFile, nullptr);
}

void CodeGen::InitPutFloat()
{
    // https://stackoverflow.com/questions/30234027/how-to-call-printf-in-llvm-through-the-module-builder-system
    /*Function *func_putFloat = TheModule->getFunction("putfloat");
    if (!func_putFloat)
    {
        PointerType *Pty = PointerType::get(IntegerType::get(TheModule->getContext(), 8), 0)
    }*/

    // https://stackoverflow.com/questions/35526075/llvm-how-to-implement-print-function-in-my-language
    /*FunctionCallee CalleeF = */TheModule->getOrInsertFunction("printf",
        FunctionType::get(IntegerType::getInt32Ty(*TheContext), PointerType::get(Type::getInt8Ty(*TheContext), 0), true /* this is var arg func type*/)
    );
}

Value *CodeGen::BoolExpr(bool value)
{
    return ConstantInt::get(*TheContext, APInt(1, value));
}

Value *CodeGen::IntExpr(int value)
{
    return ConstantInt::get(*TheContext, APInt(NUM_BITS, value, true));
}

Value *CodeGen::FloatExpr(double value)
{
    return ConstantFP::get(*TheContext, APFloat(value));
}

Value *CodeGen::StringExpr(std::string value)
{
    return Builder->CreateGlobalStringPtr(StringRef(value));
}

Value *CodeGen::VariableExpr(std::string name)
{
    AllocaInst *A = SymbolTable::GetIRAllocaInst(name);
    if (!A)
    {
        ReportError(ERROR_SYMBOL_DOESNT_EXIST);
    }

    // Load value from the stack
    return Builder->CreateLoad(A->getAllocatedType(), A, name.c_str());
}

Value *CodeGen::NegateExpr(Value *value)
{
    return Builder->CreateNeg(value, "negtmp");
}

Value *CodeGen::TermExpr(Value *LHS, Value *RHS, TOKEN_TYPE op)
{
    if (!LHS || !RHS)
    {
        return nullptr; // TODO: Maybe throw error
    }

    if (op == T_MULTIPLY)
    {
        return Builder->CreateFMul(LHS, RHS, "multmp");
    }
    else if (op == T_DIVIDE)
    {
        return Builder->CreateFDiv(LHS, RHS, "divtmp");
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
        return Builder->CreateFCmpULT(LHS, RHS, "cmptmp");
    case T_GREATERTHANEQUALTO:
        return Builder->CreateFCmpUGE(LHS, RHS, "cmptmp");
    case T_LESSTHANEQUALTO:
        return Builder->CreateFCmpULE(LHS, RHS, "cmptmp");
    case T_GREATERTHAN:
        return Builder->CreateFCmpUGT(LHS, RHS, "cmptmp");
    case T_EQUALS:
        return Builder->CreateFCmpUEQ(LHS, RHS, "cmptmp");
    case T_NOTEQUALS:
        return Builder->CreateFCmpUNE(LHS, RHS, "cmptmp");
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
        return Builder->CreateFAdd(LHS, RHS, "addtmp");
    case T_SUBTRACT:
        return Builder->CreateFSub(LHS, RHS, "subtmp");
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

    // Symbol table does this
    /*if (F->arg_size() != args.size())
    {
        // TODO: Throw error
        return nullptr;
    }*/ 

    return Builder->CreateCall(F, args, "calltmp");
}

Value *CodeGen::VariableDeclaration(std::string name, ValueType type, bool hasGlobal)
{
    Value *InitVal = nullptr;
    Type *T = nullptr;
    switch (type)
    {
    case ValueType::BOOL:
        InitVal = BoolExpr(false);
        T = BoolType();
        break;
    case ValueType::INT:
        InitVal = IntExpr(0);
        T = IntType();
        break;
    case ValueType::DOUBLE:
        InitVal = FloatExpr(0.0);
        T = DoubleType();
        break;
    case ValueType::STRING: // TODO
        break;
    default:
        break;
    }

    AllocaInst *Alloca = nullptr;

    Function *TheFunction = Builder->GetInsertBlock()->getParent();
    Alloca = CreateEntryBlockAlloca(TheFunction, name, T);

    /*if (hasGlobal)
    {
        Alloca = CreateEntryBlockAlloca(name, T);
    }
    else
    {
        BasicBlock *BB = Builder->GetInsertBlock();

        if (!BB)
        {
            Alloca = CreateEntryBlockAlloca(name, T);
        }
        else
        {
            Function *TheFunction = Builder->GetInsertBlock()->getParent();

            if (!TheFunction)
            {
                Alloca = CreateEntryBlockAlloca(name, T);
            }
            else
            {
                Alloca = CreateEntryBlockAlloca(TheFunction, name, T);
            }
        }
    }
    */

    Builder->CreateStore(InitVal, Alloca);

    SymbolTable::SetIRAllocaInst(name, Alloca);

    return Alloca;
}

Value *CodeGen::AssignmentStatement(std::string name, llvm::Value *RHS)
{
    Value *V = SymbolTable::GetIRAllocaInst(name);

    if (!V)
    {
        // TODO: Throw error
    }

    Builder->CreateStore(RHS, V);
    return RHS;
}



Type *CodeGen::BoolType()
{
    return Type::getInt1Ty(*TheContext);
}

Type *CodeGen::IntType()
{
    return Type::getInt32Ty(*TheContext);
}

Type *CodeGen::DoubleType()
{
    return Type::getDoubleTy(*TheContext);
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
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", F);
    Builder->SetInsertPoint(BB);

    // Insert function arguments into NamedValues map, might need to clear
    for (auto &Arg : F->args())
    {
        AllocaInst *Alloca = CreateEntryBlockAlloca(F, std::string(Arg.getName()), Arg.getType());
        Builder->CreateStore(&Arg, Alloca);

        SymbolTable::SetIRAllocaInst(std::string(Arg.getName()), Alloca);
    }

    // TODO: Return logic, probably split this out into procedure body gen

    //verifyFunction(*F);

    return F;
}