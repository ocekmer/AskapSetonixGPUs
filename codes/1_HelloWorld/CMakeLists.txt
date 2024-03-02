cmake_minimum_required(VERSION 3.18)

# set the project name
project(hello
	VERSION 1.0
	LANGUAGES CXX)

# specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

macro(hello_option optname optdesc status)
	option(HELLO_${optname} "${optdesc}" "${status}")
endmacro()

hello_option(USE_HIP "Attempt to use HIP instead of CUDA" ON)
hello_option(USE_AMD "Attempt to use HIP AMD" ON)

# set the GPU langugae and standard
if (HELLO_USE_HIP)
    # Find hip
    find_package(HIP REQUIRED)
    # Link with HIP
    enable_language(HIP)
    if(NOT DEFINED CMAKE_HIP_STANDARD)
        set(CMAKE_HIP_STANDARD 17)
        set(CMAKE_HIP_STANDARD_REQUIRED ON)
    endif()
    add_definitions("-DUSEHIP")
    add_compile_options("-fPIE")
    target_link_libraries(hip::device)
    if (HELLO_USE_AMD)
        add_definitions("-D__HIP_PLATFORM_AMD__")
        set(GPU_TARGETS "gfx90a" CACHE STRING "GPU targets to compile for")
    endif()
else()
    find_package(CUDA REQUIRED)
    enable_language(CUDA)
    add_definitions("-DUSECUDA")
    if(NOT DEFINED CMAKE_CUDA_STANDARD)
        set(CMAKE_CUDA_STANDARD 17)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    endif()
endif()

configure_file(helloConfig.h.in helloConfig.h)

# add the executable
add_executable(hello main.cpp)

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
    target_link_libraries(hello PUBLIC OpenMP::OpenMP_CXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -O2")
endif()

find_package(MPI REQUIRED)
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")
set(CMAKE_CXX_COMPILER "hipcc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} ${MPI_CXX_FLAGS} -I/opt/cray/pe/mpich/8.1.19/ofi/gnu/9.1/include -L/opt/cray/pe/mpich/8.1.19/ofi/gnu/9.1/lib -L/opt/cray/pe/mpich/8.1.19/gtl/lib -DMPICH_GPU_SUPPORT_ENABLED=1")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lmpi_gtl_hsa -lmpi -I/opt/cray/xpmem/2.4.4-2.3_13.8__gff0e1d9.shasta/include -L/opt/cray/xpmem/2.4.4-2.3_13.8__gff0e1d9.shasta/lib64/ -Wl,--no-as-needed -lxpmem")
target_link_libraries(hello PUBLIC MPI::MPI_CXX)

# add the include library
#add_subdirectory("utilities/src")

#target_link_libraries(hello PUBLIC UtilitiesCMake)

target_include_directories(hello PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           #"${PROJECT_BINARY_DIR}/utilities/src"
                           )
