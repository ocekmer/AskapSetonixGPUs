#include "../include/CpuSerial.h"

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::abs;
using std::vector;

void CpuSerial::findPeak(const vector<float>& image, float& maxVal, size_t& maxPos)
{
	maxVal = 0.0;
	maxPos = 0;
	const size_t SIZE = image.size();

	for (auto i = 0; i < SIZE; ++i)
	{
		if (abs(image[i]) > abs(maxVal))
		{
			maxVal = image[i];
			maxPos = i;
		}
	}
}

void CpuSerial::subtractPSF(const size_t peakPos,
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

	for (int y = starty; y <= stopy; ++y)
	{
		for (int x = startx; x <= stopx; ++x)
		{
			residual[y * imageWidth + x] -= gGain * absPeakVal
				* psf[(y - diffy) * imageWidth + (x - diffx)];
		}
	}
}

void CpuSerial::deconvolve()
{
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
