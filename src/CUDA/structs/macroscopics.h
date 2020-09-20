/*
*   @file macroscopics.h
*   @author Waine Jr. (waine@alunos.utfpr.edu.br)
*   @brief Struct for LBM macroscopics
*   @version 0.3.0
*   @date 16/12/2019
*/

#ifndef __MACROSCOPICS_H
#define __MACROSCOPICS_H

#include "../var.h"
#include "../globalFunctions.h"
#include "../errorDef.h"
#include <cuda.h>

/*
*   Struct for LBM macroscopics
*/
typedef struct macroscopics
{
private:
    int varLocation;
public:
    dfloat* rho;    // density
    dfloat* ux;     // x velocity
    dfloat* uy;     // y velocity
    dfloat* uz;     // z velocity

    #ifdef IBM
    dfloat* fx;     // x force
    dfloat* fy;     // y force
    dfloat* fz;     // z force
    #endif

    /* Constructor */
    __host__
    macroscopics()
    {
        this->rho = nullptr;
        this->ux = nullptr;
        this->uy = nullptr;
        this->uz = nullptr;
        
        #ifdef IBM
        this->fx = nullptr;
        this->fy = nullptr;
        this->fz = nullptr;
        #endif
    }

    /* Destructor */
    __host__
    ~macroscopics()
    {
        this->rho = nullptr;
        this->ux = nullptr;
        this->uy = nullptr;
        this->uz = nullptr;
        
        #ifdef IBM
        this->fx = nullptr;
        this->fy = nullptr;
        this->fz = nullptr;
        #endif
    }

    /* Allocate macroscopics */
    __host__
    void macrAllocation(int varLocation)
    {
        this->varLocation = varLocation;
        switch (varLocation)
        {
        case IN_HOST:
            // allocate with CUDA for pinned memory and for all GPUS
            checkCudaErrors(cudaMallocHost((void**)&(this->rho), totalMemSizeScalar));
            checkCudaErrors(cudaMallocHost((void**)&(this->ux), totalMemSizeScalar));
            checkCudaErrors(cudaMallocHost((void**)&(this->uy), totalMemSizeScalar));
            checkCudaErrors(cudaMallocHost((void**)&(this->uz), totalMemSizeScalar));
            #ifdef IBM
            checkCudaErrors(cudaMallocHost((void**)&(this->fx), totalMemSizeScalar));
            checkCudaErrors(cudaMallocHost((void**)&(this->fy), totalMemSizeScalar));
            checkCudaErrors(cudaMallocHost((void**)&(this->fz), totalMemSizeScalar));
            #endif
            break;
        case IN_VIRTUAL:
            checkCudaErrors(cudaMallocManaged((void**)&(this->rho), memSizeScalar));
            checkCudaErrors(cudaMallocManaged((void**)&(this->ux), memSizeScalar));
            checkCudaErrors(cudaMallocManaged((void**)&(this->uy), memSizeScalar));
            checkCudaErrors(cudaMallocManaged((void**)&(this->uz), memSizeScalar));
            #ifdef IBM
            checkCudaErrors(cudaMallocManaged((void**)&(this->fx), memSizeScalar));
            checkCudaErrors(cudaMallocManaged((void**)&(this->fy), memSizeScalar));
            checkCudaErrors(cudaMallocManaged((void**)&(this->fz), memSizeScalar));
            #endif
            break;
        default:
            break;
        }
    }

    /* Free macroscopics */
    __host__
    void macrFree()
    {
        switch (varLocation)
        {
        case IN_HOST:
            checkCudaErrors(cudaFreeHost(this->rho));
            checkCudaErrors(cudaFreeHost(this->ux));
            checkCudaErrors(cudaFreeHost(this->uy));
            checkCudaErrors(cudaFreeHost(this->uz));
            #ifdef IBM
            checkCudaErrors(cudaFreeHost(this->fx));
            checkCudaErrors(cudaFreeHost(this->fy));
            checkCudaErrors(cudaFreeHost(this->fz));
            #endif
            break;
        case IN_VIRTUAL:
            checkCudaErrors(cudaFree(this->rho));
            checkCudaErrors(cudaFree(this->ux));
            checkCudaErrors(cudaFree(this->uy));
            checkCudaErrors(cudaFree(this->uz));
            #ifdef IBM
            checkCudaErrors(cudaFree(this->fx));
            checkCudaErrors(cudaFree(this->fy));
            checkCudaErrors(cudaFree(this->fz));
            #endif
            break;
        default:
            break;
        }
    }

    /*  
        Copies macrRef to this object  
        this <- macrRef
    */
    __host__
    void copyMacr(macroscopics* macrRef, size_t baseIdx=0, size_t baseIdxRef=0, bool all_domain=false)
    {
        size_t memSize = (all_domain ? totalMemSizeScalar : memSizeScalar);

        cudaStream_t streamRho, streamUx, streamUy, streamUz;
        checkCudaErrors(cudaStreamCreate(&(streamRho)));
        checkCudaErrors(cudaStreamCreate(&(streamUx)));
        checkCudaErrors(cudaStreamCreate(&(streamUy)));
        checkCudaErrors(cudaStreamCreate(&(streamUz)));
        #ifdef IBM
        checkCudaErrors(cudaStreamCreate(&(streamFx)));
        checkCudaErrors(cudaStreamCreate(&(streamFy)));
        checkCudaErrors(cudaStreamCreate(&(streamFz)));
        #endif

        checkCudaErrors(cudaMemcpyAsync(this->rho+baseIdx, macrRef->rho+baseIdxRef, 
            memSize, cudaMemcpyDefault, streamRho));
        checkCudaErrors(cudaMemcpyAsync(this->ux+baseIdx, macrRef->ux+baseIdxRef, 
            memSize, cudaMemcpyDefault, streamUx));
        checkCudaErrors(cudaMemcpyAsync(this->uy+baseIdx, macrRef->uy+baseIdxRef, 
            memSize, cudaMemcpyDefault, streamUy));
        checkCudaErrors(cudaMemcpyAsync(this->uz+baseIdx, macrRef->uz+baseIdxRef,
            memSize, cudaMemcpyDefault, streamUz));

        #ifdef IBM
        checkCudaErrors(cudaMemcpyAsync(this->fx+baseIdx, macrRef->fx+baseIdxRef, 
            memSize, cudaMemcpyDefault, streamFx));
        checkCudaErrors(cudaMemcpyAsync(this->fy+baseIdx, macrRef->fy+baseIdxRef, 
            memSize, cudaMemcpyDefault, streamFy));
        checkCudaErrors(cudaMemcpyAsync(this->fz+baseIdx, macrRef->fz+baseIdxRef,
            memSize, cudaMemcpyDefault, streamFz));
        #endif

        checkCudaErrors(cudaStreamSynchronize(streamRho));
        checkCudaErrors(cudaStreamSynchronize(streamUx));
        checkCudaErrors(cudaStreamSynchronize(streamUy));
        checkCudaErrors(cudaStreamSynchronize(streamUz));

        checkCudaErrors(cudaStreamDestroy(streamRho));
        checkCudaErrors(cudaStreamDestroy(streamUx));
        checkCudaErrors(cudaStreamDestroy(streamUy));
        checkCudaErrors(cudaStreamDestroy(streamUz));
        #ifdef IBM
        checkCudaErrors(cudaStreamDestroy(streamFx));
        checkCudaErrors(cudaStreamDestroy(streamFy));
        checkCudaErrors(cudaStreamDestroy(streamFz));
        #endif

    }

} Macroscopics;


#endif // !__MACROSCOPICS_H