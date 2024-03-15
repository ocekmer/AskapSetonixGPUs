#pragma once

#include "../interface/ISolver.h"

#include <cmath>
#include <iostream>
#include <omp.h>

class CpuOmp : public ISolver
{
private:
	// Private methods
	void findPeak(const std::vector<float>& image, float& maxVal, size_t& maxPos);
	void subtractPSF(const size_t peakPos,
		const size_t psfPeakPos,
		const float absPeakVal) override;

	const int NTHREADS = omp_get_max_threads();
	//const int NTHREADS = 4;

public:
	CpuOmp(const std::vector<float>& dirty,
		const std::vector<float>& psf,
		const size_t imageWidth,
		std::vector<float>& model,
		std::vector<float>& residual) : ISolver(dirty, psf, imageWidth,
			model, residual) {}

	virtual ~CpuOmp() {}

	// Public methods
	void deconvolve() override;
};