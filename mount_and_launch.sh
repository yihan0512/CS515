#!/bin/bash

SOURCE_PATH=`pwd`/Passes
OUTPUT_PATH=`pwd`/Output
SCRIPT_PATH=`pwd`/Scripts

# MOUNT POINTS IN CONTAINER:
# --------------------------
#
# SOURCE CODE: /LLVM_ROOT/llvm/lib/Transforms/CS515_Project
# OUTPUT: /output

docker run --rm -it -v ${SOURCE_PATH}:/LLVM_ROOT/llvm/lib/Transforms/CS515_Project -v ${OUTPUT_PATH}:/output -v ${SCRIPT_PATH}:/scripts rutgers/llvm:8 /bin/bash
