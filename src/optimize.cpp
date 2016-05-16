#include "Rllvm.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>

#if 0
#include <llvm/Target/TargetData.h>
#endif
#include <llvm/LinkAllPasses.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#if 0
extern "C"
SEXP
R_optimizeModule(SEXP r_module, SEXP r_passMgr)
{
}
#endif

extern "C"
SEXP
R_getPassManager(SEXP r_module, SEXP r_ee, SEXP r_level)
{
  llvm::Module *TheModule = GET_REF(r_module, Module);
  llvm::ExecutionEngine *TheExecutionEngine = NULL;

  llvm::legacy::FunctionPassManager *mgr = new llvm::legacy::FunctionPassManager(TheModule);

  if(r_ee != R_NilValue) {
     TheExecutionEngine = GET_REF(r_ee, ExecutionEngine);
     // Set up the optimizer pipeline.  Start with registering info about how the
     // target lays out data structures.
#ifndef LLVM_VERSION_THREE_TWO
     mgr->add(new llvm::TargetData(*TheExecutionEngine->getTargetData()));
#endif
  }

#if 1
 llvm::PassManagerBuilder Builder;
 Builder.OptLevel = INTEGER(r_level)[0];
 Builder.populateFunctionPassManager(*mgr);
// Builder.populateModulePassManager(MPM);
#else
  // Promote allocas to registers.
  mgr->add(llvm::createPromoteMemoryToRegisterPass());
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  mgr->add(llvm::createInstructionCombiningPass());
  // Reassociate expressions.
  mgr->add(llvm::createReassociatePass());
  // Eliminate Common SubExpressions.
  mgr->add(llvm::createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  mgr->add(llvm::createCFGSimplificationPass());
#endif
  mgr->doInitialization();

  return(R_createRef(mgr, "FunctionPassManager"));
}


extern "C"
SEXP
R_optimizeFunction(SEXP r_func, SEXP r_passMgr)
{
  llvm::legacy::FunctionPassManager *mgr = GET_REF(r_passMgr, legacy::FunctionPassManager);
  llvm::Function *func = GET_REF(r_func, Function);
  return(ScalarLogical(mgr->run(*func)));
}


extern "C"
SEXP
R_PassManager_run(SEXP r_passMgr, SEXP r_module)
{
  llvm::legacy::PassManager *mgr = GET_REF(r_passMgr, legacy::PassManager);
  llvm::Module *module = GET_REF(r_module, Module);
  return(ScalarLogical(mgr->run(*module)));
}



extern "C"
SEXP
R_PassManagerBase_Add(SEXP r_mgr, SEXP r_pass)
{
    llvm::legacy::PassManagerBase *mgr = GET_REF(r_mgr, legacy::PassManagerBase);
    llvm::Pass *pass = GET_REF(r_pass, Pass);
    mgr->add(pass);
    return(ScalarLogical(TRUE));
}

/*
// Returns the TargetMachine instance or zero if no triple is provided.
static TargetMachine* GetTargetMachine(Triple TheTriple, StringRef CPUStr,
                                       StringRef FeaturesStr,
                                       const TargetOptions &Options) {
  std::string Error;
	
  const llvm::Target *TheTarget = llvm::TargetRegistry::lookupTarget(MArch, TheTriple,
                                                         Error);
  // Some modules don't specify a triple, and this is okay.
  if (!TheTarget) {
    return nullptr;
  }

  return TheTarget->createTargetMachine(TheTriple.getTriple(),
                                        CPUStr, FeaturesStr, Options,
                                        RelocModel, CMModel,
                                        llvm::CodeGenOpt::None);
}*/


extern "C"
SEXP
R_PassManager_new(SEXP r_mod, SEXP r_fnMgr)
{
	llvm::Module *mod = GET_REF(r_mod, Module);\

    if(LOGICAL(r_fnMgr)[0]) {
        llvm::legacy::FunctionPassManager *fm = new llvm::legacy::FunctionPassManager(mod);
        return(R_createRef(fm, "FunctionPassManager"));
    } else {
        llvm::legacy::PassManager *m = new llvm::legacy::PassManager();  

/*
		llvm::Triple ModuleTriple(mod->getTargetTriple());
		std::string CPUStr, FeaturesStr;
		llvm::TargetMachine *Machine = nullptr;
    	CPUStr = getCPUStr();
    	FeaturesStr = getFeaturesStr();
		const llvm::TargetOptions Options = InitTargetOptionsFromCodeGenFlags();


		Machine = GetTargetMachine(ModuleTriple, CPUStr, FeaturesStr, Options);


		std::unique_ptr<llvm::TargetMachine> TM(Machine);
 

	  	// Add an appropriate TargetLibraryInfo pass for the module's triple.
	  	llvm::TargetLibraryInfoImpl TLII(ModuleTriple);

	  	m->add(new llvm::TargetLibraryInfoWrapperPass(TLII));


	  	// Add internal analysis passes from the target machine.
	  	m->add(createTargetTransformInfoWrapperPass(TM ? TM->getTargetIRAnalysis()
		                                                 : TargetIRAnalysis()));

*/     
		SEXP res=R_createRef(m, "PassManager");
        return(res);
    }
}

extern "C"
SEXP
R_FunctionPassManager_DoInit(SEXP r_mgr, SEXP r_pass)
{
    llvm::legacy::FunctionPassManager *mgr = GET_REF(r_mgr, legacy::FunctionPassManager);
    mgr->doInitialization();
    return(ScalarLogical(TRUE));
}

extern "C"
SEXP
R_Pass_Initialize()
{
  llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
  llvm::initializeCore(Registry);
  llvm::initializeScalarOpts(Registry);
  llvm::initializeObjCARCOpts(Registry);
  llvm::initializeVectorization(Registry);
  llvm::initializeIPO(Registry);
  llvm::initializeAnalysis(Registry);
  llvm::initializeIPA(Registry);
  llvm::initializeTransformUtils(Registry);
  llvm::initializeInstCombine(Registry);
  llvm::initializeInstrumentation(Registry);
  llvm::initializeTarget(Registry);
  // For codegen passes, only passes that do IR to IR transformation are
  // supported.
  llvm::initializeCodeGenPreparePass(Registry);
  llvm::initializeAtomicExpandPass(Registry);
  llvm::initializeRewriteSymbolsPass(Registry);
  llvm::initializeWinEHPreparePass(Registry);
  llvm::initializeDwarfEHPreparePass(Registry);
  llvm::initializeSjLjEHPreparePass(Registry);

	return(R_NilValue);
}
