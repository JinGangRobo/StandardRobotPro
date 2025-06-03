/**
 ******************************************************************************
 * @file    matrix.h
 * @brief   Matrix/vector calculation. 矩阵/向量运算
 * @author  Spoon Guan
 ******************************************************************************
 * Copyright (c) 2023 Team JiaoLong-SJTU
 * All rights reserved.
 ******************************************************************************
 */

#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>
#include <string.h>
#include <math.h>
#include "arm_math.h"

// 定义矩阵结构体
typedef struct {
    uint32_t rows;
    uint32_t cols;
    float* data;
    arm_matrix_instance_f32 arm_mat;  // ARM CMSIS-DSP矩阵实例
} Matrixf;

// 函数声明

// 初始化矩阵
void matrix_init(Matrixf* mat, uint32_t rows, uint32_t cols, float* data);

// 复制矩阵
void matrix_copy(Matrixf* dest, const Matrixf* src);

// 创建零矩阵
void matrix_zeros(Matrixf* mat);

// 创建单位矩阵
void matrix_eye(Matrixf* mat);

// 创建对角矩阵
void matrix_diag(Matrixf* mat, const Matrixf* vec);

// 矩阵加法
void matrix_add(const Matrixf* mat1, const Matrixf* mat2, Matrixf* result);

// 矩阵减法
void matrix_sub(const Matrixf* mat1, const Matrixf* mat2, Matrixf* result);

// 矩阵缩放
void matrix_scale(const Matrixf* mat, float val, Matrixf* result);

// 矩阵乘法
void matrix_mult(const Matrixf* mat1, const Matrixf* mat2, Matrixf* result);

// 矩阵转置
void matrix_trans(const Matrixf* mat, Matrixf* result);

// 矩阵求逆
int matrix_inv(const Matrixf* mat, Matrixf* result);

// 获取矩阵的迹
float matrix_trace(const Matrixf* mat);

// 计算矩阵范数
float matrix_norm(const Matrixf* mat);

// 获取子矩阵
void matrix_block(const Matrixf* mat, uint32_t start_row, uint32_t start_col, 
                 uint32_t rows, uint32_t cols, Matrixf* result);

// 获取行向量
void matrix_row(const Matrixf* mat, uint32_t row, Matrixf* result);

// 获取列向量
void matrix_col(const Matrixf* mat, uint32_t col, Matrixf* result);

// 矩阵比较
int matrix_equal(const Matrixf* mat1, const Matrixf* mat2);

#endif // MATRIX_H