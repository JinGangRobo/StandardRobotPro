#include "rls.h"
#include <stdlib.h>
#include <string.h>

// Helper function for matrix operations
static void validate_params(float lambda, float delta)
{
    configASSERT(lambda > 0.0f && lambda <= 1.0f);
    configASSERT(delta > 0);
}

int RLS_Init(RLS *rls, uint32_t dim, float delta_, float lambda_)
{
    validate_params(lambda_, delta_);

    // Allocate memory for matrix data
    rls->transMatrixData = (float *)pvPortMalloc(dim * dim * sizeof(float));
    rls->gainVectorData = (float *)pvPortMalloc(dim * sizeof(float));
    rls->paramsVectorData = (float *)pvPortMalloc(dim * sizeof(float));
    rls->defaultParamsVectorData = (float *)pvPortMalloc(dim * sizeof(float));

    if (!rls->transMatrixData || !rls->gainVectorData ||
        !rls->paramsVectorData || !rls->defaultParamsVectorData)
    {
        // Free any allocated memory if initialization fails
        if (rls->transMatrixData)
            vPortFree(rls->transMatrixData);
        if (rls->gainVectorData)
            vPortFree(rls->gainVectorData);
        if (rls->paramsVectorData)
            vPortFree(rls->paramsVectorData);
        if (rls->defaultParamsVectorData)
            vPortFree(rls->defaultParamsVectorData);
        return -1;
    }

    // Initialize matrix structures
    matrix_init(&rls->transMatrix, dim, dim, rls->transMatrixData);
    matrix_init(&rls->gainVector, dim, 1, rls->gainVectorData);
    matrix_init(&rls->paramsVector, dim, 1, rls->paramsVectorData);
    matrix_init(&rls->defaultParamsVector, dim, 1, rls->defaultParamsVectorData);

    rls->dimension = dim;
    rls->lambda = lambda_;
    rls->delta = delta_;
    rls->lastUpdate = 0;
    rls->updateCnt = 0;
    rls->output = 0.0f;

    // Set default parameters to zero
    memset(rls->defaultParamsVectorData, 0, dim * sizeof(float));

    RLS_Reset(rls);
    return 0;
}

int RLS_InitWithParams(RLS *rls, uint32_t dim, float delta_, float lambda_, const float *initParam)
{
    int ret = RLS_Init(rls, dim, delta_, lambda_);
    if (ret == 0)
    {
        memcpy(rls->defaultParamsVectorData, initParam, dim * sizeof(float));
        memcpy(rls->paramsVectorData, initParam, dim * sizeof(float));
    }
    return ret;
}

void RLS_Reset(RLS *rls)
{
    // Reset P matrix to identity * delta
    matrix_eye(&rls->transMatrix);
    matrix_scale(&rls->transMatrix, rls->delta, &rls->transMatrix);

    // Reset gain and parameter vectors
    matrix_zeros(&rls->gainVector);
    memcpy(rls->paramsVectorData, rls->defaultParamsVectorData,
           rls->dimension * sizeof(float));

    rls->updateCnt = 0;
    rls->lastUpdate = xTaskGetTickCount();
}

const float *RLS_Update(RLS *rls, const float *sampleVector, float actualOutput)
{
    // Temporary matrices for calculations
    Matrixf tempVector, tempMatrix;
    float tempData1[rls->dimension];
    float tempMatrixData[rls->dimension * rls->dimension];

    matrix_init(&tempVector, rls->dimension, 1, tempData1);
    matrix_init(&tempMatrix, rls->dimension, rls->dimension, tempMatrixData);

    // Create sample vector matrix
    Matrixf phi;
    matrix_init(&phi, rls->dimension, 1, (float *)sampleVector); // Cast away const for calculation

    // Step 1: K = (P * φ) / (λ + φ' * P * φ)
    matrix_mult(&rls->transMatrix, &phi, &tempVector); // P * φ

    Matrixf phiT;
    float phiTData[rls->dimension];
    matrix_init(&phiT, 1, rls->dimension, phiTData);
    matrix_trans(&phi, &phiT); // φ'

    float denominator;
    matrix_mult(&phiT, &tempVector, &tempMatrix);  // φ' * P * φ
    denominator = rls->lambda + tempMatrixData[0]; // λ + φ' * P * φ

    matrix_scale(&tempVector, 1.0f / denominator, &rls->gainVector); // K = (P * φ) / denominator

    // Step 2: θ = θ + K * (y - φ' * θ)
    float predictedOutput;
    matrix_mult(&phiT, &rls->paramsVector, &tempMatrix); // φ' * θ
    predictedOutput = tempMatrixData[0];
    float error = actualOutput - predictedOutput;

    matrix_scale(&rls->gainVector, error, &tempVector);              // K * error
    matrix_add(&rls->paramsVector, &tempVector, &rls->paramsVector); // θ = θ + K*error

    // Step 3: P = (P - K * φ' * P) / λ
    matrix_mult(&rls->gainVector, &phiT, &tempMatrix);                      // K * φ'
    matrix_mult(&tempMatrix, &rls->transMatrix, &tempMatrix);               // K * φ' * P
    matrix_sub(&rls->transMatrix, &tempMatrix, &rls->transMatrix);          // P - K * φ' * P
    matrix_scale(&rls->transMatrix, 1.0f / rls->lambda, &rls->transMatrix); // (P - Kφ'P)/λ

    // Update statistics
    rls->updateCnt++;
    rls->lastUpdate = xTaskGetTickCount();
    rls->output = predictedOutput;

    return rls->paramsVectorData;
}

void RLS_SetParamVector(RLS *rls, const float *updatedParams)
{
    memcpy(rls->paramsVectorData, updatedParams, rls->dimension * sizeof(float));
    memcpy(rls->defaultParamsVectorData, updatedParams, rls->dimension * sizeof(float));
}

const float *RLS_GetParamsVector(const RLS *rls)
{
    return rls->paramsVectorData;
}

float RLS_GetOutput(const RLS *rls)
{
    return rls->output;
}

void RLS_Deinit(RLS *rls)
{
    if (rls->transMatrixData)
        vPortFree(rls->transMatrixData);
    if (rls->gainVectorData)
        vPortFree(rls->gainVectorData);
    if (rls->paramsVectorData)
        vPortFree(rls->paramsVectorData);
    if (rls->defaultParamsVectorData)
        vPortFree(rls->defaultParamsVectorData);

    memset(rls, 0, sizeof(RLS));
}