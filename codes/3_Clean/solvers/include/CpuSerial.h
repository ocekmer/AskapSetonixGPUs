#pragma once

#include "../interface/ISolver.h"

#include <cmath>
#include <iostream>

class CpuSerial : public ISolver
{
private:
	// Private methods
	void findPeak(const std::vector<float>& image, float& maxVal, size_t& maxPos);
	void subtractPSF(const size_t peakPos,
		const size_t psfPeakPos,
		const float absPeakVal) override;

public:
	CpuSerial(const std::vector<float>& dirty,
		const std::vector<float>& psf,
		const size_t imageWidth,
		std::vector<float>& model,
		std::vector<float>& residual) : ISolver(dirty, psf, imageWidth,
			model, residual) {}

	virtual ~CpuSerial() {}

	// Public methods
	void deconvolve() override;
};