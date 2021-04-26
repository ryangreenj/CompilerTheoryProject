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

static bool runtimeGenerated = false;

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

void CodeGen::GetTypeAndInitVal(ValueType type, Value *&InitValOut, Type *&TypeOut, int &AlignNum)
{
    switch (type)
    {
    case ValueType::BOOL:
        InitValOut = BoolExpr(false);
        TypeOut = BoolType();
        AlignNum = 1;
        break;
    case ValueType::INT:
        InitValOut = IntExpr(0);
        TypeOut = IntType();
        AlignNum = 4;
        break;
    case ValueType::DOUBLE:
        InitValOut = FloatExpr(0.0);
        TypeOut = DoubleType();
        AlignNum = 8;
        break;
    case ValueType::STRING:
        InitValOut = StringExpr("");
        TypeOut = StringType();
        AlignNum = 1;
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

    runtimeGenerated = false;

    FunctionType *FT = FunctionType::get(IntType(), false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, "main", TheModule.get());

    BasicBlock *BB = BasicBlock::Create(*TheContext, "program", F);
    SetBasicBlock(BB);

    Runtime();
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

void CodeGen::Runtime()
{
    // https://stackoverflow.com/questions/30234027/how-to-call-printf-in-llvm-through-the-module-builder-system
    /*Function *func_putFloat = TheModule->getFunction("putfloat");
    if (!func_putFloat)
    {
        PointerType *Pty = PointerType::get(IntegerType::get(TheModule->getContext(), 8), 0)
    }*/

    // https://stackoverflow.com/questions/35526075/llvm-how-to-implement-print-function-in-my-language
    TheModule->getOrInsertFunction("printf", FunctionType::get(IntType(), PointerType::get(Type::getInt8Ty(*TheContext), 0), true));

    std::vector<std::string> argNames = { "IN" };
    Function *F = nullptr;

    // putBool
    SymbolTable::AddLevel();
    SymbolTable::Insert("IN", ValueType::BOOL, false, 0);
    F = ProcedureHeader("putbool", BoolType(), argNames, { BoolType() });
    ProcedureDeclaration(F);
    ReturnStatement(ProcedureCall("printf", { StringExpr("%d\n"), VariableExpr("IN") }));
    ProcedureEnd(F);
    SymbolTable::DeleteLevel();

    // putInteger
    SymbolTable::AddLevel();
    SymbolTable::Insert("IN", ValueType::INT, false, 0);
    F = ProcedureHeader("putinteger", BoolType(), argNames, { IntType() });
    ProcedureDeclaration(F);
    ReturnStatement(ProcedureCall("printf", { StringExpr("%d\n"), VariableExpr("IN") }));
    ProcedureEnd(F);
    SymbolTable::DeleteLevel();

    // putFloat
    SymbolTable::AddLevel();
    SymbolTable::Insert("IN", ValueType::DOUBLE, false, 0);
    F = ProcedureHeader("putfloat", BoolType(), argNames, { DoubleType() });
    ProcedureDeclaration(F);
    ReturnStatement(ProcedureCall("printf", { StringExpr("%f\n"), VariableExpr("IN") }));
    ProcedureEnd(F);
    SymbolTable::DeleteLevel();

    // putString
    SymbolTable::AddLevel();
    SymbolTable::Insert("IN", ValueType::STRING, false, 0);
    F = ProcedureHeader("putstring", BoolType(), argNames, { StringType() });
    ProcedureDeclaration(F);
    ReturnStatement(ProcedureCall("printf", { StringExpr("%s\n"), VariableExpr("IN") }));
    ProcedureEnd(F);
    SymbolTable::DeleteLevel();

    // sqrt      Tried using C lib sqrt but too many problems, use LLVM intrinsic instead!
    SymbolTable::AddLevel();
    SymbolTable::Insert("IN", ValueType::INT, false, 0);
    F = ProcedureHeader("sqrt", DoubleType(), argNames, { IntType() });
    ProcedureDeclaration(F);

    std::vector<llvm::Type *> ArgTypes;
    ArgTypes.push_back(DoubleType());

    Function *SQRTInt = Intrinsic::getDeclaration(F->getParent(), Intrinsic::sqrt, ArgTypes);

    std::vector<llvm::Value *> Args;
    Args.push_back(ConvertType(DoubleType(), VariableExpr("IN")));

    ReturnStatement(Builder->CreateCall(SQRTInt, Args));
    ProcedureEnd(F);
    SymbolTable::DeleteLevel();

    

    runtimeGenerated = true;
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
        // Check if it's a global variable
        GlobalVariable *gVar = TheModule->getNamedGlobal(name);

        if (!gVar)
        {
            ReportError(ERROR_SYMBOL_DOESNT_EXIST);
            return nullptr;
        }
        return Builder->CreateLoad(gVar);
    }
    else
    {
        // Load value from the stack
        return Builder->CreateLoad(A->getAllocatedType(), A, name.c_str());
    }
}

Value *CodeGen::NegateExpr(Value *value)
{
    if (value->getType()->isIntegerTy())
    {
        return Builder->CreateMul(value, IntExpr(-1));
    }
    else if (value->getType()->isFloatingPointTy())
    {
        return Builder->CreateFMul(value, FloatExpr(-1.0));
    }
    else
    {
        return nullptr; // TODO: Maybe throw error
    }
}

Value *CodeGen::TermExpr(Value *LHS, Value *RHS, TOKEN_TYPE op)
{
    if (!LHS || !RHS)
    {
        return nullptr; // TODO: Maybe throw error
    }

    LHS = ConvertToDouble(LHS);
    RHS = ConvertToDouble(RHS);

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

    LHS = ConvertToDouble(LHS);
    RHS = ConvertToDouble(RHS);

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

    LHS = ConvertToDouble(LHS);
    RHS = ConvertToDouble(RHS);

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

    LHS = ConvertToDouble(LHS);
    RHS = ConvertToDouble(RHS);

    switch (op)
    {
    case T_AND: // TODO: Case for strings
        return Builder->CreateAnd(CheckIfValueTrue(LHS, "andcomp"), CheckIfValueTrue(RHS, "andcomp"), "and");
    case T_OR:
        return Builder->CreateOr(CheckIfValueTrue(LHS, "orcomp"), CheckIfValueTrue(RHS, "orcomp"), "or");
    default:
        return nullptr; // TODO: Throw error
    }
}

Value *CodeGen::ProcedureCall(std::string name, std::vector<Value *> args)
{
    // Get the real full name (if multiple procedures with same name exist in different scopes
    std::string realName = SymbolTable::GetRealProcedureName(name);

    if (realName.compare("") != 0)
    {
        name = realName;
    }

    Function *F = TheModule->getFunction(name);
    if (!F)
    {
        // TODO: Throw error
        return nullptr;
    }

    int i = 0;
    for (auto &arg : F->args())
    {
        args[i] = ConvertType(arg.getType(), args[i]);
        ++i;
    }

    return Builder->CreateCall(F, args, "calltmp");
}

void CodeGen::VariableDeclaration(std::string name, ValueType type, bool isGlobal)
{
    Value *InitVal = nullptr;
    Type *T = nullptr;
    int AlignNum = 0;
    GetTypeAndInitVal(type, InitVal, T, AlignNum);

    if (!isGlobal && Blocks.size() == 1) // "Outer function" variables are always inserted globally
    {
        isGlobal = true;
    }

    if (isGlobal)
    {
        TheModule->getOrInsertGlobal(name, T);
        GlobalVariable *gVar = TheModule->getNamedGlobal(name);
        gVar->setLinkage(GlobalValue::CommonLinkage);

#ifdef _WIN64
        gVar->setAlignment(Align(AlignNum));
#else
        gVar->setAlignment(AlignNum);
#endif

        Constant *C = dyn_cast<Constant>(InitVal);
        gVar->setInitializer(C);
    }
    else
    {
        Function *TheFunction = Builder->GetInsertBlock()->getParent();
        AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, name, T);

        Builder->CreateStore(InitVal, Alloca);

        SymbolTable::SetIRAllocaInst(name, Alloca);
    }
}

