#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <mpi.h>
#include <sched.h>
#include <omp.h>

// Parameters
#include "Parameters.h"

// Utilities
#include "utilities/include/GpuCommon.h"
#include "utilities/include/Hello.h"
#include "utilities/include/WarmupGPU.h"
#include "utilities/include/ImageProcess.h"
#include "utilities/include/MaxError.h"

// Solvers
#include "solvers/factory/SolverFactory.h"
// #include "solvers/interface/ISolver.h"

using std::cout;
using std::endl;
using std::cerr;
using std::vector;
using std::left;
using std::right;
using std::fixed;
using std::setw;
using std::setprecision;
using std::scientific;
using std::shared_ptr;
using std::string;
using std::to_string;

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char name[MPI_MAX_PROCESSOR_NAME];
    int resultlength; // dummy
    MPI_Get_processor_name(name, &resultlength);

    Hello hello(size, rank, name);
    hello.hello();



	/********************************************************************************/
    
	// Max error evaluator
	MaxError<float> maxError;

	// Initiate an image process object 
	ImageProcess imagProc;

	// Load dirty image & psf
	cout << "Reading dirty image & psf" << endl;
	auto t0 = omp_get_wtime();
	vector<float> dirty = imagProc.readImage(gDirtyFile);
	vector<float> psf = imagProc.readImage(gPsfFile);
	const size_t DIRTY_SIZE = imagProc.checkSquare(dirty);
	const size_t PSF_SIZE = imagProc.checkSquare(psf);
	auto t1 = omp_get_wtime();
	auto timeImagProc = t1 - t0;

	if (PSF_SIZE != DIRTY_SIZE)
	{
		cerr << "Wrong set of PSF & DIRTY images" << endl;
		return -1;
	}

	const size_t IMAGE_SIZE = DIRTY_SIZE;
	
	// Reporting problem size and number of iterations
	cout << "\nIterations: " << gNiters << endl;
	cout << "Image dimension: " << IMAGE_SIZE << " x " << IMAGE_SIZE << endl;
	
	WarmupGPU warmupGPU;
    warmupGPU.setup(refGPU, testGPU);

    cout << "RefGPU = " << refGPU << endl;
    cout << "TestGPU = " << testGPU << endl;


	// Reference Solver
	vector<float> refResidual(IMAGE_SIZE * IMAGE_SIZE, 0.0);
	vector<float> refModel(IMAGE_SIZE * IMAGE_SIZE, 0.0);
	cout << "\nSolver: " << refSolverName << endl;
	SolverFactory refSolverFactory(dirty, psf, IMAGE_SIZE, refModel, refResidual);
	shared_ptr<ISolver> solver = refSolverFactory.getSolver(refSolverName);
	if (refGPU)
	{
		warmupGPU.warmup();
		cout << "Warmup for reference solver: " << refSolverName << endl;
	}
	t0 = omp_get_wtime();
	solver->deconvolve();
	t1 = omp_get_wtime();
	auto timeRef = t1 - t0;
	


	// Test Solver
	vector<float> testResidual(IMAGE_SIZE * IMAGE_SIZE, 0.0);
	vector<float> testModel(IMAGE_SIZE * IMAGE_SIZE, 0.0);
	cout << "\nSolver: " << testSolverName << endl;
	SolverFactory testSolverFactory(dirty, psf, IMAGE_SIZE, testModel, testResidual);
	solver = testSolverFactory.getSolver(testSolverName);
	if ((!refGPU) && testGPU)
	{
		warmupGPU.warmup();
		cout << "Warmup for test solver: " << testSolverName << endl;
	}
	t0 = omp_get_wtime();
	solver->deconvolve();
	t1 = omp_get_wtime();
	auto timeTest = t1 - t0;
	
	cout << "\nReference & Test Models:" << endl;
	maxError.maxError(testModel, refModel);
	cout << "\nReference & Test Residuals:" << endl;
	maxError.maxError(testResidual, refResidual);
	
	cout << "\nRUNTIMES" << endl;
	cout << setprecision(4) << fixed;
	cout << left << setw(21) << "Read image" << left << timeImagProc << " s" << endl;
	cout << left << setw(21) << refSolverName << left << timeRef << " s" << endl;
	cout << left << setw(21) << testSolverName << left << timeTest << " s" << endl;
	cout << left << setw(21) << "Speedup" << left << timeRef / timeTest << endl;

	/********************************************************************************/






    MPI_Finalize();

    return 0;
}