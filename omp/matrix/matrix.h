// matrix.h
#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <functional>
#include <algorithm>
#include <immintrin.h>
#include <omp.h>
#include <tuple>

enum MatrixSize : size_t {
    M_32 = 32U,
    M_64 = 64U,
    M_128 = 128U,
    M_256 = 256U,
    M_512 = 512U,
    M_1024 = 1024U,
    M_2048 = 2048U,
    M_NONE = 0U
};

// Вспомогательная структура "MatrixSize"
template<MatrixSize SIZE>
struct HalfSize;

// Специализации для каждого размера
template<> struct HalfSize<M_2048> { static constexpr MatrixSize value = M_1024; };
template<> struct HalfSize<M_1024> { static constexpr MatrixSize value = M_512; };
template<> struct HalfSize<M_512> { static constexpr MatrixSize value = M_256; };
template<> struct HalfSize<M_256> { static constexpr MatrixSize value = M_128; };
template<> struct HalfSize<M_128> { static constexpr MatrixSize value = M_64; };
template<> struct HalfSize<M_64> { static constexpr MatrixSize value = M_32; };
template<> struct HalfSize<M_32> { static constexpr MatrixSize value = M_NONE; };

// Вспомогательная структура "DoubleSize"
template<MatrixSize SIZE>
struct DoubleSize;

// Специализации для каждого размера
template<> struct DoubleSize<M_2048> { static constexpr MatrixSize value = M_NONE; };
template<> struct DoubleSize<M_1024> { static constexpr MatrixSize value = M_2048; };
template<> struct DoubleSize<M_512> { static constexpr MatrixSize value = M_1024; };
template<> struct DoubleSize<M_256> { static constexpr MatrixSize value = M_512; };
template<> struct DoubleSize<M_128> { static constexpr MatrixSize value = M_256; };
template<> struct DoubleSize<M_64> { static constexpr MatrixSize value = M_128; };
template<> struct DoubleSize<M_32> { static constexpr MatrixSize value = M_64; };

template<MatrixSize M_SIZE>
class Matrix {
private:
    std::vector<std::vector<double>> _data;

public:
    auto extractSubmatrix() const {
        constexpr MatrixSize HALF = HalfSize<M_SIZE>::value;
        Matrix<HALF> M11;
        Matrix<HALF> M12;
        Matrix<HALF> M21;
        Matrix<HALF> M22;
        for (size_t i = 0; i < HALF; ++i) {
            for (size_t j = 0; j < HALF; ++j) {
                M11(i, j) = _data[i][j];
                M12(i, j) = _data[i][j + HALF];
                M21(i, j) = _data[i + HALF][j];
                M22(i, j) = _data[i + HALF][j + HALF];
            }
        }
        return std::make_tuple(M11, M12, M21, M22);
    }

    static Matrix<M_SIZE> insertSubmatrix(const Matrix<HalfSize<M_SIZE>::value>& A11,
                                         const Matrix<HalfSize<M_SIZE>::value>& A12,
                                         const Matrix<HalfSize<M_SIZE>::value>& A21,
                                         const Matrix<HalfSize<M_SIZE>::value>& A22) {
        constexpr MatrixSize HALF = HalfSize<M_SIZE>::value;
        Matrix<M_SIZE> result;
        for (size_t i = 0; i < HALF; ++i) {
            for (size_t j = 0; j < HALF; ++j) {
                result(i, j) = A11(i, j);
                result(i, j + HALF) = A12(i, j);
                result(i + HALF, j) = A21(i, j);
                result(i + HALF, j + HALF) = A22(i, j);
            }
        }
        return result;
    }

