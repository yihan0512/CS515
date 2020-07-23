#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include <map>

using namespace llvm;
using namespace std;

namespace {
	struct CountStaticInstructions : public FunctionPass {
		static char ID;

		CountStaticInstructions() : FunctionPass(ID) {}

		bool runOnFunction(Function &F) override {
			errs() << "Collecting Static Instruction Counts: ";
			errs().write_escaped(F.getName()) << '\n';

			map<string, int> count;
			for (Function::iterator B = F.begin(); B != F.end(); ++B)
				for (BasicBlock::iterator I = B->begin(); I != B->end(); ++I)
					++count[string(I->getOpcodeName())];

			for (map<string, int>::iterator iter = count.begin(); iter != count.end(); ++iter)
				errs() << iter->first << "\t" << iter->second << "\n";

			return false;
		}
	};
}

char CountStaticInstructions::ID = 0;
static RegisterPass<CountStaticInstructions> X("cs515-csi", "Collecting Static Instruction Counts",
                                false /* Only looks at CFG */,
                                false /* Analysis Pass */);
