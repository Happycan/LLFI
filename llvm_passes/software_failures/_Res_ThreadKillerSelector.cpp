#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CFG.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"

#include "FIInstSelector.h"
#include "_SoftwareFaultRegSelectors.h"
#include "FICustomSelectorManager.h"

#include <fstream>
#include <iostream>
#include <set>
#include <string>
/**
 * This instruction selector only selects the API call functions as target
 */

using namespace llvm;
namespace llfi{
    class _Res_ThreadKillerInstSelector: public SoftwareFIInstSelector{
        public:
            _Res_ThreadKillerInstSelector(){
                funcNames.insert(std::string("pthread_create"));
                funcNames.insert(std::string("pthread_join"));
            }
            virtual void getCompileTimeInfo(std::map<std::string, std::string>& info){
                info["failure_class"] = "Resource";
                info["failure_mode"] = "ThreadKiller";
                for(std::set<std::string>::iterator SI = funcNames.begin();
                  SI != funcNames.end(); SI++){
                  info["targets"] += *SI + "()/";
                }
                //remove the '/' at the end
                info["targets"] = info["targets"].substr(0, info["targets"].length()-1);
                info["injector"] = "PthreadThreadKillerInjector";
            }

        private:
            std::set<std::string> funcNames;

        virtual bool isInstFITarget(Instruction* inst){
            if(isa<CallInst>(inst) == false)    return false;
            else{
                CallInst* CI = dyn_cast<CallInst>(inst);
                Function* called_func = CI->getCalledFunction();
                if(called_func == NULL) return false;
                std::string func_name = std::string(called_func->getName());
                if(funcNames.find(func_name) != funcNames.end())    return true;
                else return false;
            }
        }
    };
    static RegisterFIInstSelector A("ThreadKiller(Res)", new _Res_ThreadKillerInstSelector());
    static RegisterFIRegSelector B("ThreadKiller(Res)", new FuncArgRegSelector(0));
}