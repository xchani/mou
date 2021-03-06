# set LD_LIBRARY_PATH
export CC  = clang
export CXX = clang++
export CXXFLAGS = -std=c++17 -Wall -O3 -I/usr/local/opt/llvm/include -I/usr/local/include
export LDFLAGS = -L/usr/local/lib -L/usr/local/cuda/lib64 \
	             $(brew info llvm | grep LDFLAGS= | cut -d = -f 2 | tr '"' ' ')
# specify tensor path
BIN = test_expression test_logger test_reflection test_pattern_matching test_tensor
CU_BIN = test_tensor_gpu

.PHONY: clean all

all: $(BIN) $(CU_BIN)

test_expression: test_expression.cc

test_logger: test_logger.cc

test_reflection: test_reflection.cc

test_pattern_matching: test_pattern_matching.cc

test_tensor: test_tensor.cc

test_tensor_gpu: test_tensor_gpu.cu

$(BIN) :
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc, $^)

$(CU_BIN) :
	$(CXX) -x cuda $(CXXFLAGS) $(LDFLAGS) -o $@ $(filter %.cu, $^) \
		--cuda-gpu-arch=sm_61 -lcudart_static -ldl -lrt -pthread

clean:
	rm -rf $(BIN) $(CU_BIN) *~
