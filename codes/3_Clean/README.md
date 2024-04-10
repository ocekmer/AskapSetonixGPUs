# Deconvolution
## Introduction
This section of the repo includes the serial and accelerated codes for deconvolution, which use the Hogbom's CLEAN algorithm. This algorithm is summarized as follows:
- Start with the dirty image
- Find the brightest pixel
- Subtract a fraction (around 0.1) of the dirty beam from the dirty image at the location of that brightest pixel.
- The subtracted image is called the residual image.
- Find the new brightest pixel.
- Continue the loop until a threshold is reached.

## Code Structure 
- The strategy and factory patterns were applied to have a single version with various solvers.
- In ***main()***, there are two solvers introduced
    1. Reference solver: The solver, which was verified to be correct before that we are going to use to verify the newly developed kernels.
    2. Test solver: The one that we develop and verify using the reference solver.
- The reference and test solvers can be assigned via the file: Parameters.h.
