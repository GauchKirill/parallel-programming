# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native
OMP_FLAGS = -fopenmp
SIMD_FLAGS = -mavx
TARGET = matrix_multiply.out

# Single executable
build: ${TARGET}

${TARGET}: main.cpp matrix.h
	$(CXX) $(CXXFLAGS) $(OMP_FLAGS) $(SIMD_FLAGS) -o $@ main.cpp

# Run targets
run_naive: ${TARGET}
	./${TARGET} naive

run_transpose: ${TARGET}
	./${TARGET} transpose

run_block: ${TARGET}
	./${TARGET} block

run_strassen: ${TARGET}
	./${TARGET} strassen

run_parallel: ${TARGET}
	./${TARGET} parallel

run_simd: ${TARGET}
	./${TARGET} simd

run_all: ${TARGET}
	./${TARGET} all

all: ${TARGET} run_all

clean:
	rm -f ${TARGET} *.csv

.PHONY: all clean run_naive run_transpose run_block run_strassen run_parallel run_simd run_all build