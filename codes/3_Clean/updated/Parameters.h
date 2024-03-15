#pragma once

#include <string>

/* Set
* 1024 for ~4M data
* 4096 for ~16.7M data
*/
static const size_t IMAGE_WIDTH = 4096;

static const std::string gDirtyFile = "data/dirty_" + std::to_string(IMAGE_WIDTH) + ".img";
static const std::string gPsfFile = "data/psf_" + std::to_string(IMAGE_WIDTH) + ".img";

static const size_t gNiters = 1000;
static const float gGain = 0.1;
static const float gThreshold = 0.00001;

// CUDA Parameters
static const int BLOCK_SIZE = 256;
static const int GRID_SIZE = 512;

static const std::string refSolverName = "cpu";
static const std::string testSolverName = "gpu";

/*
	Solvers:
	- cpu: serial CPU solver
	- gpu: a simple GPU solver


	WARNING: All GPU solvers need to have the letters "gpu"
	(in this order & lower case) in their names
*/

static bool refGPU = false;
static bool testGPU = false;