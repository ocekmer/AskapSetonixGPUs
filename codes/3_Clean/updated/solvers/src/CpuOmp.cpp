#include "../include/CpuOmp.h"

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::abs;
using std::vector;

void CpuOmp::findPeak(const vector<float>& image, float& maxVal, size_t& maxPos)
{
	maxVal = 0.0;
	maxPos = 0;
	const size_t SIZE = image.size();

	float threadAbsMaxVal = 0.0;
	size_t threadAbsMaxPos = 0;

	#pragma omp parallel for
	for (auto i = 0; i < SIZE; ++i)
	{
		if (abs(image[i]) > abs(threadAbsMaxVal))
		{
			threadAbsMaxVal = image[i];
			threadAbsMaxPos = i;
		}
	}

#pragma omp critical
	if (abs(threadAbsMaxVal) > abs(maxVal))
	{
		maxVal = threadAbsMaxVal;
		maxPos = threadAbsMaxPos;
	}

}

void CpuOmp::subtractPSF(const size_t peakPos,
	const size_t psfPeakPos,
	const float absPeakVal)
{
	const int rx = peakPos % imageWidth;
	const int ry = peakPos / imageWidth;

	const int px = psfPeakPos % imageWidth;
	const int py = psfPeakPos / imageWidth;

	const int diffx = rx - px;
	const int diffy = ry - py;

	const int startx = max(0, diffx);
	const int starty = max(0, diffy);

	const int stopx = min(imageWidth - 1, rx + (imageWidth - px - 1));
	const int stopy = min(imageWidth - 1, ry + (imageWidth - py - 1));
	/*
		cout << "rx = " << rx << endl;
		cout << "ry = " << ry << endl;
		cout << "px = " << px << endl;
		cout << "py = " << py << endl;
		cout << "diffx = " << diffx << endl;
		cout << "diffy = " << diffy << endl;
		cout << "startx = " << startx << endl;
		cout << "starty = " << starty << endl;
		cout << "stopx = " << stopx << endl;
		cout << "stopy = " << stopy << endl;
		cout << "stopx - startx = " << stopx - startx << endl;
		cout << "stopy - starty = " << stopy - starty << endl;
	*/

	#pragma omp parallel for default(shared) schedule(static)
	for (int y = starty; y <= stopy; ++y)
	{
		for (int x = startx; x <= stopx; ++x)
		{
			residual[y * imageWidth + x] -= gGain * absPeakVal
				* psf[(y - diffy) * imageWidth + (x - diffx)];
		}
	}
}

void CpuOmp::deconvolve()
{
	omp_set_num_threads(NTHREADS);
	cout << "Working with " << NTHREADS << " threads" << endl;


	residual = dirty;

	// Find the peak of the PSF
	float psfPeakVal = 0.0;
	size_t psfPeakPos = 0;
	findPeak(psf, psfPeakVal, psfPeakPos);

	cout << "PSF peak: " << psfPeakVal << ", at location: " << psfPeakPos % imageWidth << ", "
		<< psfPeakPos / imageWidth << endl;

	for (unsigned int i = 0; i < gNiters; ++i)
	{
		// Find the peak in the residual image
		float absPeakVal = 0.0;
		size_t absPeakPos = 0;
		findPeak(residual, absPeakVal, absPeakPos);

		if ((i + 1) % 100 == 0 || i == 0)
		{
			cout << "Iteration: " << i + 1 << " - Maximum = " << absPeakVal
				<< " at location " << absPeakPos % imageWidth << ","
				<< absPeakPos / imageWidth << ", index = " << absPeakPos << endl;
		}

		// Check if the threshold is reached
		if (abs(absPeakVal) < gThreshold)
		{
			cout << "Reached stopping threshold" << endl;
			break;
		}

		// Add to model
		model[absPeakPos] += absPeakVal * gGain;

		// Subtract the PSF from the residual image 
		subtractPSF(absPeakPos, psfPeakPos, absPeakVal);
	}
}
