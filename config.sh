#change LLVM_EXTERNAL_LIT and MLIR_DIR to ones local directory
cmake -G Ninja .. -Wno-dev \
 		-DLLVM_EXTERNAL_LIT=/home/ubuntu/Github/llvm-project/build/bin/llvm-lit \
		-DMLIR_DIR=/home/ubuntu/Github/llvm-project/build/lib/cmake/mlir \
		-DCMAKE_C_COMPILER=/home/ubuntu/Github/llvm-project/build/bin/clang \
		-DCMAKE_CXX_COMPILER=/home/ubuntu/Github/llvm-project/build/bin/clang++ \
 		-DCMAKE_BUILD_TYPE=Debug