Value *CodeGen::AssignmentStatement(std::string name, Value *RHS)
{
    Value *V = SymbolTable::GetIRAllocaInst(name);

    if (!V)
    {
        // Check if it's a global variable
        V = TheModule->getNamedGlobal(name);
        if (!V)
        {
            return nullptr; // TODO: Probably throw error
        }
    }
    
    Builder->CreateStore(ConvertType(V, RHS), V);
        
    return RHS;
}

Value *CodeGen::ReturnStatement(Value *RHS)
{
    Function *TheFunction = Builder->GetInsertBlock()->getParent();

    Builder->CreateRet(ConvertType(TheFunction->getReturnType(), RHS));
    return RHS;
}

Value *CodeGen::CheckIfValueTrue(Value *ValIn, std::string CondName)
{
    if (ValIn->getType()->isFloatingPointTy()) // If it's a float
    {
        return Builder->CreateFCmpONE(ValIn, FloatExpr(0.0), CondName);
    }
    else if (ValIn->getType()->isIntegerTy()) // Int or bool
    {
        return Builder->CreateICmpNE(ValIn, BoolExpr(0), CondName);
    }
}

void CodeGen::IfStatement(Value *Condition, BasicBlock *&ThenBBOut, BasicBlock *&ElseBBOut, BasicBlock *&MergeBBOut, Function *&TheFunctionOut)
{
    if (!Condition)
    {
        // TODO: Probably throw error
        return;
    }

    TheFunctionOut = Builder->GetInsertBlock()->getParent();

    ThenBBOut = BasicBlock::Create(*TheContext, "then", TheFunctionOut);
    ElseBBOut = BasicBlock::Create(*TheContext, "else");
    MergeBBOut = BasicBlock::Create(*TheContext, "ifcont");

    Builder->CreateCondBr(CheckIfValueTrue(Condition, "ifcond"), ThenBBOut, ElseBBOut);

    Builder->SetInsertPoint(ThenBBOut);

    // Codegen if then statements
}

