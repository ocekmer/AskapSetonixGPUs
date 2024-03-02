#include "../include/GpuSimple.h"

using std::vector;
using std::cout;
using std::endl;
using std::min;
using std::max;

__global__
void dFindPeak_Step2_Simple(float* data, size_t* inIndex, size_t* outIndex, size_t n)
{
    __shared__ float TILE_VAL[BLOCK_SIZE];
    __shared__ size_t TILE_IDX[BLOCK_SIZE];

    size_t tileIdx = threadIdx.x;
    TILE_VAL[tileIdx] = 0.0;
    TILE_IDX[tileIdx] = 0;
    size_t globalIdx = threadIdx.x + blockIdx.x * blockDim.x;

    // grid stride loop to load data
    while (globalIdx < n)
    {
        if (fabs(data[globalIdx]) > fabs(TILE_VAL[tileIdx]))
        {
            TILE_VAL[tileIdx] = data[globalIdx];
            TILE_IDX[tileIdx] = inIndex[globalIdx];
        }
        globalIdx += gridDim.x * blockDim.x;
    }

    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1)
    {
        __syncthreads();

        // parallel sweep reduction
        if (tileIdx < s)
        {
            if (fabs(TILE_VAL[tileIdx + s]) > fabs(TILE_VAL[tileIdx]))
            {
                TILE_VAL[tileIdx] = TILE_VAL[tileIdx + s];
                TILE_IDX[tileIdx] = TILE_IDX[tileIdx + s];
            }
        }
    }

    if (tileIdx == 0)
    {
        outIndex[blockIdx.x] = TILE_IDX[tileIdx];
        data[blockIdx.x] = TILE_VAL[tileIdx];
    }
}

__global__
void dFindPeak_Step1_Simple(const float* data, float* outMax, size_t* outIndex, size_t n)
{
    __shared__ float TILE_VAL[BLOCK_SIZE];
    __shared__ size_t TILE_IDX[BLOCK_SIZE];

    size_t tileIdx = threadIdx.x;

    TILE_VAL[tileIdx] = 0.0;
    TILE_IDX[tileIdx] = 0;

    size_t globalIdx = threadIdx.x + blockIdx.x * blockDim.x;
    size_t gridSize = gridDim.x * blockDim.x;

    // grid stride loop to load data
    while (globalIdx < n)
    {
        if (fabs(data[globalIdx]) > fabs(TILE_VAL[tileIdx]))
        {
            TILE_VAL[tileIdx] = data[globalIdx];
            TILE_IDX[tileIdx] = globalIdx;
        }
        globalIdx += gridSize;
    }

    __syncthreads();

    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1)
    {
        //__syncthreads();

        // parallel sweep reduction
        if (tileIdx < s)
        {
            if (fabs(TILE_VAL[tileIdx + s]) > fabs(TILE_VAL[tileIdx]))
            {
                TILE_VAL[tileIdx] = TILE_VAL[tileIdx + s];
                TILE_IDX[tileIdx] = TILE_IDX[tileIdx + s];
            }
        }
        __syncthreads();
    }

    if (tileIdx == 0)
    {
        outMax[blockIdx.x] = TILE_VAL[tileIdx];
        outIndex[blockIdx.x] = TILE_IDX[tileIdx];
    }
}

__host__
void GpuSimple::reportDevice()
{
    // Report the type of device being used
    int device;
    gpuDeviceProp_t devprop;
    gpuGetDevice(&device);
    gpuGetDeviceProperties(&devprop, device);
    std::cout << "    Using GPU Device " << device << ": "
        << devprop.name << std::endl;
}

__global__
void dSubtractPSF_Simple(const float* dPsf,
    float* dResidual,
    const int imageWidth,
    const int startx, const int starty,
    int const stopx, const int stopy,
    const int diffx, const int diffy,
    const float absPeakVal, const float gain)
{
    const int x = startx + threadIdx.x + (blockIdx.x * blockDim.x);
    const int y = starty + threadIdx.y + (blockIdx.y * blockDim.y);

    // Because thread blocks are of size 16, and the workload is not always
    // a multiple of 16, need to ensure only those threads whose responsibility
    // lies in the work area actually do work
    if (x <= stopx && y <= stopy)
    {
        dResidual[y * imageWidth + x] -= gain * absPeakVal
            * dPsf[(y - diffy) * imageWidth + (x - diffx)];
    }
}

void GpuSimple::subtractPSF(const size_t peakPos,
    const size_t psfPeakPos,
    const float absPeakVal) {}

__host__
void GpuSimple::subtractPSFSimple(const int& px, const int& py, const size_t& peakPos,
    const float& absPeakVal)
{
    const int blockDim = 16;

    const int rx = peakPos % imageWidth;
    const int ry = peakPos / imageWidth;

    const int diffx = rx - px;
    const int diffy = ry - px;

    const int startx = (0 > diffx) ? 0 : diffx;
    const int starty = (0 > diffy) ? 0 : diffy;

    //const int startx = max(0, diffx);
    //const int starty = max(0, diffy);

    const int stopx = ((imageWidth - 1) < (rx + (imageWidth - px - 1))) ? (imageWidth - 1) : (rx + (imageWidth - px - 1));
    const int stopy = ((imageWidth - 1) < (ry + (imageWidth - py - 1))) ? (imageWidth - 1) : (ry + (imageWidth - py - 1));

    //const int stopx = min(imageWidth - 1, rx + (imageWidth - px - 1));
    //const int stopy = min(imageWidth - 1, ry + (imageWidth - py - 1));


    // Note: Both start* and stop* locations are inclusive.
    const int blocksx = ceil((stopx - startx + 1.0) / static_cast<float>(blockDim));
    const int blocksy = ceil((stopy - starty + 1.0) / static_cast<float>(blockDim));

    dim3 numBlocks(blocksx, blocksy);
    dim3 threadsPerBlock(blockDim, blockDim);
    dSubtractPSF_Simple << <numBlocks, threadsPerBlock >> > (dPsf, dResidual, imageWidth,
        startx, starty, stopx, stopy, diffx, diffy, absPeakVal, gGain);
    gpuCheckErrors("kernel launch failure in subtractPSF");
}

