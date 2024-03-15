#pragma once

#include "../interface/ISolver.h"
#include "../../utilities/include/GpuCommon.h"

/*
// CUDA libs
#include <cuda_runtime_api.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
*/
#include <cmath>
#include <iostream>

/*
// Error checking macro
#define gpuCheckErrors(msg) \
    do { \
        cudaError_t __err = cudaGetLastError(); \
        if (__err != cudaSuccess) { \
            fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
                msg, cudaGetErrorString(__err), \
                __FILE__, __LINE__); \
            fprintf(stderr, "*** FAILED - ABORTING\n"); \
            exit(1); \
        } \
    } while (0)

*/

class GpuSimple : public ISolver
{
private:
	// device vectors
	float* dDirty;
	float* dPsf;
	float* dResidual;

	const size_t SIZE_IMAGE = dirty.size() * sizeof(float);

	void reportDevice();

	// Private methods
	//__host__
		void findPeak(float* dMax, size_t* dIndex, size_t* d2Index, float& peakValue, size_t& peakPos, const float* dData, size_t N);

	void subtractPSF(const size_t peakPos,
		const size_t psfPeakPos,
		const float absPeakVal) override;

	void subtractPSFSimple(const int& px, const int& py, const size_t& peakPos,
		const float& absPeakVal);

	void memAlloc();
	void copyH2D();
	void copyD2H();

public:
	GpuSimple(const std::vector<float>& dirty,
		const std::vector<float>& psf,
		const size_t imageWidth,
		std::vector<float>& model,
		std::vector<float>& residual) : ISolver(dirty, psf, imageWidth,
			model, residual) {}

	virtual ~GpuSimple();

	// Public methods
	void deconvolve() override;


};