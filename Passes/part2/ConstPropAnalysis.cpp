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
    class ConstantInfo : public Info {
    // protected:
    public:
        map<unsigned, int> const_pairs;
        bool bottom = true;


        ConstantInfo(): Info() { }

        ~ConstantInfo() { }

       /*
        * Print out the information
        *
        * Direction:
        *   In your subclass you should implement this function according to the project specifications.
        */
        void print() {
            for (auto it : const_pairs){
                if (it.second != INT_MAX) {
                    errs()<<'('<<it.first<<','<<it.second<<')'<<'|';
                }
            }
            errs()<<"\n";
        }

       /*
        * Compare two pieces of information
        *
        * Direction:
        *   In your subclass you need to implement this function.
        */
        static bool equals(ConstantInfo * info1, ConstantInfo * info2) {
            return info1->const_pairs == info2->const_pairs;
        }


        /*
        * Join two pieces of information.
        * The third parameter points to the result.
        *
        * Direction:
        *   In your subclass you need to implement this function.
        */
        static Info* join(ConstantInfo * info1, ConstantInfo * info2, ConstantInfo * result){
            if (info1->bottom && !info2->bottom) {
                result->const_pairs = info2->const_pairs;
            } else if (!info1->bottom && info2->bottom) {
                result->const_pairs = info1->const_pairs;
            } else {
                // for (auto &it : info1->const_pairs) {
                //     // in both info1 and info2
                //     if (info2->const_pairs.find(it.first) != info2->const_pairs.end()) {
                //         if (it.second == info2->const_pairs.find(it.first)->second) {
                //             result->const_pairs[it.first] = it.second;
                //         }
                //         // in both info1 and info2 w/ different value
                //         else {
                //             result->const_pairs[it.first] = INT_MAX;
                //         }
                //     }
                // }
                for (auto &it : info1->const_pairs) {
                    // only in info1
                    if (info2->const_pairs.find(it.first) == info2->const_pairs.end()) {
                        result->const_pairs[it.first] = INT_MAX;
                    }
                    // in both info1 and info2
                    else if (it.second == info2->const_pairs.find(it.first)->second) {
                        result->const_pairs[it.first] = it.second;
                    }
                    // in both info1 and info2 w/ different value
                    else {
                        result->const_pairs[it.first] = INT_MAX;
                    }
                }
                
                for (auto &it : info2->const_pairs) {
                    // only in info2
                    if (info1->const_pairs.find(it.first) == info1->const_pairs.end()) {
                        result->const_pairs[it.first] = INT_MAX;
                    }
                }
            }
            

            // for (auto &it : info1->const_pairs) {
            //     // only in info1
            //     if (info2->const_pairs.find(it.first) == info2->const_pairs.end()) {
            //         result->const_pairs[it.first] = it.second;
            //     }
            //     // in both info1 and info2
            //     else if (it.second == info2->const_pairs.find(it.first)->second) {
            //         result->const_pairs[it.first] = it.second;
            //     }
            //     // in both info1 and info2 w/ different value
            //     else {
            //         result->const_pairs[it.first] = INT_MAX;
            //     }
            // }
            
            // for (auto &it : info2->const_pairs) {
            //     // only in info2
            //     if (info1->const_pairs.find(it.first) == info1->const_pairs.end()) {
            //         result->const_pairs[it.first] = it.second;
            //     }
            //     // in both info1 and info2
            //     else if (it.second == info1->const_pairs.find(it.first)->second) {
            //         result->const_pairs[it.first] = it.second;
            //     }
            // }

        result->bottom = false;
        return result;
        }

        
    };

    template <class Info, bool Direction>
    class ConstPropAnalysis : public DataFlowAnalysis<Info, Direction> {
    public:
        ConstPropAnalysis(Info &bottom, Info &initialState):   
            DataFlowAnalysis<Info, Direction>::DataFlowAnalysis(bottom, initialState) { }

        ~ConstPropAnalysis() {}

        void flowfunction(Instruction * I,
            std::vector<unsigned> & IncomingEdges,
            std::vector<unsigned> & OutgoingEdges,
            std::vector<ConstantInfo *> & Infos)
        {
            // get instruction info
            unsigned index = this->getInstrToIndex()[I];
            I->getOpcodeName();
            string opname(I->getOpcodeName());

            // errs()<<"processing instruction name: "<<opname<<". processing instruction index: "<<index<<"\n";

            // combining incoming infos
            ConstantInfo *info_in = new ConstantInfo();
            ConstantInfo *info_out = new ConstantInfo();
            unsigned end = index;
            for (auto start : IncomingEdges){
                auto edge = make_pair(start, end);
                
                // errs()<<"----------------begin join-----------------\n";
                // errs()<<"Edge "<<start<<"->""Edge "<<end<<"\n";
                // errs()<<"joining...\n";
                // info_in->print();
                // this->getEdgeToInfo()[edge]->print();

                ConstantInfo::join(info_in, this->getEdgeToInfo()[edge], info_in);

                // errs()<<"join result...\n";
                // info_in->print();
                // errs()<<"----------------end of join-----------------\n";
            }
            info_out->const_pairs = info_in->const_pairs;
            info_out->bottom = info_in->bottom;

            // computing outgoing info
            // errs()<<I->getOpcodeName()<<':'<<I->getOpcode()<<"\n";
            Value *v = I->getOperand(0);
            Value *v1 = I->getOperand(1);
            ConstantInt *CI = dyn_cast<ConstantInt>(v);
            // operand 1 is a constant
            if (CI) {
                ConstantInt *CI1 = dyn_cast<ConstantInt>(v1);
                // op1 const, op2 const
                if (CI1) {
                    switch (I->getOpcode()) {
                        case 12: // add
                            // arithmatic operations assigning to a new variable, no need to check INT_MAX
                            info_out->const_pairs[index] = CI->getZExtValue() + CI1->getZExtValue();
                            break;
                        case 14: // sub
                            info_out->const_pairs[index] = CI->getZExtValue() - CI1->getZExtValue();
                            break;
                        case 16: // mul
                            info_out->const_pairs[index] = CI->getZExtValue() * CI1->getZExtValue();
                            break;
                        case 18: // udiv
                            info_out->const_pairs[index] = CI->getZExtValue() / CI1->getZExtValue();
                            break;
                        case 19: // sdiv
                            info_out->const_pairs[index] = CI->getZExtValue() / CI1->getZExtValue();
                            break;
                        case 27: // and
                            info_out->const_pairs[index] = CI->getZExtValue() & CI1->getZExtValue();
                            break;
                        case 28: // or
                            info_out->const_pairs[index] = CI->getZExtValue() | CI1->getZExtValue();
                            break;
                        case 29: // xor
                            info_out->const_pairs[index] = CI->getZExtValue() ^ CI1->getZExtValue();
                            break;
                        case 21: // urem
                            info_out->const_pairs[index] = CI->getZExtValue() % CI1->getZExtValue();
                            break;
                        case 22: // srem
                            info_out->const_pairs[index] = CI->getZExtValue() % CI1->getZExtValue();
                            break;
                        case 24: // shl
                            info_out->const_pairs[index] = CI->getZExtValue() << CI1->getZExtValue();
                            break;
                        case 25: // lshr
                            info_out->const_pairs[index] = CI->getZExtValue() >> CI1->getZExtValue();
                            break;
                        case 26: // ashr
                            info_out->const_pairs[index] = CI->getZExtValue() >> CI1->getZExtValue();
                            break;
                        case 32: // store
                            errs()<<"Invalid operands for store (1,1)!\n";
                    }
                }
                // op1 const, op2 not const
                else {
                    Instruction *instr1 = dyn_cast<Instruction>(v1); 
                    switch (I->getOpcode()) {
                        case 12: // add
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() + info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 14: // sub
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() - info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 16: // mul
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() * info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 18: // udiv
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() / info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 19: // sdiv
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() / info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 27: // and
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() & info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 28: // or
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() | info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 29: // xor
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() ^ info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 21: // urem
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() % info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 22: // srem
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() % info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 24:
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() << info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 25:
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() >> info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 26:
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] = CI->getZExtValue() << info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                }
                            }
                            break;
                        case 32: // store
                            if (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX) {
                                info_out->const_pairs[this->getInstrToIndex()[instr1]] = CI->getZExtValue();
                            }
                    }
                }
            }
            // operand 1 is not a constant
            else {
                Instruction *instr = dyn_cast<Instruction>(v); 
                ConstantInt *CI1 = dyn_cast<ConstantInt>(v1);
                // op1 not const, op2 const
                if (CI1) {
                    switch (I->getOpcode()) {
                        case 12: // add
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] + CI1->getZExtValue();

                                }
                            }
                            break;
                        case 14: // sub
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] - CI1->getZExtValue();

                                }
                            }
                            break;
                        case 16: // mul
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] * CI1->getZExtValue();

                                }
                            }
                            break;
                        case 18: // udiv
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] / CI1->getZExtValue();

                                }
                            }
                            break;
                        case 19: // sdiv
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] / CI1->getZExtValue();

                                }
                            }
                            break;
                        case 27: // and
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] & CI1->getZExtValue();

                                }
                            }
                            break;
                        case 28: // or
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] | CI1->getZExtValue();

                                }
                            }
                            break;
                        case 29: // xor
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] ^ CI1->getZExtValue();

                                }
                            }
                            break;
                        case 21: // urem
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] % CI1->getZExtValue();

                                }
                            }
                            break;
                        case 22: // srem
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] % CI1->getZExtValue();

                                }
                            }
                            break;
                        case 24: 
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] << CI1->getZExtValue();

                                }
                            }
                            break;
                        case 25:
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] >> CI1->getZExtValue();

                                }
                            }
                            break;
                        case 26:
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) {
                                    info_out->const_pairs[index] = INT_MAX;
                                } else {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] >> CI1->getZExtValue();

                                }
                            }
                            break;
                        case 32: // store
                            errs()<<"Invalid operands for store (0,1)!\n";
                    }
                }
                // op1 not const, op2 not const
                else {
                    Instruction *instr1 = dyn_cast<Instruction>(v1); 
                    switch (I->getOpcode()) {
                        case 12: // add
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] + info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 14: // sub
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] - info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 16: // mul
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] * info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 18: // udiv
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] / info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 19: // sdiv
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] / info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 27: // and
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] & info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 28: // or
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] | info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 29: // xor
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] ^ info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 21: // urem
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] % info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 22: // srem
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] % info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 24:
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] << info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 25:
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] >> info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 26:
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                                    if ((info_out->const_pairs[this->getInstrToIndex()[instr]] == INT_MAX) || (info_out->const_pairs[this->getInstrToIndex()[instr1]] == INT_MAX)) {
                                        info_out->const_pairs[index] = INT_MAX;
                                    } else {
                                        info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] >> info_out->const_pairs[this->getInstrToIndex()[instr1]];
                                    }
                                }
                            }
                            break;
                        case 31: // load, note load has only one op
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                    info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]];
                            }
                            break;
                        case 32: // store
                            if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)*/) {
                                if (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX) {
                                    info_out->const_pairs[this->getInstrToIndex()[instr1]] = info_out->const_pairs[this->getInstrToIndex()[instr]];
                                }
                            }
                    }
                }

            }

            // case for icmp
            if (I->getOpcode() == 52) {
                Value *vm = I->getOperand(0);
                Instruction *instrm = dyn_cast<Instruction>(vm);
                errs()<<instrm->getOpcodeName()<<"\n";
                errs()<<I->getOpcodeName()<<"\n";
            }
                // Value *vm = I->getOperand(0)
                // Value *v = I->getOperand(1);
                // Value *v1 = I->getOperand(2);
                // ConstantInt *CI = dyn_cast<ConstantInt>(v);
                // // operand 1 is a constant
                // if (CI) {
                //     ConstantInt *CI1 = dyn_cast<ConstantInt>(v1);
                //     // op1 constant, op2 constant
                //     if (CI1) {
                //         switch (I->getOpcode()) {
                //             case 12:
                //                 info_out->const_pairs[index] = CI->getZExtValue() + CI1->getZExtValue();
                //             case 32:
                //                 errs()<<"Invalid operands for store (1,1)!\n";
                //         }
                //     }
                //     // op1 constant, op2 not constant
                //     else {
                //         Instruction *instr1 = dyn_cast<Instruction>(v1); 
                //         // errs()<<info_out->const_pairs[this->getInstrToIndex()[instr1]]<<"\n";
                //         if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) /*&& (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)*/) {
                        
                //             switch (I->getOpcode()) {
                //                 case 12: // add
                //                     info_out->const_pairs[index] = CI->getZExtValue() + info_out->const_pairs[this->getInstrToIndex()[instr1]];
                //                     break;
                //                 case 31:
                //                     info_out->const_pairs[index] = info_out->const_pairs[this->getInstrToIndex()[instr1]];
                //                     break;
                //                 case 32: // store
                //                     info_out->const_pairs[this->getInstrToIndex()[instr1]] = CI->getZExtValue();
                //             }

                //         }
                        
                //     }
                // }
                // // operand 1 is not a constant
                // else {
                //     Instruction *instr = dyn_cast<Instruction>(v); 
                //     ConstantInt *CI1 = dyn_cast<ConstantInt>(v1);
                //     // op1 not constant, op2 constant
                //     if (CI1) {
                //         if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) && (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)) {
                //             switch (I->getOpcode()) {
                //                 case 12: // add
                //                     info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] + CI1->getZExtValue();
                //                     break;
                //                 case 32: // store
                //                     errs()<<"Invalid operands for store (0,1)!\n";
                //             }
                //         }
                //     }
                //     // op1 not constant, op2 not constant
                //     else {
                //         Instruction *instr1 = dyn_cast<Instruction>(v1); 
                //         if ((info_out->const_pairs.find(this->getInstrToIndex()[instr]) != info_out->const_pairs.end()) && (info_out->const_pairs[this->getInstrToIndex()[instr]] != INT_MAX)) {
                //             if ((info_out->const_pairs.find(this->getInstrToIndex()[instr1]) != info_out->const_pairs.end()) && (info_out->const_pairs[this->getInstrToIndex()[instr1]] != INT_MAX)) {
                //                 switch (I->getOpcode()) {
                //                     case 12: // add
                //                         info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]] + info_out->const_pairs[this->getInstrToIndex()[instr1]];
                //                         break;
                //                     case 31: // load, note load has only one op
                //                         info_out->const_pairs[index] =  info_out->const_pairs[this->getInstrToIndex()[instr]];
                //                         break;
                //                     case 32: // store
                //                         info_out->const_pairs[this->getInstrToIndex()[instr1]] = info_out->const_pairs[this->getInstrToIndex()[instr]];
                //                 }
                //             }

                //         }
                //     }

                // }



            // case for select



            // write to output info
            for(auto &info : Infos){
                info->const_pairs = info_out->const_pairs;
                info->bottom = info_out->bottom;
            }
            
            // errs()<<"outgoing info...\n";
            // info_out->print();
            // errs()<<"\n";

            delete info_in;
            delete info_out;

        }
    };

    struct ConstPropAnalysisPass : public FunctionPass {
        static char ID;

        ConstPropAnalysisPass() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            
            ConstantInfo bot;
            ConstantInfo init;
            ConstPropAnalysis<ConstantInfo,true> cpa(bot, init);
            cpa.runWorklistAlgorithm(&F);
            cpa.print();
            return false;
        }
    };
}

char ConstPropAnalysisPass::ID = 0;
static RegisterPass<ConstPropAnalysisPass> X("cs515-constproping","Developed for part 2", false, false);