void GpuSimple::deconvolve()
{
    //gpuReportDevice();
    reportDevice();

    residual = dirty;

    // Allocate memory for device vectors
    memAlloc();

    // Copy data from host to device
    copyH2D();

    const size_t SIZE_MAX_VALUE = GRID_SIZE * sizeof(float);
    const size_t SIZE_MAX_INDEX = GRID_SIZE * sizeof(size_t);

    // Device vectors
    float* dMax;
    size_t* dIndex;
    size_t* d2Index;

    gpuMalloc(&dMax, SIZE_MAX_VALUE);
    gpuMalloc(&dIndex, SIZE_MAX_INDEX);
    gpuMalloc(&d2Index, sizeof(size_t));
    gpuCheckErrors("gpuMalloc failure!");

    // Find peak of psf
    float psfPeakVal = 0.0;
    size_t psfPeakPos = 0;
    findPeak(dMax, dIndex, d2Index, psfPeakVal, psfPeakPos, dPsf, psf.size());
    const int px = psfPeakPos % imageWidth;
    const int py = psfPeakPos / imageWidth;

    cout << "Found peak of PSF: " << "Maximum = " << psfPeakVal
        << " at location " << psfPeakPos % imageWidth << ","
        << psfPeakPos / imageWidth << endl;
    float peakVal;
    size_t peakPos;
    for (unsigned int i = 0; i < gNiters; ++i)
    {
        // Find peak in the residual image
        peakVal = 0.0;
        peakPos = 0;
        findPeak(dMax, dIndex, d2Index, peakVal, peakPos, dResidual, residual.size());
        if ((i + 1) % 100 == 0 || i == 0)
        {
            cout << "Iteration: " << i + 1 << " - Maximum = " << peakVal
                << " at location " << peakPos % imageWidth << ","
                << peakPos / imageWidth << ", index = " << peakPos << endl;
        }

        // Check if threshold has been reached
        if (abs(peakVal) < gThreshold)
        {
            cout << "Reached stopping threshold" << endl;
            break;
        }

        // Subtract the PSF from the residual image
        // This function will launch a kernel
        // asynchronously, need to sync later
        subtractPSFSimple(px, py, peakPos, peakVal);
        // Add to model
        model[peakPos] += peakVal * gGain;
    }

    gpuFree(dMax);
    gpuFree(dIndex);
    gpuFree(d2Index);
    gpuCheckErrors("gpuFree failure!");

    copyD2H();
}

__host__
void GpuSimple::findPeak(float* dMax, size_t* dIndex, size_t* d2Index, float& peakValue, size_t& peakPos, const float* dData, size_t N)
{

    // Host vector for max values
    vector<float> hMax(GRID_SIZE, 0.0);
    // Host vector for index values
    vector<size_t> hIndex(GRID_SIZE, 0);


    dFindPeak_Step1_Simple << <GRID_SIZE, BLOCK_SIZE >> > (dData, dMax, dIndex, N);
    gpuCheckErrors("gpu kernel launch 1 failure!");
    dFindPeak_Step2_Simple << <1, BLOCK_SIZE >> > (dMax, dIndex, d2Index, GRID_SIZE);
    gpuCheckErrors("gpu kernel launch 2 failure!");

    gpuMemcpy(hMax.data(), dMax, sizeof(float), gpuMemcpyDeviceToHost);
    gpuCheckErrors("gpuMemcpy D2H failure in findPeak (hmax)!");
    gpuMemcpy(hIndex.data(), d2Index, sizeof(size_t), gpuMemcpyDeviceToHost);
    gpuCheckErrors("gpuMemcpy D2H failure in findPeak (hindex)!");

    peakValue = hMax[0];
    peakPos = hIndex[0];


}

void GpuSimple::memAlloc()
{
    gpuMalloc(&dDirty, SIZE_IMAGE);
    gpuMalloc(&dPsf, SIZE_IMAGE);
    gpuMalloc(&dResidual, SIZE_IMAGE);
    gpuCheckErrors("gpuMalloc failure");
}

GpuSimple::~GpuSimple()
{
    gpuFree(dDirty);
    gpuFree(dPsf);
    gpuFree(dResidual);
    gpuCheckErrors("gpuFree failure");
}

void GpuSimple::copyH2D()
{
    gpuMemcpy(dDirty, dirty.data(), SIZE_IMAGE, gpuMemcpyHostToDevice);
    gpuMemcpy(dPsf, psf.data(), SIZE_IMAGE, gpuMemcpyHostToDevice);
    gpuMemcpy(dResidual, residual.data(), SIZE_IMAGE, gpuMemcpyHostToDevice);
    gpuCheckErrors("gpuMemcpy H2D failure");
}

void GpuSimple::copyD2H()
{
    gpuMemcpy(residual.data(), dResidual, SIZE_IMAGE, gpuMemcpyDeviceToHost);
    gpuCheckErrors("gpuMemcpy D2H failure");
}
