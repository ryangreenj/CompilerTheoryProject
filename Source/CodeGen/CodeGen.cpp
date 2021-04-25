#include "CodeGen/CodeGen.h"

#include <fstream>
#include <map>
#include <memory>
#include <sstream>

#include "Utilities/Error.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace Error;
using namespace llvm;

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;

static std::stack<BasicBlock *> Blocks;

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

static void SetBasicBlock(BasicBlock *BB)
{
    Blocks.push(BB);
    Builder->SetInsertPoint(BB);
}

static void EndBasicBlock()
{
    Blocks.pop();
    Builder->SetInsertPoint(Blocks.top());
}

void CodeGen::GetTypeAndInitVal(ValueType type, Value *&InitValOut, Type *&TypeOut)
{
    switch (type)
    {
    case ValueType::BOOL:
        InitValOut = BoolExpr(false);
        TypeOut = BoolType();
        break;
    case ValueType::INT:
        InitValOut = IntExpr(0);
        TypeOut = IntType();
        break;
    case ValueType::DOUBLE:
        InitValOut = FloatExpr(0.0);
        TypeOut = DoubleType();
        break;
    case ValueType::STRING:
        InitValOut = StringExpr("");
        TypeOut = StringType();
        break;
    default:
        break; // TODO: Arrays
    }
}

void CodeGen::InitCodeGen()
{
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("prog", *TheContext);

    Builder = std::make_unique<IRBuilder<>>(*TheContext);

    FunctionType *FT = FunctionType::get(IntType(), false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, "main", TheModule.get());

    BasicBlock *BB = BasicBlock::Create(*TheContext, "program", F);
    SetBasicBlock(BB);

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

void CodeGen::Out(std::string outFileName)
{
#ifdef _WIN64
    std::ofstream StdOutputFile("test.ll");
    raw_os_ostream OutputFile(StdOutputFile);
    TheModule->print(OutputFile, nullptr);
#else

    auto TargetTriple = sys::getDefaultTargetTriple();

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target)
    {
        errs() << Error;
        return; // TODO: Throw error
    }

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    TheModule->setDataLayout(TargetMachine->createDataLayout());
    TheModule->setTargetTriple(TargetTriple);

    auto Filename = "tempOutFile.o";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::F_None);

    if (EC)
    {
        errs() << "Could not open file: " << EC.message();
        return; // TODO: Throw error
    }

    legacy::PassManager pass;
    auto FileType = TargetMachine::CGFT_ObjectFile;

    if (TargetMachine->addPassesToEmitFile(pass, dest, FileType))
    {
        errs() << "TargetMachine can't emit a file of this type";
        return; // TODO: Throw error
    }

    pass.run(*TheModule);
    dest.flush();

    std::stringstream ss;
    ss << "gcc " << Filename << " -no-pie -o " << outFileName;

    std::system(ss.str().c_str()); // compile
    std::remove(Filename);
#endif
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
    TheModule->getOrInsertFunction("printf", FunctionType::get(IntegerType::getInt32Ty(*TheContext), PointerType::get(Type::getInt8Ty(*TheContext), 0), true));
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
    GetTypeAndInitVal(type, InitVal, T);

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

    // TODO: Use hasGlobal
    SymbolTable::SetIRAllocaInst(name, Alloca);

    return Alloca;
}

Value *CodeGen::AssignmentStatement(std::string name, Value *RHS)
{
    Value *V = SymbolTable::GetIRAllocaInst(name);

    if (!V)
    {
        // TODO: Throw error
    }

    Builder->CreateStore(RHS, V);
    return RHS;
}

Value *CodeGen::ReturnStatement(Value *RHS)
{
    AllocaInst *RetAllocaInst = SymbolTable::GetReturnAllocaInst();

    Builder->CreateStore(RHS, RetAllocaInst);
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
    return Type::getInt8PtrTy(*TheContext);
}



Function *CodeGen::ProcedureHeader(std::string name, Type *retType, std::vector<std::string> argNames, std::vector<Type *> argTypes)
{
    FunctionType *FT = FunctionType::get(retType, argTypes, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, name, TheModule.get()); // TODO: Change ExternalLinkage for scoping or maybe not

    // Set names of arguments
    int i = 0;
    for (auto &Arg : F->args())
    {
        Arg.setName(argNames[i++]);
    }

    return F;
}

Function *CodeGen::ProcedureDeclaration(Function *F, ValueType retType)
{
    // Create basic block to start insertion into
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", F);
    SetBasicBlock(BB);

    // Insert function arguments into NamedValues map, might need to clear
    for (auto &Arg : F->args())
    {
        AllocaInst *Alloca = CreateEntryBlockAlloca(F, std::string(Arg.getName()), Arg.getType());
        Builder->CreateStore(&Arg, Alloca);

        SymbolTable::SetIRAllocaInst(std::string(Arg.getName()), Alloca);
    }
    
    // Allocate return value and set in symbol table
    Value *InitVal = nullptr;
    Type *T = nullptr;
    GetTypeAndInitVal(retType, InitVal, T);

    AllocaInst *Alloca = nullptr;

    Alloca = CreateEntryBlockAlloca(F, "RET", T);

    Builder->CreateStore(InitVal, Alloca);

    SymbolTable::SetReturnAllocaInst(Alloca);

    // TODO: Return logic, probably split this out into procedure body gen

    //verifyFunction(*F);

    return F;
}

Function *CodeGen::ProcedureEnd(Function *F)
{
    if (AllocaInst *RetAllocaInst = SymbolTable::GetReturnAllocaInst())
    {
        Builder->CreateRet(RetAllocaInst);

        verifyFunction(*F);

        EndBasicBlock();

        return F;
    }

    // Can't read body, remove the function
    F->eraseFromParent();
    EndBasicBlock();
    return nullptr;
}