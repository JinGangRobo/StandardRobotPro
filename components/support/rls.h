/**
 ******************************************************************************
 * @file    rls.h
 * @brief   Recursive Least Squares (RLS) implementation
 * @author  Spoon Guan
 ******************************************************************************
 * Copyright (c) 2023 Team JiaoLong-SJTU
 * All rights reserved.
 ******************************************************************************
 */

#ifndef RLS_H
#define RLS_H

#include "matrix.h"
#include "FreeRTOS.h"
#include "task.h"

typedef struct
{
    uint32_t dimension; // Dimension of the RLS space
    float lambda;       // The forgetting factor (0 < λ ≤ 1)
    float delta;        // Initialized value of the transferred matrix

    TickType_t lastUpdate; // Last update tick
    uint32_t updateCnt;    // Total update count

    // RLS relevant matrices
    Matrixf transMatrix;         // Transfer matrix (P)
    Matrixf gainVector;          // Gain vector (K) for params update
    Matrixf paramsVector;        // Parameters vector (θ)
    Matrixf defaultParamsVector; // Default parameters

    float output; // Estimated/filtered output

    // Memory buffers (allocated during initialization)
    float *transMatrixData;
    float *gainVectorData;
    float *paramsVectorData;
    float *defaultParamsVectorData;
} RLS;

/**
 * @brief Initialize RLS filter
 * @param rls Pointer to RLS instance
 * @param dim Dimension of the problem
 * @param delta_ Initial diagonal value for P matrix
 * @param lambda_ Forgetting factor (0 < λ ≤ 1)
 * @return 0 on success, -1 on failure
 */
int RLS_Init(RLS *rls, uint32_t dim, float delta_, float lambda_);

/**
 * @brief Initialize RLS filter with initial parameters
 * @param rls Pointer to RLS instance
 * @param dim Dimension of the problem
 * @param delta_ Initial diagonal value for P matrix
 * @param lambda_ Forgetting factor (0 < λ ≤ 1)
 * @param initParam Initial parameter vector
 * @return 0 on success, -1 on failure
 */
int RLS_InitWithParams(RLS *rls, uint32_t dim, float delta_, float lambda_, const float *initParam);

/**
 * @brief Reset RLS filter to initial state
 * @param rls Pointer to RLS instance
 */
void RLS_Reset(RLS *rls);

/**
 * @brief Update RLS filter with new sample
 * @param rls Pointer to RLS instance
 * @param sampleVector New input sample vector (dim x 1)
 * @param actualOutput Actual measured output
 * @return Pointer to updated parameter vector
 */
const float *RLS_Update(RLS *rls, const float *sampleVector, float actualOutput);

/**
 * @brief Set parameter vector
 * @param rls Pointer to RLS instance
 * @param updatedParams New parameter values
 */
void RLS_SetParamVector(RLS *rls, const float *updatedParams);

/**
 * @brief Get current parameter vector
 * @param rls Pointer to RLS instance
 * @return Pointer to parameter vector
 */
const float *RLS_GetParamsVector(const RLS *rls);

/**
 * @brief Get current output estimate
 * @param rls Pointer to RLS instance
 * @return Current output estimate
 */
float RLS_GetOutput(const RLS *rls);

/**
 * @brief Free resources used by RLS filter
 * @param rls Pointer to RLS instance
 */
void RLS_Deinit(RLS *rls);

#endif // RLS_H