    // Рекурсивная функция для алгоритма Штрассена
    template<MatrixSize SIZE>
    static Matrix<SIZE> strassenRecursive(const Matrix<SIZE>& A, const Matrix<SIZE>& B) {
        constexpr MatrixSize HALF = HalfSize<SIZE>::value;
        
        if constexpr (HALF == M_NONE) {
            // Базовый случай - используем прямое умножение для самых маленьких матриц
            return Matrix<SIZE>::transposeMultiply(A, B);
        } else {
            auto [A11, A12, A21, A22] = A.extractSubmatrix();
            auto [B11, B12, B21, B22] = B.extractSubmatrix();

            // Вычисляем промежуточные матрицы по алгоритму Штрассена
            auto M1 = strassenRecursive(A11 + A22, B11 + B22);
            auto M2 = strassenRecursive(A21 + A22, B11);
            auto M3 = strassenRecursive(A11, B12 - B22);
            auto M4 = strassenRecursive(A22, B21 - B11);
            auto M5 = strassenRecursive(A11 + A12, B22);
            auto M6 = strassenRecursive(A21 - A11, B11 + B12);
            auto M7 = strassenRecursive(A12 - A22, B21 + B22);
            
            // Вычисляем результирующие блоки
            auto C11 = M1 + M4 - M5 + M7;
            auto C12 = M3 + M5;
            auto C21 = M2 + M4;
            auto C22 = M1 - M2 + M3 + M6;
            
            // Собираем результат
            return Matrix<SIZE>::insertSubmatrix(C11, C12, C21, C22);
        }
    }

    Matrix() : _data(M_SIZE, std::vector<double>(M_SIZE, 0.0)) {
        omp_set_num_threads(8);
    }
    
    Matrix(double value) : _data(M_SIZE, std::vector<double>(M_SIZE, value)) {
        omp_set_num_threads(8);
    }
    
    size_t getSize() const { return M_SIZE; }
    double& operator()(size_t i, size_t j) { return _data[i][j]; }
    const double& operator()(size_t i, size_t j) const { return _data[i][j]; }
    
    void fill(double value) {
        for (size_t i = 0; i < M_SIZE; ++i) {
            for (size_t j = 0; j < M_SIZE; ++j) {
                _data[i][j] = value;
            }
        }
    }
    
    bool operator==(const Matrix& other) const {
        for (size_t i = 0; i < M_SIZE; ++i) {
            for (size_t j = 0; j < M_SIZE; ++j) {
                if (std::abs(_data[i][j] - other._data[i][j]) > 1e-6) {
                    return false;
                }
            }
        }
        return true;
    }

    Matrix operator+(const Matrix& other) const {
        Matrix result;
        for (size_t i = 0; i < M_SIZE; ++i) {
            for (size_t j = 0; j < M_SIZE; ++j) {
                result(i, j) = _data[i][j] + other(i, j);
            }
        }
        return result;
    }

    Matrix operator-(const Matrix& other) const {
        Matrix result;
        for (size_t i = 0; i < M_SIZE; ++i) {
            for (size_t j = 0; j < M_SIZE; ++j) {
                result(i, j) = _data[i][j] - other(i, j);
            }
        }
        return result;
    }

    static Matrix naiveMultiply(const Matrix& a, const Matrix& b);

    static Matrix transposeMultiply(const Matrix& a, const Matrix& b);

    static Matrix blockMultiply(const Matrix& a, const Matrix& b);

    static Matrix parallelMultiply(const Matrix& a, const Matrix& b);

    static Matrix simdMultiply(const Matrix& a, const Matrix& b);

    static Matrix strassenMultiply(const Matrix& a, const Matrix& b);
};

template <MatrixSize M_SIZE>
Matrix<M_SIZE> Matrix<M_SIZE>::naiveMultiply(const Matrix<M_SIZE>& a,
                                             const Matrix<M_SIZE>& b) {
    Matrix<M_SIZE> result;
    for (size_t i = 0; i < M_SIZE; ++i) {
        for (size_t j = 0; j < M_SIZE; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < M_SIZE; ++k) {
                sum += a._data[i][k] * b._data[k][j];
            }
            result._data[i][j] = sum;
        }
    }
    return result;
}

template <MatrixSize M_SIZE>
Matrix<M_SIZE> Matrix<M_SIZE>::transposeMultiply(const Matrix<M_SIZE>& a,
                                                 const Matrix<M_SIZE>& b) {
    Matrix<M_SIZE> result;
    Matrix<M_SIZE> bTransposed;
    
    // Transpose matrix b
    for (size_t i = 0; i < M_SIZE; ++i) {
        for (size_t j = 0; j < M_SIZE; ++j) {
            bTransposed._data[j][i] = b._data[i][j];
        }
    }
    
    // Multiply with transposed matrix
    for (size_t i = 0; i < M_SIZE; ++i) {
        for (size_t j = 0; j < M_SIZE; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < M_SIZE; ++k) {
                sum += a._data[i][k] * bTransposed._data[j][k];  // Исправлено: bTransposed._data[j][k]
            }
            result._data[i][j] = sum;
        }
    }
    return result;
}

