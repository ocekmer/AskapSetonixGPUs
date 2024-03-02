#include "../include/Hello.h"

using std::cout;
using std::endl;
using std::string;
using std::to_string;

void Hello::hello() const
{
    // If ROCR_VISIBLE_DEVICES is set, capture visible GPUs
    const char* gpu_id_list;
    const char* gpu_visible_devices = getenv(GPU_VISIBLE_DEVICES);
    if (gpu_visible_devices == NULL)
    {
        gpu_id_list = "N/A";
    }
    else
    {
        gpu_id_list = gpu_visible_devices;
    }

    // Find how many GPUs HIP runtime says are available
    int num_devices = 0;
//    gpuErrorCheck(gpuGetDeviceCount(&num_devices));
    gpuGetDeviceCount(&num_devices);
    gpuCheckErrors("device count error");

    int hwthread;
    int thread_id = 0;

    if (num_devices == 0)
    {
        #pragma omp parallel default(shared) private(hwthread, thread_id)
        {
            thread_id = omp_get_thread_num();
            hwthread = sched_getcpu();

            cout << "MPI " << rank << " - OMP " << thread_id << " - HWT " << hwthread << " - Node " << name << endl;
        }
    }
    else
    {
        char busid[64];

        string busid_list = "";
        string rt_gpu_id_list = "";

        // Loop over the GPUs available to each MPI rank
        for (int i = 0; i < num_devices; ++i)
        {
            //gpuErrorCheck(gpuSetDevice(i));
            gpuSetDevice(i);
            gpuCheckErrors("device set error");
            // Get the PCIBusId for each GPU and use it to query for UUID
            //gpuErrorCheck(gpuDeviceGetPCIBusId(busid, 64, i));
            gpuDeviceGetPCIBusId(busid, 64, i);
            gpuCheckErrors("device bus id get error");

            // Concatenate per-MPIrank GPU info into strings for print
            if (i > 0)
            {
                rt_gpu_id_list.append(",");
            }
            rt_gpu_id_list.append(to_string(i));
            string temp_busid(busid);

            if (i > 0)
            {
                busid_list.append(",");
            }
            busid_list.append(temp_busid.substr(5, 2));

        }

        #pragma omp parallel default(shared) private(hwthread, thread_id)
        {
            #pragma omp critical
            {
                thread_id = omp_get_thread_num();
                hwthread = sched_getcpu();
                cout << "MPI " << rank << " - OMP " << thread_id << " - HWT " << hwthread << " - Node " << name 
                << " - RT_GPU_ID " << rt_gpu_id_list.c_str() << " - GPU_ID " << gpu_id_list << " - Bus_ID " << busid_list.c_str()
                << endl;
            }
        }



    }
}

