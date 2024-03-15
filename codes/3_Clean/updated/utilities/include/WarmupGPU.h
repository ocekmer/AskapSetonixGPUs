#pragma once

#include "../../Parameters.h"

// Utilities
#include "MaxError.h"

// CUDA libs
#include "GpuCommon.h"

// Standard libs
#include <vector>
#include <iostream>
#include <string>
#include <regex>

class WarmupGPU
{
private:
	const size_t N = 1 << 20;

public:
	void warmup() const;
	void setup(bool& refGPU, bool& testGPU);
};
