#pragma once

#include <string>
#include <sched.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#include "GpuCommon.h"

class Hello

{
private:
    int size;
    int rank;
    std::string name;

public:
    Hello(int size, int rank, std::string name) : size{size}, rank{rank}, name{name} {}
    void hello() const;

};