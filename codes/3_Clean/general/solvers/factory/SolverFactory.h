#pragma once

#include <memory>
#include <string>

#include "../interface/ISolver.h"
#include "../include/CpuSerial.h"
#include "../include/GpuSimple.h"

class SolverFactory
{
private:
	const std::vector<float>& dirty;
	const std::vector<float>& psf;
	const size_t imageWidth;
	std::vector<float>& model;
	std::vector<float>& residual;

	std::shared_ptr<ISolver> solverSelect;

public:
	SolverFactory(const std::vector<float>& dirty,
		const std::vector<float>& psf,
		const size_t imageWidth,
		std::vector<float>& model,
		std::vector<float>& residual) : dirty{ dirty }, psf{ psf }, imageWidth{ imageWidth },
		model{ model }, residual{ residual } {}
	std::shared_ptr<ISolver> getSolver(std::string solverType)
	{
		if (solverType == "cpu")
		{
			solverSelect = std::make_shared<CpuSerial>(dirty, psf, imageWidth,
				model, residual);
		}
		else if (solverType == "gpu")
		{
			solverSelect = std::make_shared<GpuSimple>(dirty, psf, imageWidth,
				model, residual);
		}
		return solverSelect;
	}

};