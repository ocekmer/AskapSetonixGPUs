#include <iostream>
#include <iomanip>
#include <omp.h>
#include <vector>
#include <memory>
#include <string>

#include "Parameters.h"
#include "utilities/include/ImageProcess.h"
#include "utilities/include/MaxError.h"
//#include "utilities/WarmupGPU.h"
//#include "utilities/WarmupSetup.h"
#include "solvers/factory/SolverFactory.h"
//#include "utilities/PrintVector.h"
//#include "utilities/CheckZeros.h"

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

int main()
{
	cout << "Clean resurrected..." << endl;

	// Max error evaluator
	MaxError<float> maxError;

	// Initiate an image process object 
	ImageProcess imagProc;

	/*
	// Warmup object
	WarmupGPU warmupGPU;
	warmupSetup();

	// Check if the vector is sparse
	CheckZeros checkZeros;

	// Print vectors to plot
	PrintVector printVector;
	std::string psfFile{ "../results/psf.dat" };
	std::string psfSizeFile{ "../results/psfSize.dat" };
	std::string dirtyFile{ "../results/dirty.dat" };
	std::string dirtySizeFile{ "../results/dirtySize.dat" };
	std::string modelFile{ "../results/model.dat" };
	std::string modelSizeFile{ "../results/modelSize.dat" };
	std::string residualFile{ "../results/residual.dat" };
	std::string residualSizeFile{ "../results/residualSize.dat" };
	*/

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

	//int zerosDirty = checkZeros.checkZeros(dirty);
	//cout << "\nZeros in dirty image: " << zerosDirty << " (" << (float)zerosDirty / (IMAGE_SIZE * IMAGE_SIZE) * 100 << "%)" << endl;
	//int zerosPsf = checkZeros.checkZeros(psf);
	//cout << "Zeros in psf image: " << zerosPsf << " (" << (float)zerosPsf / (IMAGE_SIZE * IMAGE_SIZE) * 100 << "%)" << endl;

	// Reporting problem size and number of iterations
	cout << "\nIterations: " << gNiters << endl;
	cout << "Image dimension: " << IMAGE_SIZE << " x " << IMAGE_SIZE << endl;

	/*
	if (refGPU)
	{
		warmupGPU.warmup();
		cout << "Warmup for reference solver: " << refSolverName << endl;
	}
	*/

	// Reference solver
	vector<float> refResidual(IMAGE_SIZE * IMAGE_SIZE, 0.0);
	vector<float> refModel(IMAGE_SIZE * IMAGE_SIZE, 0.0);
	cout << "\nSolver: " << refSolverName << endl;
	SolverFactory refSolverFactory(dirty, psf, IMAGE_SIZE, refModel, refResidual);
	shared_ptr<ISolver> solver = refSolverFactory.getSolver(refSolverName);
	t0 = omp_get_wtime();
	solver->deconvolve();
	t1 = omp_get_wtime();
	auto timeRef = t1 - t0;

	/*
	if (!refGPU && testGPU)
	{
		warmupGPU.warmup();
		cout << "Warmup for test solver: " << testSolverName << endl;
	}
	*/

	// Test solver
	vector<float> testResidual(IMAGE_SIZE * IMAGE_SIZE, 0.0);
	vector<float> testModel(IMAGE_SIZE * IMAGE_SIZE, 0.0);
	cout << "\nSolver: " << testSolverName << endl;
	SolverFactory testSolverFactory(dirty, psf, IMAGE_SIZE, testModel, testResidual);
	solver = testSolverFactory.getSolver(testSolverName);
	t0 = omp_get_wtime();
	solver->deconvolve();
	t1 = omp_get_wtime();
	auto timeTest = t1 - t0;

	cout << "\nReference & Test Models:" << endl;
	maxError.maxError(testModel, refModel);
	cout << "\nReference & Test Residuals:" << endl;
	maxError.maxError(testResidual, refResidual);

	/*
	// Printing vectors to plot
	t0 = omp_get_wtime();
	printVector.printVector(dirty, dirtyFile, dirtySizeFile, IMAGE_SIZE, IMAGE_SIZE);
	printVector.printVector(psf, psfFile, psfSizeFile, IMAGE_SIZE, IMAGE_SIZE);
	printVector.printVector(testModel, modelFile, modelSizeFile, IMAGE_SIZE, IMAGE_SIZE);
	printVector.printVector(testResidual, residualFile, residualSizeFile, IMAGE_SIZE, IMAGE_SIZE);
	t1 = omp_get_wtime();
	auto timePrint = t1 - t0;
	*/

	cout << "\nRUNTIMES" << endl;
	cout << setprecision(4) << fixed;
	cout << left << setw(21) << "Read image" << left << timeImagProc << " s" << endl;
	cout << left << setw(21) << refSolverName << left << timeRef << " s" << endl;
	cout << left << setw(21) << testSolverName << left << timeTest << " s" << endl;
	cout << left << setw(21) << "Speedup" << left << timeRef / timeTest << endl;
	//cout << left << setw(21) << "Print time" << left << timePrint << " s" << endl;
}