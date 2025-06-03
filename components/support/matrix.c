#include "matrix.h"
#include <stdlib.h>

// 初始化矩阵
void matrix_init(Matrixf *mat, uint32_t rows, uint32_t cols, float *data)
{
    mat->rows = rows;
    mat->cols = cols;
    mat->data = data;
    arm_mat_init_f32(&mat->arm_mat, rows, cols, data);
}

// 复制矩阵
void matrix_copy(Matrixf *dest, const Matrixf *src)
{
    if (dest->rows != src->rows || dest->cols != src->cols)
    {
        return; // 尺寸不匹配
    }
    memcpy(dest->data, src->data, src->rows * src->cols * sizeof(float));
    arm_mat_init_f32(&dest->arm_mat, dest->rows, dest->cols, dest->data);
}

// 创建零矩阵
void matrix_zeros(Matrixf *mat)
{
    memset(mat->data, 0, mat->rows * mat->cols * sizeof(float));
}

// 创建单位矩阵
void matrix_eye(Matrixf *mat)
{
    matrix_zeros(mat);
    uint32_t min_dim = (mat->rows < mat->cols) ? mat->rows : mat->cols;
    for (uint32_t i = 0; i < min_dim; i++)
    {
        mat->data[i * mat->cols + i] = 1.0f;
    }
}

// 创建对角矩阵
void matrix_diag(Matrixf *mat, const Matrixf *vec)
{
    if (vec->cols != 1)
        return; // 输入必须是列向量

    matrix_zeros(mat);
    uint32_t min_dim = (mat->rows < mat->cols) ? mat->rows : mat->cols;
    min_dim = (min_dim < vec->rows) ? min_dim : vec->rows;

    for (uint32_t i = 0; i < min_dim; i++)
    {
        mat->data[i * mat->cols + i] = vec->data[i];
    }
}

// 矩阵加法
void matrix_add(const Matrixf *mat1, const Matrixf *mat2, Matrixf *result)
{
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols ||
        mat1->rows != result->rows || mat1->cols != result->cols)
    {
        return; // 尺寸不匹配
    }
    arm_mat_add_f32(&mat1->arm_mat, &mat2->arm_mat, &result->arm_mat);
}

// 矩阵减法
void matrix_sub(const Matrixf *mat1, const Matrixf *mat2, Matrixf *result)
{
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols ||
        mat1->rows != result->rows || mat1->cols != result->cols)
    {
        return; // 尺寸不匹配
    }
    arm_mat_sub_f32(&mat1->arm_mat, &mat2->arm_mat, &result->arm_mat);
}

// 矩阵缩放
void matrix_scale(const Matrixf *mat, float val, Matrixf *result)
{
    if (mat->rows != result->rows || mat->cols != result->cols)
    {
        return; // 尺寸不匹配
    }
    arm_mat_scale_f32(&mat->arm_mat, val, &result->arm_mat);
}

// 矩阵乘法
void matrix_mult(const Matrixf *mat1, const Matrixf *mat2, Matrixf *result)
{
    if (mat1->cols != mat2->rows || mat1->rows != result->rows || mat2->cols != result->cols)
    {
        return; // 尺寸不匹配
    }
    arm_mat_mult_f32(&mat1->arm_mat, &mat2->arm_mat, &result->arm_mat);
}

// 矩阵转置
void matrix_trans(const Matrixf *mat, Matrixf *result)
{
    if (mat->rows != result->cols || mat->cols != result->rows)
    {
        return; // 尺寸不匹配
    }
    arm_mat_trans_f32(&mat->arm_mat, &result->arm_mat);
}

// 矩阵求逆
int matrix_inv(const Matrixf *mat, Matrixf *result)
{
    if (mat->rows != mat->cols || mat->rows != result->rows || mat->cols != result->cols)
    {
        return ARM_MATH_SIZE_MISMATCH; // 必须是方阵且尺寸匹配
    }
    return arm_mat_inverse_f32(&mat->arm_mat, &result->arm_mat);
}

// 获取矩阵的迹
float matrix_trace(const Matrixf *mat)
{
    float trace = 0.0f;
    uint32_t min_dim = (mat->rows < mat->cols) ? mat->rows : mat->cols;

    for (uint32_t i = 0; i < min_dim; i++)
    {
        trace += mat->data[i * mat->cols + i];
    }
    return trace;
}

// 计算矩阵范数
float matrix_norm(const Matrixf *mat)
{
    float norm = 0.0f;
    for (uint32_t i = 0; i < mat->rows * mat->cols; i++)
    {
        norm += mat->data[i] * mat->data[i];
    }
    return sqrtf(norm);
}

// 获取子矩阵
void matrix_block(const Matrixf *mat, uint32_t start_row, uint32_t start_col,
                  uint32_t rows, uint32_t cols, Matrixf *result)
{
    if (start_row + rows > mat->rows || start_col + cols > mat->cols ||
        result->rows != rows || result->cols != cols)
    {
        return; // 尺寸不匹配
    }

    for (uint32_t row = 0; row < rows; row++)
    {
        for (uint32_t col = 0; col < cols; col++)
        {
            result->data[row * cols + col] = mat->data[(start_row + row) * mat->cols + (start_col + col)];
        }
    }
}

// 获取行向量
void matrix_row(const Matrixf *mat, uint32_t row, Matrixf *result)
{
    if (row >= mat->rows || result->rows != 1 || result->cols != mat->cols)
    {
        return; // 尺寸不匹配
    }
    memcpy(result->data, &mat->data[row * mat->cols], mat->cols * sizeof(float));
}

// 获取列向量
void matrix_col(const Matrixf *mat, uint32_t col, Matrixf *result)
{
    if (col >= mat->cols || result->rows != mat->rows || result->cols != 1)
    {
        return; // 尺寸不匹配
    }
    for (uint32_t i = 0; i < mat->rows; i++)
    {
        result->data[i] = mat->data[i * mat->cols + col];
    }
}

// 矩阵比较
int matrix_equal(const Matrixf *mat1, const Matrixf *mat2)
{
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols)
    {
        return 0; // 尺寸不同
    }

    for (uint32_t i = 0; i < mat1->rows * mat1->cols; i++)
    {
        if (fabsf(mat1->data[i] - mat2->data[i]) > 1e-6f)
        {
            return 0; // 元素不同
        }
    }
    return 1; // 矩阵相同
}