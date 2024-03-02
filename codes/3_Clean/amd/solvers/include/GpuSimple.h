#pragma once

#include "../interface/ISolver.h"

#include <hip/hip_runtime.h>
#include <hip/hip_runtime_api.h>

#include <cmath>
#include <iostream>

#define gpuCheckErrors(msg)                                     \
do { \
    hipError_t __err = hipGetLastError(); \
    __err != hipSuccess ? \
    fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n*** FAILED - ABORTING\n", \
        msg, hipGetErrorString(__err), __FILE__, __LINE__), exit(1) : \
    void(0); \
} while (0)

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