template <MatrixSize M_SIZE>
Matrix<M_SIZE> Matrix<M_SIZE>::blockMultiply(const Matrix<M_SIZE>& a,
                                             const Matrix<M_SIZE>& b) {
    Matrix result;
    result.fill(0.0);
    constexpr size_t blockSize = 64U;
    
    for (size_t i = 0; i < M_SIZE; i += blockSize) {
        for (size_t j = 0; j < M_SIZE; j += blockSize) {
            for (size_t k = 0; k < M_SIZE; k += blockSize) {
                
                size_t i_end = std::min(i + blockSize, static_cast<size_t>(M_SIZE));
                size_t j_end = std::min(j + blockSize, static_cast<size_t>(M_SIZE));
                size_t k_end = std::min(k + blockSize, static_cast<size_t>(M_SIZE));
                
                for (size_t i1 = i; i1 < i_end; ++i1) {
                    for (size_t j1 = j; j1 < j_end; ++j1) {
                        double sum = result._data[i1][j1];
                        for (size_t k1 = k; k1 < k_end; ++k1) {
                            sum += a._data[i1][k1] * b._data[k1][j1];
                        }
                        result._data[i1][j1] = sum;
                    }
                }
            }
        }
    }
    return result;
}

template <MatrixSize M_SIZE>
Matrix<M_SIZE> Matrix<M_SIZE>::parallelMultiply(const Matrix<M_SIZE>& a,
                                                const Matrix<M_SIZE>& b) {
    Matrix<M_SIZE> result;
    Matrix<M_SIZE> bTransposed;
    
    // Transpose matrix b
    #pragma omp parallel for schedule(dynamic, 2)
    for (size_t i = 0; i < M_SIZE; ++i) {
        for (size_t j = 0; j < M_SIZE; ++j) {
            bTransposed._data[j][i] = b._data[i][j];
        }
    }
    
    // Multiply with transposed matrix
    #pragma omp parallel for schedule(dynamic, 2)
    for (size_t i = 0; i < M_SIZE; ++i) {
        for (size_t j = 0; j < M_SIZE; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < M_SIZE; ++k) {
                sum += a._data[i][k] * bTransposed._data[j][k];
            }
            result._data[i][j] = sum;
        }
    }
    return result;
}

template <MatrixSize M_SIZE>
Matrix<M_SIZE> Matrix<M_SIZE>::simdMultiply(const Matrix<M_SIZE>& a,
                                            const Matrix<M_SIZE>& b) {
    Matrix<M_SIZE> result;
    Matrix<M_SIZE> bTransposed;
    
    // Transpose matrix b
    #pragma omp parallel for schedule(dynamic, 2)
    for (size_t i = 0; i < M_SIZE; ++i) {
        for (size_t j = 0; j < M_SIZE; ++j) {
            bTransposed._data[j][i] = b._data[i][j];
        }
    }
    
    // Multiply with SIMD
    for (size_t i = 0; i < M_SIZE; ++i) {
        for (size_t j = 0; j < M_SIZE; ++j) {
            __m256d sum = _mm256_setzero_pd();
            size_t k = 0;
            
            for (; k + 3 < M_SIZE; k += 4) {
                __m256d a_vec = _mm256_loadu_pd(&a._data[i][k]);
                __m256d b_vec = _mm256_loadu_pd(&bTransposed._data[j][k]);
                sum = _mm256_add_pd(sum, _mm256_mul_pd(a_vec, b_vec));
            }
            
            double temp[4];
            _mm256_storeu_pd(temp, sum);
            double total = temp[0] + temp[1] + temp[2] + temp[3];
            
            for (; k < M_SIZE; ++k) {
                total += a._data[i][k] * bTransposed._data[j][k];
            }
            
            result._data[i][j] = total;
        }
    }
    return result;
}

template <MatrixSize M_SIZE>
Matrix<M_SIZE> Matrix<M_SIZE>::strassenMultiply(const Matrix<M_SIZE>& a,
                                                const Matrix<M_SIZE>& b) {
    constexpr MatrixSize HALF = HalfSize<M_SIZE>::value;
    
    if constexpr (HALF != M_NONE) {
        // Рекурсивный случай
        return strassenRecursive(a, b);
    } else {
        // Базовый случай для самых маленьких матриц
        return simdMultiply(a, b);
    }
}

#endif