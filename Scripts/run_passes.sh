#echo "TestPass:" > /output/test_pass_output.txt
#opt -load LLVMTestPass.so -TestPass < /tests/HelloWorld/HelloWorld.ll > /dev/null 2>> /output/test_pass_output.txt

#echo "cs515-csi:" > /output/test_pass_output.txt
#opt -load submission_pt1.so -cs515-csi < /tests/HelloWorld/HelloWorld.ll > /dev/null 2>> /output/test_pass_output.txt

# echo "cs515-csi(HelloWorld):" > /output/test_pass_output.txt
# opt -load submission_pt2.so -cs515-reaching < /tests/HelloWorld/HelloWorld.ll > /dev/null 2>> /output/test_pass_output.txt

# echo "cs515-csi(conditionalSum):" > /output/test_pass_output.txt
# opt -load submission_pt2.so -cs515-reaching < /tests/conditionalSum/main.ll > /dev/null 2>> /output/test_pass_output.txt

echo "cs515-csi(conditionalSum):" > /output/test_pass_output.txt
opt -load submission_pt2.so -cs515-constproping < /tests/conditionalSum/main.ll > /dev/null 2>> /output/test_pass_output.txt

# echo "cs515-csi(conditionalSum):" > /output/test_pass_output.txt
# opt -load submission_pt2.so -cs515-reason < /tests/conditionalSum/main.ll > /dev/null 2>> /output/test_pass_output.txt

# echo "cs515-csi(conditionalSum):" > /output/test_pass_output.txt
# opt -load submission_pt2.so -cs515-constproping < /tests/example/main.ll > /dev/null 2>> /output/test_pass_output.txt