void CodeGen::ElseStatement(BasicBlock *&ThenBBOut, BasicBlock *&ElseBBOut, BasicBlock *&MergeBBOut, Function *&TheFunctionOut)
{
    Builder->CreateBr(MergeBBOut);
    ThenBBOut = Builder->GetInsertBlock();

    TheFunctionOut->getBasicBlockList().push_back(ElseBBOut);
    Builder->SetInsertPoint(ElseBBOut);

    // Codegen else statements
}

void CodeGen::EndIfStatement(BasicBlock *ThenBB, BasicBlock *ElseBB, BasicBlock *MergeBB, Function *TheFunction)
{
    Builder->CreateBr(MergeBB);

    ElseBB = Builder->GetInsertBlock();

    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder->SetInsertPoint(MergeBB);
    
    // Done
}

void CodeGen::ForStatementHeader(BasicBlock *&ForCheckBBOut, BasicBlock *&LoopBBOut, BasicBlock *&AfterForBBOut, Function *&TheFunctionOut)
{
    TheFunctionOut = Builder->GetInsertBlock()->getParent();
    ForCheckBBOut = BasicBlock::Create(*TheContext, "forcheck", TheFunctionOut);
    LoopBBOut = BasicBlock::Create(*TheContext, "for");
    AfterForBBOut = BasicBlock::Create(*TheContext, "forcont");

    Builder->CreateBr(ForCheckBBOut);

    Builder->SetInsertPoint(ForCheckBBOut);

    // Codegen for loop check expression
}

void CodeGen::ForStatementCheck(Value *LoopCondition, BasicBlock *&ForCheckBBOut, BasicBlock *&LoopBBOut, BasicBlock *&AfterForBBOut, Function *&TheFunctionOut)
{
    Builder->CreateCondBr(CheckIfValueTrue(LoopCondition, "forcond"), LoopBBOut, AfterForBBOut);

    TheFunctionOut->getBasicBlockList().push_back(LoopBBOut);
    Builder->SetInsertPoint(LoopBBOut);
}

void CodeGen::EndForStatement(BasicBlock *ForCheckBB, BasicBlock *LoopBB, BasicBlock *AfterForBB, Function *TheFunction)
{
    Builder->CreateBr(ForCheckBB);

    TheFunction->getBasicBlockList().push_back(AfterForBB);
    Builder->SetInsertPoint(AfterForBB);

    // Done
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

// This function converts types for assignment statements so they match
Value *CodeGen::ConvertType(Value *Destination, Value *RHS)
{
    return ConvertType(Destination->getType(), RHS);
}

Value *CodeGen::ConvertType(Type *DestinationType, Value *RHS)
{
    Type *destType = DestinationType;
    Type *rhsType = RHS->getType();

    if (destType->isPointerTy())
    {
        destType = destType->getPointerElementType();
    }
    if (rhsType->isPointerTy())
    {
        rhsType = rhsType->getPointerElementType();
    }

    if (destType->isIntegerTy())
    {
        if (rhsType->isIntegerTy())
        {
            return RHS;
        }
        else if (rhsType->isFloatingPointTy())
        {
            return Builder->CreateFPToSI(RHS, IntType(), "ftoi");
        }
    }
    else if (destType->isFloatingPointTy())
    {
        if (rhsType->isIntegerTy())
        {
            return Builder->CreateSIToFP(RHS, DoubleType(), "itof");
        }
        else if (rhsType->isFloatingPointTy())
        {
            return RHS;
        }
    }
    else
    {
        return RHS;
    }
}

// This function converts NUMERICAL types to floats
Value *CodeGen::ConvertToDouble(Value *Val)
{
    Type *T = Val->getType();
    
    if (T->isPointerTy())
    {
        T = T->getPointerElementType();
    }

    if (T->isIntegerTy())
    {
        return Builder->CreateSIToFP(Val, DoubleType(), "itof");
    }
    else
    {
        return Val;
    }
}

Function *CodeGen::ProcedureHeader(std::string name, Type *retType, std::vector<std::string> argNames, std::vector<Type *> argTypes)
{
    FunctionType *FT = FunctionType::get(retType, argTypes, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, name, TheModule.get()); // TODO: Change ExternalLinkage for scoping or maybe not

    if (runtimeGenerated) // Don't do this with runtime functions
    {
        SymbolTable::SetRealProcedureName(name, F->getName().str());
    }

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
    SetBasicBlock(BB);

    // Insert function arguments into NamedValues map, might need to clear
    for (auto &Arg : F->args())
    {
        AllocaInst *Alloca = CreateEntryBlockAlloca(F, std::string(Arg.getName()), Arg.getType());
        Builder->CreateStore(&Arg, Alloca);

        SymbolTable::SetIRAllocaInst(std::string(Arg.getName()), Alloca);
    }

    return F;
}

Function *CodeGen::ProcedureEnd(Function *F)
{
    // TODO: Better condition here
    if (F) //if (AllocaInst *RetAllocaInst = SymbolTable::GetReturnAllocaInst())
    {
        //Builder->CreateRet(RetAllocaInst);

        verifyFunction(*F);

        EndBasicBlock();

        return F;
    }

    // Can't read body, remove the function
    F->eraseFromParent();
    EndBasicBlock();
    return nullptr;
}