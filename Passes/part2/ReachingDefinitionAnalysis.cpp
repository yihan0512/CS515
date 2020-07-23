#include "515DFA.h"

// #include "llvm/Pass.h"
// #include "llvm/IR/Type.h"
// #include "llvm/IR/Value.h"
// #include "llvm/IR/Module.h"
// #include "llvm/IR/Function.h"
// #include "llvm/IR/Constants.h"
// #include "llvm/IR/IRBuilder.h"
// #include "llvm/IR/BasicBlock.h"
// #include "llvm/IR/Instruction.h"
// #include "llvm/IR/LLVMContext.h"
// #include "llvm/Support/raw_ostream.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <vector>
#include <set>


using namespace llvm;
using namespace std;

namespace {
    class ReachingInfo : public Info {
    // protected:
    public:
        set<unsigned> defs;
        ReachingInfo(): Info() { }

        ReachingInfo(const ReachingInfo& other): Info(other) {
            defs = other.defs;
        }

        ~ReachingInfo() { }

        

       /*
        * Print out the information
        *
        * Direction:
        *   In your subclass you should implement this function according to the project specifications.
        */
        void print() {
            for (auto def : defs){
                errs()<<def<<'|';
            }
            errs()<<"\n";
        }

       /*
        * Compare two pieces of information
        *
        * Direction:
        *   In your subclass you need to implement this function.
        */
        static bool equals(ReachingInfo * info1, ReachingInfo * info2) {
            return info1->defs == info2->defs;
        }


        /*
        * Join two pieces of information.
        * The third parameter points to the result.
        *
        * Direction:
        *   In your subclass you need to implement this function.
        */
        static Info* join(ReachingInfo * info1, ReachingInfo * info2, ReachingInfo * result){
            result->defs = info1->defs;
            result->defs.insert(info2->defs.begin(), info2->defs.end());
            return result;
        }

        
    };

    template <class Info, bool Direction>
    class ReachingDefinitionAnalysis : public DataFlowAnalysis<Info, Direction> {
    public:
        ReachingDefinitionAnalysis(Info &bottom, Info &initialState):   
            DataFlowAnalysis<Info, Direction>::DataFlowAnalysis(bottom, initialState) { }

        ~ReachingDefinitionAnalysis() {}

        void flowfunction(Instruction * I,
            std::vector<unsigned> & IncomingEdges,
            std::vector<unsigned> & OutgoingEdges,
            std::vector<ReachingInfo *> & Infos)
        {
            // unsigned index = this->InstrToIndex[I];
            unsigned index = this->getInstrToIndex()[I];
            
            I->getOpcodeName();
            string opname(I->getOpcodeName());

            // combining incoming infos
            ReachingInfo *info_in = new ReachingInfo();
            ReachingInfo *info_out = new ReachingInfo();
            unsigned end = index;
            for (auto start : IncomingEdges){
                auto edge = make_pair(start, end);
                ReachingInfo::join(info_in, this->getEdgeToInfo()[edge], info_in);
            }
            info_out->defs = info_in->defs;
            set<string> cat1 = {"alloca", "load", "select", "getelementptr","icmp","fcmp"};
            set<string> cat2 = {"br","switch", "store"};
            set<string> cat3 = {"phi"};
            
            // outgoing info
            // category 1
            if (I->isBinaryOp() || cat1.find(opname)!=cat1.end()){
                info_out->defs.insert(index);
            // category 2 do nothing
            } 
            else if(cat2.find(opname)!=cat2.end()){
               // 
            } 
            // category 3
            else if(cat3.find(opname)!=cat3.end()){
                unsigned i=index;
                Instruction * firstNonPhi = I->getParent()->getFirstNonPHI();
                unsigned indexFirstNonPhi = this->getInstrToIndex()[firstNonPhi];

                while(i<indexFirstNonPhi) {
                    info_out->defs.insert(i);
                    i++;
                }
            } else {
                // 
            }

            for(auto &info : Infos){
                info->defs = info_out->defs;
            }

            delete info_in;
            delete info_out;
        }
    };

    struct ReachingDefinitionAnalysisPass : public FunctionPass {
        static char ID;

        ReachingDefinitionAnalysisPass() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            
            ReachingInfo bot;
            ReachingInfo init;
            ReachingDefinitionAnalysis<ReachingInfo, true> rda(bot, init);
            rda.runWorklistAlgorithm(&F);
            rda.print();
            return false;
        }
    };
}

char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass<ReachingDefinitionAnalysisPass> X("cs515-reaching","Developed for part 2", false, false);