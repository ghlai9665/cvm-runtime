ROOTDIR = $(CURDIR)

.PHONY: clean all test

INCLUDE_FLAGS = -Iinclude
PKG_CFLAGS = -std=c++11 -Wall -O2 $(INCLUDE_FLAGS) -fPIC
PKG_LDFLAGS =


all: dep
	@mkdir -p build/lib && cd build/lib && cmake ../../ && $(MAKE)

dep:
	@cp cmake/config.cmake . --update

cpu: dep
	@mkdir -p build/cpu && cd build/cpu && cmake ../.. -DUSE_CUDA=OFF -DUSE_FORMAL=OFF && $(MAKE)
	@mkdir -p build/cpu && cd build/cpu && cmake ../.. -DUSE_CUDA=OFF -DUSE_FORMAL=OFF -DCMAKE_BUILD_TYPE=Debug && $(MAKE)
	ln -sf build/cpu/libcvm_runtime_cpu.so .

gpu: dep
	@mkdir -p build/gpu && cd build/gpu && cmake ../.. -DUSE_CUDA=ON -DUSE_FORMAL=OFF && $(MAKE)
	@mkdir -p build/gpu && cd build/gpu && cmake ../.. -DUSE_CUDA=ON -DCMAKE_BUILD_TYPE=Debug && $(MAKE)
	ln -sf build/gpu/libcvm_runtime_cuda.so .
formal: dep
	@mkdir -p build/formal && cd build/formal && cmake ../.. -DUSE_CUDA=OFF -DUSE_FORMAL=ON && $(MAKE)
	ln -sf build/formal/libcvm_runtime_formal.so .


test_model_cpu: cpu
	g++ ./tests/test_model.cc -Iinclude -fopenmp -std=c++11 -o tests/$@ -lcvm_runtime_cpu && ./tests/$@
test_model_gpu: gpu
	g++ ./tests/test_model.cc -Iinclude -fopenmp -std=c++11 -o tests/$@ -lcvm_runtime_cuda -DUSE_GPU && ./tests/$@
test_model_formal: formal
	g++ ./tests/test_model.cc -Iinclude -fopenmp -std=c++11 -o tests/$@ -lcvm_runtime_formal && ./tests/$@

test_op_cpu: cpu 
	g++ ./tests/test_op.cc -Iinclude -fopenmp -std=c++11 -o tests/$@ -lcvm_runtime_cpu && ./tests/$@
test_op_gpu: gpu
	g++ ./tests/test_op.cc -Iinclude -fopenmp -std=c++11 -o tests/$@ -lcvm_runtime_cuda -DUSE_GPU && ./tests/$@
test_op_formal: formal
	g++ ./tests/test_op.cc -Iinclude -fopenmp -std=c++11 -o tests/$@ -lcvm_runtime_formal && ./tests/$@

clean:
	  rm -rf ./build/*
