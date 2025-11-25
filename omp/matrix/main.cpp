// main.cpp
#include "matrix.h"
#include <iostream>
#include <vector>
#include <functional>
#include <tuple>

class PerformanceTester {
public:
    template <MatrixSize M_SIZE>
    static std::tuple<Matrix<M_SIZE>, long long>
        testPerformance(const std::function<Matrix<M_SIZE>(
                                    const Matrix<M_SIZE>&,
                                    const Matrix<M_SIZE>&)> 
                                    &multiplyFunc) {
        Matrix<M_SIZE> A(2.0);
        Matrix<M_SIZE> B(3.0);
        
        auto start = std::chrono::high_resolution_clock::now();
        Matrix<M_SIZE> C = multiplyFunc(A, B);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return std::make_tuple(C, duration.count());
    }
};

template <MatrixSize M_SIZE>
void testForSize(const std::string& methodName, 
                const std::function<Matrix<M_SIZE>(const Matrix<M_SIZE>&, const Matrix<M_SIZE>&)>
                &multiplyFunc,
                std::ofstream& file_time,
                std::ofstream& file_res) {
    auto [result, time_us] = PerformanceTester::testPerformance<M_SIZE>(multiplyFunc);
    file_time << static_cast<size_t>(M_SIZE) << "," << time_us << "\n";
    std::cout << methodName << " - Size: " << static_cast<size_t>(M_SIZE) << ", Time: " << time_us << " microseconds" << std::endl;

    for (size_t i = 0; i < M_SIZE; ++i)
    {
        for (size_t j = 0; j < M_SIZE; ++j)
        {
            file_res << result(i, j) << " ";
        }
        file_res << std::endl;
    }
}

void testMethod(const std::string& methodName, 
                const std::function<Matrix<M_64>(const Matrix<M_64>&, const Matrix<M_64>&)>& func64,
                const std::function<Matrix<M_128>(const Matrix<M_128>&, const Matrix<M_128>&)>& func128,
                const std::function<Matrix<M_256>(const Matrix<M_256>&, const Matrix<M_256>&)>& func256,
                const std::function<Matrix<M_512>(const Matrix<M_512>&, const Matrix<M_512>&)>& func512,
                const std::function<Matrix<M_1024>(const Matrix<M_1024>&, const Matrix<M_1024>&)>& func1024,
                const std::function<Matrix<M_2048>(const Matrix<M_2048>&, const Matrix<M_2048>&)>& func2048) {
    
    std::ofstream file_time(methodName + "_time.csv");
    std::ofstream file_res(methodName + "_res.csv");
    file_time << "Size,Time(microseconds)\n";
    
    std::cout << "Testing " << methodName << " method:" << std::endl;
    
    testForSize<M_64>(methodName, func64, file_time, file_res);
    testForSize<M_128>(methodName, func128, file_time, file_res);
    testForSize<M_256>(methodName, func256, file_time, file_res);
    testForSize<M_512>(methodName, func512, file_time, file_res);
    testForSize<M_1024>(methodName, func1024, file_time, file_res);
    testForSize<M_2048>(methodName, func2048, file_time, file_res);
    
    file_time.close();
    file_res.close();
    std::cout << methodName << " testing completed." << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <method>" << std::endl;
        std::cout << "Methods: naive, transpose, block, strassen, parallel, simd, all" << std::endl;
        return 1;
    }
    
    std::string method = argv[1];
    
    if (method == "naive") {
        testMethod("naive", 
            Matrix<M_64>::naiveMultiply,
            Matrix<M_128>::naiveMultiply,
            Matrix<M_256>::naiveMultiply,
            Matrix<M_512>::naiveMultiply,
            Matrix<M_1024>::naiveMultiply,
            Matrix<M_2048>::naiveMultiply
        );
    } else if (method == "transpose") {
        testMethod("transpose",
            Matrix<M_64>::transposeMultiply,
            Matrix<M_128>::transposeMultiply,
            Matrix<M_256>::transposeMultiply,
            Matrix<M_512>::transposeMultiply,
            Matrix<M_1024>::transposeMultiply,
            Matrix<M_2048>::transposeMultiply
        );
    } else if (method == "block") {
        testMethod("block",
            Matrix<M_64>::blockMultiply,
            Matrix<M_128>::blockMultiply,
            Matrix<M_256>::blockMultiply,
            Matrix<M_512>::blockMultiply,
            Matrix<M_1024>::blockMultiply,
            Matrix<M_2048>::blockMultiply
        );
    } else if (method == "strassen") {
        testMethod("strassen",
            Matrix<M_64>::strassenMultiply,
            Matrix<M_128>::strassenMultiply,
            Matrix<M_256>::strassenMultiply,
            Matrix<M_512>::strassenMultiply,
            Matrix<M_1024>::strassenMultiply,
            Matrix<M_2048>::strassenMultiply
        );
    } else if (method == "parallel") {
        testMethod("parallel",
            Matrix<M_64>::parallelMultiply,
            Matrix<M_128>::parallelMultiply,
            Matrix<M_256>::parallelMultiply,
            Matrix<M_512>::parallelMultiply,
            Matrix<M_1024>::parallelMultiply,
            Matrix<M_2048>::parallelMultiply
        );
    } else if (method == "simd") {
        testMethod("simd",
            Matrix<M_64>::simdMultiply,
            Matrix<M_128>::simdMultiply,
            Matrix<M_256>::simdMultiply,
            Matrix<M_512>::simdMultiply,
            Matrix<M_1024>::simdMultiply,
            Matrix<M_2048>::simdMultiply
        );
    } else if (method == "all") {
        testMethod("naive", Matrix<M_64>::naiveMultiply, Matrix<M_128>::naiveMultiply, Matrix<M_256>::naiveMultiply, Matrix<M_512>::naiveMultiply, Matrix<M_1024>::naiveMultiply, Matrix<M_2048>::naiveMultiply);
        testMethod("transpose", Matrix<M_64>::transposeMultiply, Matrix<M_128>::transposeMultiply, Matrix<M_256>::transposeMultiply, Matrix<M_512>::transposeMultiply, Matrix<M_1024>::transposeMultiply, Matrix<M_2048>::transposeMultiply);
        testMethod("block", Matrix<M_64>::blockMultiply, Matrix<M_128>::blockMultiply, Matrix<M_256>::blockMultiply, Matrix<M_512>::blockMultiply, Matrix<M_1024>::blockMultiply, Matrix<M_2048>::blockMultiply);
        testMethod("strassen", Matrix<M_64>::strassenMultiply, Matrix<M_128>::strassenMultiply, Matrix<M_256>::strassenMultiply, Matrix<M_512>::strassenMultiply, Matrix<M_1024>::strassenMultiply, Matrix<M_2048>::strassenMultiply);
        testMethod("parallel", Matrix<M_64>::parallelMultiply, Matrix<M_128>::parallelMultiply, Matrix<M_256>::parallelMultiply, Matrix<M_512>::parallelMultiply, Matrix<M_1024>::parallelMultiply, Matrix<M_2048>::parallelMultiply);
        testMethod("simd", Matrix<M_64>::simdMultiply, Matrix<M_128>::simdMultiply, Matrix<M_256>::simdMultiply, Matrix<M_512>::simdMultiply, Matrix<M_1024>::simdMultiply, Matrix<M_2048>::simdMultiply);
    } else {
        std::cout << "Unknown method: " << method << std::endl;
        std::cout << "Available methods: naive, transpose, block, strassen, parallel, simd, all" << std::endl;
        return 1;
    }
    
    return 0;
}